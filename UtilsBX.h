#pragma once
#include "renderer/backend/Types.h"
#include "renderer/backend/VertexLayout.h"
#include "ThreadBX.hpp"
#include "bgfx/bgfx.h"

#define BACKEND_VIEW UtilsBX::getCurrentView()

CC_BACKEND_BEGIN

class UtilsBX
{
public:
	static uint8_t getVertexFormatSize(VertexFormat vertexFormat);
	static uint32_t getUniformTypeSize(bgfx::UniformType::Enum uniformType);
	
	static bgfx::Attrib::Enum toBXAttrib(const std::string& name);
	static std::pair<bgfx::AttribType::Enum, uint8_t> toBXAttribType(VertexFormat vertexFormat);
	static bgfx::VertexLayout toBXVertexLayout(const VertexLayout& vertexLayout);

	static bgfx::TextureFormat::Enum toBXTextureFormat(PixelFormat pixelFormat, bool* isCompressed = nullptr);
	static uint64_t toBXStatePrimitiveType(PrimitiveType primitiveType);
	
	static uint64_t toBXStateBlendFac(BlendFactor blendFactor);
	static uint64_t toBXStateBlendFunc(BlendFactor src, BlendFactor dst);
	static uint64_t toBXStateBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA);
	
	static uint64_t toBXStateBlendOp(BlendOperation blendOperation);
	static uint64_t toBXStateBlendEquation(BlendOperation blendOperation);
	static uint64_t toBXStateBlendEquation(BlendOperation blendOpRGB, BlendOperation blendOpA);

	static uint64_t toBXStateBlend(const BlendDescriptor& blendDescriptor);

	static uint64_t toBXStateWriteRGBA(ColorWriteMask colorWriteMask);
	static uint64_t toBXStateCull(CullMode cullMode);
	static uint64_t toBXStateWinding(Winding winding);

	static uint64_t toBXStateDepthTest(CompareFunction compareFunction);
	static uint32_t toBXStencilTest(CompareFunction compareFunction);
	static uint32_t toBXStencilOpFailS(StencilOperation stencilOperation);
	static uint32_t toBXStencilOpFailZ(StencilOperation stencilOperation);
	static uint32_t toBXStencilOpPassZ(StencilOperation stencilOperation);

	static uint32_t toBXSamplerMag(SamplerFilter samplerFilter);
	static uint32_t toBXSamplerMin(SamplerFilter samplerFilter, bool hasMipmaps, bool isPow2);
	static uint32_t toBXSamplerU(SamplerAddressMode samplerAddressMode, bool isPow2);
	static uint32_t toBXSamplerV(SamplerAddressMode samplerAddressMode, bool isPow2);
	static uint32_t toBXSamplerUV(SamplerAddressMode samplerAddressMode, bool isPow2);
	static uint32_t toBXSampler(const SamplerDescriptor& samplerDescriptor, bool hasMipmaps, bool isPow2);

	static void setCurrentView(bgfx::ViewId id);
	static bgfx::ViewId getCurrentView();
};

ThreadPool& getThreadPool();
void addThreadTask(const std::function<void()>& task);
void addThreadTaskSync(const std::function<void()>& task);

CC_BACKEND_END
