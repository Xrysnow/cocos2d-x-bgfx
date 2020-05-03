#include "ProgramBX.h"
#include "renderer/backend/Texture.h"
#include "ShaderModuleBX.h"
#include "TextureBX.h"
#include "UtilsBX.h"
#include "base/CCConsole.h"
#include "3d/CC3DProgramInfo.h"
#include "CCDirector.h"
#include <unordered_set>

using namespace bgfx;

CC_BACKEND_BEGIN

ProgramBX::ProgramBX(const std::string& vertexShader, const std::string& fragmentShader)
: Program(vertexShader, fragmentShader)
{
	_handle = BGFX_INVALID_HANDLE;
	_vertexShaderModule = (ShaderModuleBX*)(ShaderCache::newVertexShaderModule(_vertexShader));
	_fragmentShaderModule = (ShaderModuleBX*)(ShaderCache::newFragmentShaderModule(_fragmentShader));
	CC_SAFE_RETAIN(_vertexShaderModule);
	CC_SAFE_RETAIN(_fragmentShaderModule);
	if (_vertexShaderModule
		&& _fragmentShaderModule
		&& isValid(_vertexShaderModule->getHandle())
		&& isValid(_fragmentShaderModule->getHandle()))
	{
		compileProgram();
		if (isValid(_handle))
		{		
			computeUniformInfos();
			computeLocations();
		}
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
	_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED,
		[this](EventCustom*)
	{
		this->reloadProgram();
	});
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
		_backToForegroundListener, -1);
#endif
}

ProgramBX::ProgramBX(const std::string& vertexShader, const std::string& fragmentShader, const std::string& varying,
	const std::vector<std::string>& defines, const std::vector<std::string>& includes)
: Program(vertexShader, fragmentShader)
{
	_handle = BGFX_INVALID_HANDLE;
	_vertexShaderModule = new ShaderModuleBX(ShaderStage::VERTEX, _vertexShader, varying, defines, includes);
	_fragmentShaderModule = new ShaderModuleBX(ShaderStage::FRAGMENT, _fragmentShader, varying, defines, includes);
	_vertexShaderModule->autorelease();
	_fragmentShaderModule->autorelease();
	CC_SAFE_RETAIN(_vertexShaderModule);
	CC_SAFE_RETAIN(_fragmentShaderModule);
	if (_vertexShaderModule
		&& _fragmentShaderModule
		&& isValid(_vertexShaderModule->getHandle())
		&& isValid(_fragmentShaderModule->getHandle()))
	{
		compileProgram();
		if (isValid(_handle))
		{
			computeUniformInfos();
			computeLocations();
		}
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
	_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED,
		[this](EventCustom*)
	{
		this->reloadProgram();
	});
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
		_backToForegroundListener, -1);
#endif
}

ProgramBX::~ProgramBX()
{
	CC_SAFE_RELEASE(_vertexShaderModule);
	CC_SAFE_RELEASE(_fragmentShaderModule);
	if (isValid(_handle))
	{
		destroy(_handle);
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
	Director::getInstance()->getEventDispatcher()->removeEventListener(
		_backToForegroundListener);
#endif
}

UniformLocation ProgramBX::getUniformLocation(const std::string& uniform) const
{
	const auto it = _locations.find(uniform);
	if (it != _locations.end())
		return it->second;
	return {};
}

UniformLocation ProgramBX::getUniformLocation(Uniform name) const
{
	if (name >= UNIFORM_MAX)
		return {};
	return _builtinUniformLocation[name];
}

int ProgramBX::getAttributeLocation(const std::string& name) const
{
	return 0;
}

int ProgramBX::getAttributeLocation(Attribute name) const
{
	return 0;
}

int ProgramBX::getMaxVertexLocation() const
{
	return 0;
}

int ProgramBX::getMaxFragmentLocation() const
{
	return 0;
}

const std::unordered_map<std::string, AttributeBindInfo> ProgramBX::getActiveAttributes() const
{
	// make all built-in attributes available
	static std::unordered_map<std::string, AttributeBindInfo> attr = {};
	if(attr.empty())
	{
		for (auto& name : {
			ATTRIBUTE_NAME_COLOR,
			ATTRIBUTE_NAME_POSITION,
			ATTRIBUTE_NAME_TEXCOORD,
			ATTRIBUTE_NAME_TEXCOORD1,
			ATTRIBUTE_NAME_TEXCOORD2,
			ATTRIBUTE_NAME_TEXCOORD3,
			shaderinfos::attribute::ATTRIBUTE_NAME_NORMAL,
			shaderinfos::attribute::ATTRIBUTE_NAME_BLEND_WEIGHT,
			shaderinfos::attribute::ATTRIBUTE_NAME_BLEND_INDEX,
			shaderinfos::attribute::ATTRIBUTE_NAME_TANGENT,
			shaderinfos::attribute::ATTRIBUTE_NAME_BINORMAL
			})
		{
			AttributeBindInfo info = {};
			info.attributeName = name;
			info.location = 0;
			attr[name] = info;
		}
	}
	return attr;
}

std::size_t ProgramBX::getUniformBufferSize(ShaderStage stage) const
{
	switch (stage)
	{
	case ShaderStage::VERTEX: return _vertBufferSize;
	case ShaderStage::FRAGMENT: return _fragBufferSize;
	case ShaderStage::VERTEX_AND_FRAGMENT: return _vertBufferSize + _fragBufferSize;
	default: ;
	}
	return 0;
}

const UniformInfo& ProgramBX::getActiveUniformInfo(ShaderStage stage, int location) const
{
	// should not be used
	static UniformInfo temp = {};
	return temp;
}

const std::unordered_map<std::string, UniformInfo>& ProgramBX::getAllActiveUniformInfo(ShaderStage stage) const
{
	switch (stage)
	{
	case ShaderStage::VERTEX: return _vertInfos;
	case ShaderStage::FRAGMENT: return _fragInfos;
	case ShaderStage::VERTEX_AND_FRAGMENT: return _infos;
	default: ;
	}
	static std::unordered_map<std::string, UniformInfo> temp = {};
	return temp;
}

void ProgramBX::setUniformAsInt(const std::string& name)
{
	// use UniformInfo::needConvert
	const auto it1 = _vertInfos.find(name);
	if (it1 != _vertInfos.end())
		it1->second.needConvert = true;
	const auto it2 = _fragInfos.find(name);
	if (it2 != _fragInfos.end())
		it2->second.needConvert = true;
	const auto it3 = _infos.find(name);
	if (it3 != _infos.end())
		it3->second.needConvert = true;
}

void ProgramBX::applyUniformBuffer(const char* vertBuffer, const char* fragBuffer)
{
	if (!isValid(_handle))
		return;
	if (vertBuffer)
	{
		for(auto& it : _vertInfos)
		{
			if (it.second.type != UniformType::Sampler)
			{
				const UniformHandle hdl = { uint16_t(it.second.location) };
				const auto count = it.second.count;
				const auto bufferOffset = it.second.bufferOffset;
				if (!it.second.needConvert)
				{
					addThreadTask([=]()
					{
						setUniform(hdl, vertBuffer + bufferOffset, count);
					});
				}
				else
				{
					// convert int to float since bgfx only supports float
					const auto buf = new float[count]();
					std::shared_ptr<float> ptr(buf, std::default_delete<float[]>());
					const auto arr = (int32_t*)(vertBuffer + bufferOffset);
					for (int i = 0; i < count; ++i)
						buf[i] = float(arr[i]);
					addThreadTask([=]()
					{
						setUniform(hdl, ptr.get(), count);
					});
				}
			}
		}
	}
	if (fragBuffer)
	{
		for (auto& it : _fragInfos)
		{
			if (it.second.type != UniformType::Sampler)
			{
				const UniformHandle hdl = { uint16_t(it.second.location) };
				const auto count = it.second.count;
				const auto bufferOffset = it.second.bufferOffset;
				if (!it.second.needConvert)
				{
					addThreadTask([=]()
					{
						setUniform(hdl, fragBuffer + bufferOffset, count);
					});
				}
				else
				{
					// convert int to float
					const auto buf = new float[count]();
					std::shared_ptr<float> ptr(buf, std::default_delete<float[]>());
					const auto arr = (int32_t*)(fragBuffer + bufferOffset);
					for (int i = 0; i < count; ++i)
						buf[i] = float(arr[i]);
					//setUniform(hdl, buf, count);
					addThreadTask([=]()
					{
						setUniform(hdl, ptr.get(), count);
					});
					//delete[] buf;
				}
			}
		}
	}
}

void ProgramBX::applyUniformTextures(
	const std::unordered_map<int, TextureInfo>& vertTextures,
	const std::unordered_map<int, TextureInfo>& fragTextures)
{
	if (!isValid(_handle))
		return;
	for (auto& textures : { vertTextures,fragTextures })
	{
		for (auto& it : textures)
		{
			const UniformHandle hdl = { uint16_t(it.first) };
			for (size_t i = 0; i < it.second.slot.size(); ++i)
			{
				TextureHandle t = BGFX_INVALID_HANDLE;
				const auto tex = it.second.textures[i];
				const auto slot = it.second.slot[i];
				switch (tex->getTextureType())
				{
				case TextureType::TEXTURE_2D:
					t = ((Texture2DBX*)tex)->getHandle();
					((Texture2DBX*)tex)->apply(slot);
					break;
				case TextureType::TEXTURE_CUBE:
					t = ((TextureCubeBX*)tex)->getHandle();
					((TextureCubeBX*)tex)->apply(slot);
					break;
				default: ;
				}
				if(!isValid(t))
					continue;
				const auto stage = uint8_t(it.second.slot[i]);
				addThreadTask([=]()
				{
					setTexture(stage, hdl, t);
				});
			}
		}		
	}
}

void ProgramBX::compileProgram()
{
	if (!_vertexShaderModule || !_fragmentShaderModule)
		return;
	const auto vertShader = _vertexShaderModule->getHandle();
	const auto fragShader = _fragmentShaderModule->getHandle();
	if (!isValid(vertShader) || !isValid(fragShader))
		return;
	_handle = createProgram(vertShader, fragShader);
	if (!isValid(_handle))
	{
		cocos2d::log("cocos2d: ERROR: %s: failed to link program", __FUNCTION__);
	}
}

//bool ProgramBX::getAttributeLocation(const std::string& attributeName, unsigned& location) const
//{
//	return true;
//}

void ProgramBX::computeUniformInfos()
{
	if (!isValid(_handle))
		return;
	const auto vertShader = _vertexShaderModule->getHandle();
	const auto fragShader = _fragmentShaderModule->getHandle();
	const auto numVert = getShaderUniforms(vertShader);
	const auto numFrag = getShaderUniforms(fragShader);
	_vertBufferSize = 0;
	_fragBufferSize = 0;
	_vertInfos.clear();
	_fragInfos.clear();
	_infos.clear();

	static std::unordered_set<std::string> predefined = {
		"u_viewRect",
		"u_viewTexel",
		"u_view",
		"u_invView",
		"u_proj",
		"u_invProj",
		"u_viewProj",
		"u_invViewProj",
		"u_model",
		"u_modelView",
		"u_modelViewProj",
		"u_alphaRef4",
		"u_vpTransform"
	};
	if (numVert > 0)
	{		
		auto uVert = new UniformHandle[numVert];
		getShaderUniforms(vertShader, uVert, numVert);
		bgfx::UniformInfo info{};
		uint32_t offset = 0;
		for (uint16_t i = 0; i < numVert; ++i)
		{
			auto& hdl = uVert[i];
			std::memset(info.name, 0, sizeof(info.name));
			getUniformInfo(hdl, info);
			info.name[sizeof(info.name) - 1] = '\0';
			const auto typeSize = UtilsBX::getUniformTypeSize(info.type);
			if (typeSize > 0)
			{
				std::string name = info.name;
				if (predefined.find(name) != predefined.end())
					continue;
				UniformInfo inf;
				inf.location = (int)hdl.idx;
				inf.count = info.num == 0 ? 1 : info.num;
				inf.type = (unsigned int)info.type;
				inf.isArray = inf.count > 1;
				inf.size = typeSize * inf.count;
				if (info.type == UniformType::Sampler)
				{
					inf.bufferOffset = 0;
				}
				else
				{
					inf.bufferOffset = offset;
					offset += inf.size;
				}
				_vertInfos[name] = inf;
			}
			else
			{
				CCLOG("unknown uniform: %s, type: %d", info.name, (int)info.type);
			}
		}
		_vertBufferSize = offset;
		delete[] uVert;
	}
	if (numFrag > 0)
	{
		auto uFrag = new UniformHandle[numFrag];
		getShaderUniforms(fragShader, uFrag, numFrag);
		bgfx::UniformInfo info{};
		uint32_t offset = 0;
		for (uint16_t i = 0; i < numFrag; ++i)
		{
			auto& hdl = uFrag[i];
			std::memset(info.name, 0, sizeof(info.name));
			getUniformInfo(hdl, info);
			info.name[sizeof(info.name) - 1] = '\0';
			const auto typeSize = UtilsBX::getUniformTypeSize(info.type);
			if (typeSize > 0)
			{
				std::string name = info.name;
				if (predefined.find(name) != predefined.end())
					continue;
				UniformInfo inf;
				inf.location = (int)hdl.idx;
				inf.count = info.num == 0 ? 1 : info.num;
				inf.type = (unsigned int)info.type;
				inf.isArray = inf.count > 1;
				inf.size = typeSize * inf.count;
				if (info.type == UniformType::Sampler)
				{
					inf.bufferOffset = 0;
				}
				else
				{
					inf.bufferOffset = offset;
					offset += inf.size;
				}
				_vertInfos[name] = inf;
			}
			else
			{
				CCLOG("unknown uniform: %s, type: %d", info.name, (int)info.type);
			}
		}
		_fragBufferSize = offset;
		delete[] uFrag;
	}
	// uniform with same name in two shaders should have same info
	for (auto& it : _vertInfos)
		_infos[it.first] = it.second;
	for (auto& it : _fragInfos)
		_infos[it.first] = it.second;

	for (auto& it : _vertInfos)
	{
		UniformLocation loc;
		loc.shaderStage = ShaderStage::VERTEX;
		if(it.second.type== UniformType::Sampler)
			loc.location[0] = it.second.location;
		else
			loc.location[0] = it.second.bufferOffset;
		_locations[it.first] = loc;
	}
	for (auto& it : _fragInfos)
	{
		const auto itLoc = _locations.find(it.first);
		if (itLoc != _locations.end())
		{
			itLoc->second.shaderStage = ShaderStage::VERTEX_AND_FRAGMENT;
			if (it.second.type == UniformType::Sampler)
				itLoc->second.location[1] = it.second.location;
			else
				itLoc->second.location[1] = it.second.bufferOffset;
		}
		else
		{
			UniformLocation loc;
			loc.shaderStage = ShaderStage::FRAGMENT;
			if (it.second.type == UniformType::Sampler)
				loc.location[0] = it.second.location;
			else
				loc.location[1] = it.second.bufferOffset;
			_locations[it.first] = loc;
		}
	}
	setBuiltinUniform(UNIFORM_NAME_MVP_MATRIX, MVP_MATRIX);
	setBuiltinUniform(UNIFORM_NAME_TEXT_COLOR, TEXT_COLOR);
	setBuiltinUniform(UNIFORM_NAME_EFFECT_COLOR, EFFECT_COLOR);
	setBuiltinUniform(UNIFORM_NAME_EFFECT_TYPE, EFFECT_TYPE);
	setBuiltinUniform(UNIFORM_NAME_TEXTURE, TEXTURE);
	setBuiltinUniform(UNIFORM_NAME_TEXTURE1, TEXTURE1);
	// some uniforms are set as int
	setUniformAsInt(UNIFORM_NAME_EFFECT_TYPE);
	setUniformAsInt("u_has_alpha");
	setUniformAsInt("u_has_light_map");
}

void ProgramBX::computeLocations()
{
	// attributes are managed by bgfx
}

void ProgramBX::setBuiltinUniform(const std::string& name, Uniform type)
{
	const auto it = _locations.find(name);
	if (it != _locations.end())
		_builtinUniformLocation[type] = it->second;
}

#if CC_ENABLE_CACHE_TEXTURE_DATA
void ProgramGL::reloadProgram()
{
	static_cast<ShaderModuleGL*>(_vertexShaderModule)->compileShader(backend::ShaderStage::VERTEX, _vertexShader);
	static_cast<ShaderModuleGL*>(_fragmentShaderModule)->compileShader(backend::ShaderStage::FRAGMENT, _fragmentShader);
	compileProgram();
}
#endif

CC_BACKEND_END
