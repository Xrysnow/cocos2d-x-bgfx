#include "CommandBufferBX.h"
#include "BufferBX.h"
#include "RenderPipelineBX.h"
#include "TextureBX.h"
#include "DepthStencilStateBX.h"
#include "ProgramBX.h"
#include "UtilsBX.h"
#include "CallbackBX.h"
#include "base/ccMacros.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventType.h"
#include "base/CCDirector.h"
#include <algorithm>

#if defined(COCOS2D_DEBUG) && COCOS2D_DEBUG > 0
#define NEED_LOG (_print)
#define LOGFUNC if(NEED_LOG){ CCLOG(__FUNCTION__); }
#else
#define NEED_LOG (false)
#define LOGFUNC if(false){}
#endif

CC_BACKEND_BEGIN

namespace
{
	bgfx::TextureHandle getHandler(TextureBackend* texture)
	{
		if (texture)
		{			
			switch (texture->getTextureType())
			{
			case TextureType::TEXTURE_2D:
				return static_cast<Texture2DBX*>(texture)->getHandle();
			case TextureType::TEXTURE_CUBE:
				return static_cast<TextureCubeBX*>(texture)->getHandle();
			default:
				assert(false);
			}
		}
		return BGFX_INVALID_HANDLE;
	}
}

CommandBufferBX::CommandBufferBX()
{
	_scissor[0] = _scissor[1] = _scissor[2] = _scissor[3] = 0;
	_vpTramsform = { 0,0,1,1 };
	_vpHandle = bgfx::createUniform("u_vpTransform", bgfx::UniformType::Vec4);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED,
		[this](EventCustom*)
	{
		const auto fbo = _generatedFBO;
		addThreadTask([=]()
		{
			if (bgfx::isValid(fbo))
				bgfx::destroy(fbo);
		});
		_generatedFBO = BGFX_INVALID_HANDLE;
	});
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
		_backToForegroundListener, -1);
#endif
}

CommandBufferBX::~CommandBufferBX()
{
	const auto fbo = _generatedFBO;
	addThreadTask([=]()
	{
		if (bgfx::isValid(fbo))
			bgfx::destroy(fbo);
	});
	CC_SAFE_RELEASE_NULL(_renderPipeline);
	cleanResources();
#if CC_ENABLE_CACHE_TEXTURE_DATA
	Director::getInstance()->getEventDispatcher()->removeEventListener(
		_backToForegroundListener);
#endif
}

void CommandBufferBX::beginFrame()
{
	LOGFUNC;
	//_state = 0;
	_state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
	addThreadTask([=]()
	{
		//bgfx::touch(0);
		bgfx::setViewMode(0, bgfx::ViewMode::Sequential);
	});
}

void CommandBufferBX::beginRenderPass(const RenderPassDescriptor& descriptor)
{
	LOGFUNC;
	applyRenderPassDescriptor(descriptor);
}

void CommandBufferBX::setRenderPipeline(RenderPipeline* renderPipeline)
{
	LOGFUNC;
	assert(renderPipeline);
	if (!renderPipeline)
		return;
	CC_SAFE_RELEASE(_renderPipeline);
	_renderPipeline = static_cast<RenderPipelineBX*>(renderPipeline);
	CC_SAFE_RETAIN(_renderPipeline);
	_state &= ~RenderPipelineBX::getStateMask();
	_state |= _renderPipeline->getState();
}

void CommandBufferBX::setViewport(int x, int y, unsigned w, unsigned h)
{
	const int width = bgfx::getStats()->width;
	const int height = bgfx::getStats()->height;
	_vpTramsform.z = (float)w / width;
	_vpTramsform.w = (float)h / height;
	_vpTramsform.x = (float)x / width * 2 - (1 - _vpTramsform.z);
	_vpTramsform.y = (float)y / height * 2 - (1 - _vpTramsform.w);
	_viewPort.x = x;
	_viewPort.y = y;
	_viewPort.w = w;
	_viewPort.h = h;
	updateScissor();
}

void CommandBufferBX::setCullMode(CullMode mode)
{
	_cullMode = mode;
}

void CommandBufferBX::setWinding(Winding winding)
{
	_winding = winding;
}

void CommandBufferBX::setVertexBuffer(Buffer* buffer)
{
	LOGFUNC;
	assert(buffer);
	if (!buffer)
		return;
	CC_SAFE_RELEASE(_vertexBuffer);
	_vertexBuffer = static_cast<BufferBX*>(buffer);
	CC_SAFE_RETAIN(_vertexBuffer);
}

void CommandBufferBX::setProgramState(ProgramState* programState)
{
	LOGFUNC;
	CC_SAFE_RELEASE(_programState);
	_programState = programState;
	CC_SAFE_RETAIN(_programState);
}

void CommandBufferBX::setIndexBuffer(Buffer* buffer)
{
	LOGFUNC;
	assert(buffer);
	if (!buffer)
		return;
	CC_SAFE_RELEASE(_indexBuffer);
	_indexBuffer = static_cast<BufferBX*>(buffer);
	CC_SAFE_RETAIN(_indexBuffer);
}

void CommandBufferBX::drawArrays(PrimitiveType primitiveType, std::size_t start, std::size_t count)
{
	LOGFUNC;
	prepareDrawing();
	_state &= ~BGFX_STATE_PT_MASK;
	_state |= UtilsBX::toBXStatePrimitiveType(primitiveType);
	_vertexBuffer->apply(start, count, 0);
	if(NEED_LOG)
	{
		const auto p = (ProgramBX*)_programState->getProgram();
		CCLOG("[%d] [drawArrays] start: %d, count: %d, pro: %d (%d)",
			_currentView, start, count, p->getHandle().idx, (int)p->getProgramType());
	}
	const auto scissor = _scissorRect;
	if (scissor.size.width > 0 && scissor.size.height > 0)
	{
		const auto program = ((ProgramBX*)_programState->getProgram())->getHandle();
		const auto state = _state;
		const auto trans = _vpTramsform;
		const auto view = _currentView;
		addThreadTask([=]()
		{
			bgfx::setUniform(_vpHandle, &trans);
			bgfx::setScissor(scissor.origin.x, scissor.origin.y, scissor.size.width, scissor.size.height);
			bgfx::setState(state);
			bgfx::submit(view, program/*, 0, BGFX_DISCARD_INDEX_BUFFER | BGFX_DISCARD_VERTEX_STREAMS | BGFX_DISCARD_TEXTURE_SAMPLERS*/);
		});
	}
	else
	{
		addThreadTask([]()
		{
			bgfx::discard(BGFX_DISCARD_INDEX_BUFFER
				| BGFX_DISCARD_VERTEX_STREAMS
				| BGFX_DISCARD_TEXTURE_SAMPLERS
				| BGFX_DISCARD_COMPUTE);
		});
	}
	cleanResources();
}

void CommandBufferBX::drawElements(PrimitiveType primitiveType, IndexFormat indexType, std::size_t count,
	std::size_t offset)
{
	LOGFUNC;
	prepareDrawing();
	_state &= ~BGFX_STATE_PT_MASK;
	_state |= UtilsBX::toBXStatePrimitiveType(primitiveType);
	_vertexBuffer->apply(0);
	const auto start = offset / (indexType == IndexFormat::U_SHORT ? 2 : 4);
	_indexBuffer->apply(start, count, 0);
	if (NEED_LOG)
	{
		const auto p = (ProgramBX*)_programState->getProgram();
		CCLOG("[%d] [drawElements] offset: %d, count: %d, pro: %d (%d)",
			_currentView, offset / 2, count, p->getHandle().idx, (int)p->getProgramType());
		CCLOG("vp trans: %.2f, %.2f, %.2f, %.2f",
			_vpTramsform.x, _vpTramsform.y, _vpTramsform.z, _vpTramsform.w);
	}
	const auto scissor = _scissorRect;
	if (scissor.size.width > 0 && scissor.size.height > 0)
	{
		const auto program = ((ProgramBX*)_programState->getProgram())->getHandle();
		const auto state = _state;
		const auto trans = _vpTramsform;
		const auto view = _currentView;
		addThreadTask([=]()
		{
			bgfx::setUniform(_vpHandle, &trans);
			bgfx::setScissor(scissor.origin.x, scissor.origin.y, scissor.size.width, scissor.size.height);
			bgfx::setState(state);
			bgfx::submit(view, program/*, 0, BGFX_DISCARD_INDEX_BUFFER | BGFX_DISCARD_VERTEX_STREAMS | BGFX_DISCARD_TEXTURE_SAMPLERS*/);
		});
	}
	else
	{
		addThreadTask([]()
		{
			bgfx::discard(BGFX_DISCARD_INDEX_BUFFER
				| BGFX_DISCARD_VERTEX_STREAMS
				| BGFX_DISCARD_TEXTURE_SAMPLERS
				| BGFX_DISCARD_COMPUTE);
		});
	}
	cleanResources();
}

void CommandBufferBX::endRenderPass()
{
	LOGFUNC;
}

void CommandBufferBX::endFrame()
{
	LOGFUNC;
	std::vector<bgfx::FrameBufferHandle> unused;
	for (auto& old : _attachmentsOld)
	{
		bool find = false;
		for (auto& val : _attachments)
		{
			if (std::memcmp(&val, &old, sizeof(val)) == 0)
			{
				find = true;
				break;
			}
		}
		if (!find)
			unused.push_back(old.second);
	}
	_attachmentsOld = _attachments;
	_attachments.clear();
	_generatedFBO = BGFX_INVALID_HANDLE;
	addThreadTask([=]()
	{
		for (auto& fbo : unused)
		{
			bgfx::destroy(fbo);
		}
	});
	_print = false;
}

void CommandBufferBX::setLineWidth(float lineWidth)
{
	// TODO: bgfx dose not support line width
}

void CommandBufferBX::setScissorRect(bool isEnabled, float x, float y, float width, float height)
{
	if(isEnabled)
	{
		_scissor[0] = x;
		_scissor[1] = y;
		_scissor[2] = width;
		_scissor[3] = height;
	}
	else
	{
		_scissor[0] = _scissor[1] = _scissor[2] = _scissor[3] = 0;
	}
	_scissorEnabled = isEnabled;
	updateScissor();
}

void CommandBufferBX::setDepthStencilState(DepthStencilState* depthStencilState)
{
	if (depthStencilState)
		_depthStencilStateGL = static_cast<DepthStencilStateBX*>(depthStencilState);
	else
		_depthStencilStateGL = nullptr;
}

void CommandBufferBX::captureScreen(std::function<void(const unsigned char*, int, int)> callback)
{
	addThreadTask([=]()
	{
		CallbackBX::getInstance()->pushCaptureCallback(callback);
		bgfx::requestScreenShot(BGFX_INVALID_HANDLE, "");
	});
}

void CommandBufferBX::prepareDrawing()
{
	const auto program = _renderPipeline->getProgram();
	bindVertexBuffer(program);
	//setUniforms(program);

	_state &= ~DepthStencilStateBX::getStateMask();
	if (_depthStencilStateGL)
	{
		_depthStencilStateGL->apply(_stencilReferenceValueFront, _stencilReferenceValueBack);
		_state |= _depthStencilStateGL->getState();
		if (NEED_LOG) {
			auto& desc = _depthStencilStateGL->_depthStencilInfo.frontFaceStencil;
			CCLOG("set stencil rMask: %x, wMask: %x, ref: %d, func %d, failS: %d, failZ: %d, passZ: %d",
				desc.readMask, desc.writeMask, _stencilReferenceValueFront
				, (int)desc.stencilCompareFunction
				, (int)desc.stencilFailureOperation
				, (int)desc.depthFailureOperation
				, (int)desc.depthStencilPassOperation
			);
		}
	}
	else
	{
		DepthStencilStateBX::reset();
		if (NEED_LOG) { CCLOG("reset stencil state"); }
	}

	_state &= ~BGFX_STATE_CULL_MASK;
	_state |= UtilsBX::toBXStateCull(_cullMode);
	_state &= ~BGFX_STATE_FRONT_CCW;
	_state |= UtilsBX::toBXStateWinding(_winding);
	setUniforms(program);
}

void CommandBufferBX::bindVertexBuffer(ProgramBX* program) const
{
	const auto vertexLayout = _programState->getVertexLayout();
	if (!vertexLayout->isValid())
		return;
	_vertexBuffer->setVertexLayout(*vertexLayout);
}

void CommandBufferBX::setUniforms(ProgramBX* program) const
{
	if (!_programState)
		return;
	auto& callbacks = _programState->getCallbackUniforms();
	for (auto &cb : callbacks)
	{
		cb.second(_programState, cb.first);
	}

	size_t vSize, fSize;
	char* vBuffer = nullptr;
	char* fBuffer = nullptr;
	_programState->getVertexUniformBuffer(&vBuffer, vSize);
	_programState->getFragmentUniformBuffer(&fBuffer, fSize);
	program->applyUniformBuffer(vBuffer, fBuffer);
	program->applyUniformTextures(
		_programState->getVertexTextureInfos(),
		_programState->getFragmentTextureInfos());
}

void CommandBufferBX::cleanResources()
{
	CC_SAFE_RELEASE_NULL(_indexBuffer);
	CC_SAFE_RELEASE_NULL(_vertexBuffer);
	CC_SAFE_RELEASE_NULL(_programState);
}

void CommandBufferBX::applyRenderPassDescriptor(const RenderPassDescriptor& descirptor)
{
	bool useColorAttachmentExternal = descirptor.needColorAttachment && descirptor.colorAttachmentsTexture[0];
	bool useDepthAttachmentExternal = descirptor.depthTestEnabled && descirptor.depthAttachmentTexture;
	bool useStencilAttachmentExternal = descirptor.stencilTestEnabled && descirptor.stencilAttachmentTexture;
	bool useGeneratedFBO = false;
	bool needCreateFBO = false;
	if (useColorAttachmentExternal || useDepthAttachmentExternal || useStencilAttachmentExternal)
	{
		if (!bgfx::isValid(_generatedFBO) || !(descirptor == _lastRPD))
		{
			// need create
			needCreateFBO = true;
		}
		useGeneratedFBO = true;
	}

	if (useGeneratedFBO && needCreateFBO)
	{
		_lastRPD = descirptor;

		Attach attach;
		bgfx::TextureHandle color = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle depth_stencil = BGFX_INVALID_HANDLE;
		// note: bgfx attach texture by its format, so we can only attach one D24S8 texture
		if (useDepthAttachmentExternal &&
			descirptor.depthAttachmentTexture->getTextureFormat() == PixelFormat::D24S8)
		{
			depth_stencil = getHandler(descirptor.depthAttachmentTexture);
			attach.dsTexture = descirptor.depthAttachmentTexture;
		}
		if (!bgfx::isValid(depth_stencil) &&
			useStencilAttachmentExternal &&
			descirptor.stencilAttachmentTexture->getTextureFormat() == PixelFormat::D24S8)
		{
			depth_stencil = getHandler(descirptor.stencilAttachmentTexture);
			attach.dsTexture = descirptor.stencilAttachmentTexture;
		}
		if(useColorAttachmentExternal)
		{
			color = getHandler(descirptor.colorAttachmentsTexture[0]);
			attach.colorTexture = descirptor.colorAttachmentsTexture[0];
		}
		std::vector<bgfx::TextureHandle> textures;
		if(bgfx::isValid(color))
		{
			textures.push_back(color);
		}
		if(bgfx::isValid(depth_stencil))
		{
			textures.push_back(depth_stencil);
		}
		attach.color = color;
		attach.ds = depth_stencil;
		size_t idx = 0;
		for (auto& val : _attachments)
		{
			if (std::memcmp(&val.first, &attach, sizeof(Attach)) == 0)
				break;
			idx++;
		}
		_currentView = idx + 1;
		UtilsBX::setCurrentView(_currentView);
		CCASSERT(_currentView < 128, "too many views");
		if (idx == _attachments.size())
		{
			if (NEED_LOG) { CCLOG("create fbo %d: %d, %d", idx, color.idx, depth_stencil.idx); }
			_generatedFBO = bgfx::createFrameBuffer(textures.size(), textures.data());
			_attachments.emplace_back(attach, _generatedFBO);
			const auto view = _currentView;
			const auto fbo = _generatedFBO;
			addThreadTask([=]()
			{
				bgfx::setViewFrameBuffer(view, fbo);
				bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
				bgfx::setViewRect(view, 0, 0, bgfx::BackbufferRatio::Equal);
				bgfx::setViewScissor(view);
			});
		}
		else
		{
			if (NEED_LOG) { CCLOG("use fbo %d", idx); }
			_generatedFBO = _attachments[idx].second;
		}
	}

	if(useGeneratedFBO)
	{
		_currentFBO = _generatedFBO;
	}
	else
	{
		_currentFBO = _defaultFBO;
		_currentView = 0;
		UtilsBX::setCurrentView(0);
	}

	uint16_t clear = BGFX_CLEAR_NONE;
	uint32_t clearColorValue = 0;
	float clearDepthValue = 1.0;
	uint8_t clearStencilValue = 0;
	if (descirptor.needClearColor)
	{
		clear |= BGFX_CLEAR_COLOR;
		const auto& c = descirptor.clearColorValue;
		clearColorValue = (uint8_t(c[0] * 255) << 24) +
			(uint8_t(c[1] * 255) << 16) +
			(uint8_t(c[2] * 255) << 8) +
			uint8_t(c[3] * 255);
		if (NEED_LOG) { CCLOG("[%d] clear color: %.2f, %.2f, %.2f, %.2f -> 0x%x",
			_currentView, c[0], c[1], c[2], c[3], clearColorValue); }
	}
	if (descirptor.needClearDepth)
	{
		clear |= BGFX_CLEAR_DEPTH;
		clearDepthValue = descirptor.clearDepthValue;
		if (NEED_LOG) { CCLOG("clear depth: %.2f", descirptor.clearDepthValue); }
	}
	if (descirptor.needClearStencil)
	{
		clear |= BGFX_CLEAR_STENCIL;
		clearStencilValue = descirptor.clearStencilValue;
		if(NEED_LOG) { CCLOG("clear stencil: %.2f", descirptor.clearStencilValue); }
	}
	const auto view = _currentView;
	if (!_scissorEnabled)
	{
		addThreadTask([=]()
		{
			bgfx::setScissor();
			bgfx::setViewClear(view, clear, clearColorValue, clearDepthValue, clearStencilValue);
			bgfx::touch(view);
			//bgfx::submit(view, BGFX_INVALID_HANDLE, 0, BGFX_DISCARD_NONE);
		});
	}
	else
	{
		std::array<float, 4> scissor = { { _scissor[0],_scissor[1],_scissor[2],_scissor[3] } };
		addThreadTask([=]()
		{
			bgfx::setScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
			bgfx::setViewClear(view, clear, clearColorValue, clearDepthValue, clearStencilValue);
			bgfx::touch(view);
			//bgfx::submit(view, BGFX_INVALID_HANDLE, 0, BGFX_DISCARD_NONE);
		});
	}
}

void CommandBufferBX::updateScissor()
{
	if (_scissorEnabled)
	{
		auto left = std::min(_scissor[0], _scissor[0] + _scissor[2]);
		auto bottom = std::min(_scissor[1], _scissor[1] + _scissor[3]);
		auto right = left + std::abs(_scissor[2]);
		auto top = bottom + std::abs(_scissor[3]);
		left = std::max(left, (float)_viewPort.x);
		bottom = std::max(bottom, (float)_viewPort.y);
		right = std::min(right, (float)(_viewPort.x + _viewPort.w));
		top = std::min(top, (float)(_viewPort.y + _viewPort.h));
		_scissorRect.setRect(left, bottom, right - left, top - bottom);
		if (NEED_LOG) {
			CCLOG("scissor: %.1f, %.1f, %.1f, %.1f", _scissor[0], _scissor[1], _scissor[2], _scissor[3]);
			CCLOG("vp: %d, %d, %d, %d", _viewPort.x, _viewPort.y, _viewPort.w, _viewPort.h);
			CCLOG("scissor rect: %.1f, %.1f, %.1f, %.1f",
				_scissorRect.origin.x, _scissorRect.origin.y, _scissorRect.size.width, _scissorRect.size.height);
		}
	}
	else
	{
		_scissorRect.setRect((float)_viewPort.x, (float)_viewPort.y, (float)_viewPort.w, (float)_viewPort.h);
		if (NEED_LOG) {
			CCLOG("scissor rect: %.1f, %.1f, %.1f, %.1f",
				_scissorRect.origin.x, _scissorRect.origin.y, _scissorRect.size.width, _scissorRect.size.height);
		}
	}
}

CC_BACKEND_END
