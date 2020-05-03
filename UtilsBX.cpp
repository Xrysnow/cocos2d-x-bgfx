#include "UtilsBX.h"
#include "CCConsole.h"
#include <map>

CC_BACKEND_BEGIN

namespace
{
	std::unordered_map<std::string, bgfx::Attrib::Enum> bgfxAttribMap =
	{
		{ "a_position", bgfx::Attrib::Position },
		{ "a_normal", bgfx::Attrib::Normal },
		{ "a_tangent", bgfx::Attrib::Tangent },
		{ "a_bitangent", bgfx::Attrib::Bitangent },
		{ "a_color0", bgfx::Attrib::Color0 },
		{ "a_color1", bgfx::Attrib::Color1 },
		{ "a_color2", bgfx::Attrib::Color2 },
		{ "a_color3", bgfx::Attrib::Color3 },
		{ "a_indices", bgfx::Attrib::Indices },
		{ "a_weight", bgfx::Attrib::Weight },
		{ "a_texcoord0", bgfx::Attrib::TexCoord0 },
		{ "a_texcoord1", bgfx::Attrib::TexCoord1 },
		{ "a_texcoord2", bgfx::Attrib::TexCoord2 },
		{ "a_texcoord3", bgfx::Attrib::TexCoord3 },
		{ "a_texcoord4", bgfx::Attrib::TexCoord4 },
		{ "a_texcoord5", bgfx::Attrib::TexCoord5 },
		{ "a_texcoord6", bgfx::Attrib::TexCoord6 },
		{ "a_texcoord7", bgfx::Attrib::TexCoord7 },
		// built-in
		{ ATTRIBUTE_NAME_COLOR, bgfx::Attrib::Color0 },
		{ ATTRIBUTE_NAME_TEXCOORD, bgfx::Attrib::TexCoord0 },
		{ ATTRIBUTE_NAME_TEXCOORD1, bgfx::Attrib::TexCoord1 },
		{ ATTRIBUTE_NAME_TEXCOORD2, bgfx::Attrib::TexCoord2 },
		{ ATTRIBUTE_NAME_TEXCOORD3, bgfx::Attrib::TexCoord3 },
		// SKINPOSITION_TEXTURE_3D
		{ "a_blendWeight", bgfx::Attrib::Color0 },
		{ "a_blendIndex", bgfx::Attrib::Color1 },
		// SKINPOSITION_NORMAL_TEXTURE_3D
		{ "a_binormal", bgfx::Attrib::TexCoord1 },
	};
	std::unordered_map<VertexFormat, std::pair<bgfx::AttribType::Enum, uint8_t>> bgfxAttribTypeMap =
	{
		{ VertexFormat::FLOAT4, { bgfx::AttribType::Float, 4 } },
		{ VertexFormat::FLOAT3, { bgfx::AttribType::Float, 3 } },
		{ VertexFormat::FLOAT2, { bgfx::AttribType::Float, 2 } },
		{ VertexFormat::FLOAT, { bgfx::AttribType::Float, 1 } },
		{ VertexFormat::INT4, { bgfx::AttribType::Float, 4 } },
		{ VertexFormat::INT3, { bgfx::AttribType::Float, 3 } },
		{ VertexFormat::INT2, { bgfx::AttribType::Float, 2 } },
		{ VertexFormat::INT, { bgfx::AttribType::Float, 1 } },
		{ VertexFormat::USHORT4, { bgfx::AttribType::Int16, 4 } },
		{ VertexFormat::USHORT2, { bgfx::AttribType::Int16, 2 } },
		{ VertexFormat::UBYTE4, { bgfx::AttribType::Uint8, 4 } },
	};
}

uint8_t UtilsBX::getVertexFormatSize(VertexFormat vertexFormat)
{
	switch (vertexFormat)
	{
	case VertexFormat::FLOAT4: return 4 * 4;
	case VertexFormat::FLOAT3: return 4 * 3;
	case VertexFormat::FLOAT2: return 4 * 2;
	case VertexFormat::FLOAT: return 4 * 1;
	case VertexFormat::INT4: return 4 * 4;
	case VertexFormat::INT3: return 4 * 3;
	case VertexFormat::INT2: return 4 * 2;
	case VertexFormat::INT: return 4 * 1;
	case VertexFormat::USHORT4: return 2 * 4;
	case VertexFormat::USHORT2: return 2 * 2;
	case VertexFormat::UBYTE4: return 1 * 4;
	default: ;
	}
	return 0;
}

uint32_t UtilsBX::getUniformTypeSize(bgfx::UniformType::Enum uniformType)
{
	switch (uniformType)
	{
	case bgfx::UniformType::Sampler: return sizeof(bgfx::TextureHandle);
	case bgfx::UniformType::End: break;
	case bgfx::UniformType::Vec4: return sizeof(float) * 4;
	case bgfx::UniformType::Mat3: return sizeof(float) * 9;
	case bgfx::UniformType::Mat4: return sizeof(float) * 16;
	case bgfx::UniformType::Count: break;
	default: ;
	}
	return 0;
}

bgfx::Attrib::Enum UtilsBX::toBXAttrib(const std::string& name)
{
	const auto it = bgfxAttribMap.find(name);
	if (it != bgfxAttribMap.end())
		return it->second;
	return bgfx::Attrib::Count;
}

std::pair<bgfx::AttribType::Enum, uint8_t> UtilsBX::toBXAttribType(VertexFormat vertexFormat)
{
	const auto it = bgfxAttribTypeMap.find(vertexFormat);
	if (it != bgfxAttribTypeMap.end())
		return it->second;
	return { bgfx::AttribType::Count, 0 };
}

bgfx::VertexLayout UtilsBX::toBXVertexLayout(const VertexLayout& vertexLayout)
{
	bgfx::VertexLayout ret;
	ret.begin();
	std::map<std::size_t, VertexLayout::Attribute> attribs;
	for (auto& it : vertexLayout.getAttributes())
	{
		attribs[it.second.offset] = it.second;
	}
	for (auto& it : attribs)
	{
		const auto attr = toBXAttrib(it.second.name);
		const auto type = toBXAttribType(it.second.format);
		if (attr == bgfx::Attrib::Count || type.second == 0)
		{
			ret.skip(getVertexFormatSize(it.second.format));
			CCLOG("invalid attribute");
		}
		else
		{
			ret.add(attr, type.second, type.first, it.second.needToBeNormallized);
			// check offset
			if (ret.getOffset(attr) != it.second.offset)
			{
				ret.m_offset[attr] = it.second.offset;
				CCLOG("offset mismatch");
			}
		}
	}
	if (attribs.empty())
	{
		CCLOG("WARNING: attributes of VertexLayout is empty");
		assert(false);
	}
	// check stride
	if (ret.getStride() != vertexLayout.getStride())
	{
		CCLOG("stride mismatch, expect %d, got %d", vertexLayout.getStride(), ret.m_stride);
		assert(false);
		ret.m_stride = vertexLayout.getStride();
	}
	ret.end();
	return ret;
}

bgfx::TextureFormat::Enum UtilsBX::toBXTextureFormat(PixelFormat pixelFormat, bool* isCompressed)
{
	if (isCompressed)
		*isCompressed = PixelFormat::PVRTC4 <= pixelFormat && pixelFormat <= PixelFormat::ATC_INTERPOLATED_ALPHA;
	switch (pixelFormat)
	{
	case PixelFormat::AUTO: break;
	case PixelFormat::BGRA8888: return bgfx::TextureFormat::BGRA8;
	case PixelFormat::RGBA8888: return bgfx::TextureFormat::RGBA8;
	case PixelFormat::RGB888: return bgfx::TextureFormat::RGB8;
	case PixelFormat::RGB565: return bgfx::TextureFormat::R5G6B5;
	case PixelFormat::A8: return bgfx::TextureFormat::A8;
	case PixelFormat::I8: return bgfx::TextureFormat::R8;
	case PixelFormat::AI88: return bgfx::TextureFormat::RG8; // note: this may not supported in GLES
	case PixelFormat::RGBA4444: return bgfx::TextureFormat::RGBA4;
	case PixelFormat::RGB5A1: return bgfx::TextureFormat::RGB5A1;
	case PixelFormat::PVRTC4: return bgfx::TextureFormat::PTC14;
	case PixelFormat::PVRTC4A: return bgfx::TextureFormat::PTC14A;
	case PixelFormat::PVRTC2: return bgfx::TextureFormat::PTC12;
	case PixelFormat::PVRTC2A: return bgfx::TextureFormat::PTC12A;
	case PixelFormat::ETC: return bgfx::TextureFormat::ETC1;
	case PixelFormat::S3TC_DXT1: return bgfx::TextureFormat::BC1;
	case PixelFormat::S3TC_DXT3: return bgfx::TextureFormat::BC2;
	case PixelFormat::S3TC_DXT5: return bgfx::TextureFormat::BC3;
	case PixelFormat::ATC_RGB: return bgfx::TextureFormat::ATC;
	case PixelFormat::ATC_EXPLICIT_ALPHA: return bgfx::TextureFormat::ATCE;
	case PixelFormat::ATC_INTERPOLATED_ALPHA: return bgfx::TextureFormat::ATCI;
	case PixelFormat::MTL_B5G6R5: return bgfx::TextureFormat::R5G6B5; // already mapped in bgfx
	case PixelFormat::MTL_BGR5A1: return bgfx::TextureFormat::RGB5A1;
	case PixelFormat::MTL_ABGR4: return bgfx::TextureFormat::RGBA4;
	case PixelFormat::D24S8: return bgfx::TextureFormat::D24S8;
	//case PixelFormat::DEFAULT: break;
	case PixelFormat::NONE: break;
	default: ;
	}
	return bgfx::TextureFormat::RGBA8;
}

uint64_t UtilsBX::toBXStatePrimitiveType(PrimitiveType primitiveType)
{
	// note: will be triangles if not set
	switch (primitiveType)
	{
	case PrimitiveType::POINT: return BGFX_STATE_PT_POINTS;
	case PrimitiveType::LINE: return BGFX_STATE_PT_LINES;
	case PrimitiveType::LINE_STRIP: return BGFX_STATE_PT_LINESTRIP;
	case PrimitiveType::TRIANGLE_STRIP: return BGFX_STATE_PT_TRISTRIP;
	default: ;
	}
	return 0;
}

uint64_t UtilsBX::toBXStateBlendFac(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case BlendFactor::ZERO: return BGFX_STATE_BLEND_ZERO;
	case BlendFactor::ONE: return BGFX_STATE_BLEND_ONE;
	case BlendFactor::SRC_COLOR: return BGFX_STATE_BLEND_SRC_COLOR;
	case BlendFactor::ONE_MINUS_SRC_COLOR: return BGFX_STATE_BLEND_INV_SRC_COLOR;
	case BlendFactor::SRC_ALPHA: return BGFX_STATE_BLEND_SRC_ALPHA;
	case BlendFactor::ONE_MINUS_SRC_ALPHA: return BGFX_STATE_BLEND_INV_SRC_ALPHA;
	case BlendFactor::DST_COLOR: return BGFX_STATE_BLEND_DST_COLOR;
	case BlendFactor::ONE_MINUS_DST_COLOR: return BGFX_STATE_BLEND_INV_DST_COLOR;
	case BlendFactor::DST_ALPHA: return BGFX_STATE_BLEND_DST_ALPHA;
	case BlendFactor::ONE_MINUS_DST_ALPHA: return BGFX_STATE_BLEND_INV_DST_ALPHA;
	case BlendFactor::SRC_ALPHA_SATURATE: return BGFX_STATE_BLEND_SRC_ALPHA_SAT;
	// not used and not supported
	//case BlendFactor::CONSTANT_ALPHA: return BGFX_STATE_BLEND_FACTOR;
	//case BlendFactor::ONE_MINUS_CONSTANT_ALPHA: return BGFX_STATE_BLEND_INV_FACTOR;
	// will be GL_CONSTANT_COLOR/MTLBlendFactorBlendColor
	case BlendFactor::BLEND_CLOLOR: return BGFX_STATE_BLEND_FACTOR;
	// missing INV_FACTOR
	case (BlendFactor)(14): return BGFX_STATE_BLEND_INV_FACTOR;
	default: ;
	}
	return BGFX_STATE_BLEND_ONE;
}

uint64_t UtilsBX::toBXStateBlendFunc(BlendFactor src, BlendFactor dst)
{
	return BGFX_STATE_BLEND_FUNC(toBXStateBlendFac(src), toBXStateBlendFac(dst));
}

uint64_t UtilsBX::toBXStateBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcA, BlendFactor dstA)
{
	return BGFX_STATE_BLEND_FUNC_SEPARATE(
		toBXStateBlendFac(srcRGB), toBXStateBlendFac(dstRGB),
		toBXStateBlendFac(srcA), toBXStateBlendFac(dstA));
}

uint64_t UtilsBX::toBXStateBlendOp(BlendOperation blendOperation)
{
	switch (blendOperation)
	{
	case BlendOperation::ADD: return BGFX_STATE_BLEND_EQUATION_ADD;
	case BlendOperation::SUBTRACT: return BGFX_STATE_BLEND_EQUATION_SUB;
	case BlendOperation::RESERVE_SUBTRACT: return BGFX_STATE_BLEND_EQUATION_REVSUB;
	// missing MIN, MAX
	case (BlendOperation)(3) : return BGFX_STATE_BLEND_EQUATION_MIN;
	case (BlendOperation)(4) : return BGFX_STATE_BLEND_EQUATION_MAX;
	default:;
	}
	return BGFX_STATE_BLEND_EQUATION_ADD;
}

uint64_t UtilsBX::toBXStateBlendEquation(BlendOperation blendOperation)
{
	return BGFX_STATE_BLEND_EQUATION(toBXStateBlendOp(blendOperation));
}

uint64_t UtilsBX::toBXStateBlendEquation(BlendOperation blendOpRGB, BlendOperation blendOpA)
{
	return BGFX_STATE_BLEND_EQUATION_SEPARATE(toBXStateBlendOp(blendOpRGB), toBXStateBlendOp(blendOpA));
}

uint64_t UtilsBX::toBXStateBlend(const BlendDescriptor& blendDescriptor)
{
	if (!blendDescriptor.blendEnabled)
		return 0;
	return 0 |
		toBXStateWriteRGBA(blendDescriptor.writeMask) |
		toBXStateBlendEquation(blendDescriptor.rgbBlendOperation, blendDescriptor.alphaBlendOperation) |
		toBXStateBlendFunc(
			blendDescriptor.sourceRGBBlendFactor, blendDescriptor.destinationRGBBlendFactor,
			blendDescriptor.sourceAlphaBlendFactor, blendDescriptor.destinationAlphaBlendFactor);
}

uint64_t UtilsBX::toBXStateWriteRGBA(ColorWriteMask colorWriteMask)
{
	uint64_t state = 0;
	if ((uint32_t)colorWriteMask & (uint32_t)ColorWriteMask::RED)
		state |= BGFX_STATE_WRITE_R;
	if ((uint32_t)colorWriteMask & (uint32_t)ColorWriteMask::GREEN)
		state |= BGFX_STATE_WRITE_G;
	if ((uint32_t)colorWriteMask & (uint32_t)ColorWriteMask::BLUE)
		state |= BGFX_STATE_WRITE_B;
	if ((uint32_t)colorWriteMask & (uint32_t)ColorWriteMask::ALPHA)
		state |= BGFX_STATE_WRITE_A;
	return state;
}

uint64_t UtilsBX::toBXStateCull(CullMode cullMode)
{
	// note: will disable cull if not set
	switch (cullMode)
	{
	case CullMode::BACK: return BGFX_STATE_CULL_CCW;
	case CullMode::FRONT: return BGFX_STATE_CULL_CW;
	default: ;
	}
	return 0;
}

uint64_t UtilsBX::toBXStateWinding(Winding winding)
{
	if (winding == Winding::COUNTER_CLOCK_WISE)
		return BGFX_STATE_FRONT_CCW;
	return 0;
}

uint64_t UtilsBX::toBXStateDepthTest(CompareFunction compareFunction)
{
	switch (compareFunction)
	{
	case CompareFunction::NEVER: return BGFX_STATE_DEPTH_TEST_NEVER;
	case CompareFunction::LESS: return BGFX_STATE_DEPTH_TEST_LESS;
	case CompareFunction::LESS_EQUAL: return BGFX_STATE_DEPTH_TEST_LEQUAL;
	case CompareFunction::GREATER: return BGFX_STATE_DEPTH_TEST_GREATER;
	case CompareFunction::GREATER_EQUAL: return BGFX_STATE_DEPTH_TEST_GEQUAL;
	case CompareFunction::EQUAL: return BGFX_STATE_DEPTH_TEST_EQUAL;
	case CompareFunction::NOT_EQUAL: return BGFX_STATE_DEPTH_TEST_NOTEQUAL;
	case CompareFunction::ALWAYS: return BGFX_STATE_DEPTH_TEST_ALWAYS;
	default: ;
	}
	return 0;
}

uint32_t UtilsBX::toBXStencilTest(CompareFunction compareFunction)
{
	switch (compareFunction)
	{
	case CompareFunction::NEVER: return BGFX_STENCIL_TEST_NEVER;
	case CompareFunction::LESS: return BGFX_STENCIL_TEST_LESS;
	case CompareFunction::LESS_EQUAL: return BGFX_STENCIL_TEST_LEQUAL;
	case CompareFunction::GREATER: return BGFX_STENCIL_TEST_GREATER;
	case CompareFunction::GREATER_EQUAL: return BGFX_STENCIL_TEST_GEQUAL;
	case CompareFunction::EQUAL: return BGFX_STENCIL_TEST_EQUAL;
	case CompareFunction::NOT_EQUAL: return BGFX_STENCIL_TEST_NOTEQUAL;
	case CompareFunction::ALWAYS: return BGFX_STENCIL_TEST_ALWAYS;
	default:;
	}
	return 0;
}

uint32_t UtilsBX::toBXStencilOpFailS(StencilOperation stencilOperation)
{
	switch (stencilOperation)
	{
	case StencilOperation::KEEP: return BGFX_STENCIL_OP_FAIL_S_KEEP;
	case StencilOperation::ZERO: return BGFX_STENCIL_OP_FAIL_S_ZERO;
	case StencilOperation::REPLACE: return BGFX_STENCIL_OP_FAIL_S_REPLACE;
	case StencilOperation::INVERT: return BGFX_STENCIL_OP_FAIL_S_INVERT;
	case StencilOperation::INCREMENT_WRAP: return BGFX_STENCIL_OP_FAIL_S_INCR;
	case StencilOperation::DECREMENT_WRAP: return BGFX_STENCIL_OP_FAIL_S_DECR;
	default: ;
	}
	return 0;
}

uint32_t UtilsBX::toBXStencilOpFailZ(StencilOperation stencilOperation)
{
	switch (stencilOperation)
	{
	case StencilOperation::KEEP: return BGFX_STENCIL_OP_FAIL_Z_KEEP;
	case StencilOperation::ZERO: return BGFX_STENCIL_OP_FAIL_Z_ZERO;
	case StencilOperation::REPLACE: return BGFX_STENCIL_OP_FAIL_Z_REPLACE;
	case StencilOperation::INVERT: return BGFX_STENCIL_OP_FAIL_Z_INVERT;
	case StencilOperation::INCREMENT_WRAP: return BGFX_STENCIL_OP_FAIL_Z_INCR;
	case StencilOperation::DECREMENT_WRAP: return BGFX_STENCIL_OP_FAIL_Z_DECR;
	default:;
	}
	return 0;
}

uint32_t UtilsBX::toBXStencilOpPassZ(StencilOperation stencilOperation)
{
	switch (stencilOperation)
	{
	case StencilOperation::KEEP: return BGFX_STENCIL_OP_PASS_Z_KEEP;
	case StencilOperation::ZERO: return BGFX_STENCIL_OP_PASS_Z_ZERO;
	case StencilOperation::REPLACE: return BGFX_STENCIL_OP_PASS_Z_REPLACE;
	case StencilOperation::INVERT: return BGFX_STENCIL_OP_PASS_Z_INVERT;
	case StencilOperation::INCREMENT_WRAP: return BGFX_STENCIL_OP_PASS_Z_INCR;
	case StencilOperation::DECREMENT_WRAP: return BGFX_STENCIL_OP_PASS_Z_DECR;
	default:;
	}
	return 0;
}

uint32_t UtilsBX::toBXSamplerMag(SamplerFilter samplerFilter)
{
	switch (samplerFilter)
	{
	case SamplerFilter::NEAREST: return BGFX_SAMPLER_MAG_POINT;
	case SamplerFilter::LINEAR: return BGFX_SAMPLER_MAG_ANISOTROPIC;
	case SamplerFilter::DONT_CARE: return 0;
	default: ;
	}
	return BGFX_SAMPLER_MAG_ANISOTROPIC;
}

uint32_t UtilsBX::toBXSamplerMin(SamplerFilter samplerFilter, bool hasMipmaps, bool isPow2)
{
	if (hasMipmaps && !isPow2)
	{
		if (samplerFilter == SamplerFilter::LINEAR)
			return BGFX_SAMPLER_MIN_ANISOTROPIC;
		else
			return BGFX_SAMPLER_MIN_POINT;
	}
	// note: mip will be linear if not set
	switch (samplerFilter)
	{
	case SamplerFilter::NEAREST: return BGFX_SAMPLER_MIN_POINT;
	case SamplerFilter::NEAREST_MIPMAP_NEAREST: return BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MIP_POINT;
	case SamplerFilter::NEAREST_MIPMAP_LINEAR: return BGFX_SAMPLER_MIN_POINT;
	case SamplerFilter::LINEAR: return BGFX_SAMPLER_MIN_ANISOTROPIC;
	case SamplerFilter::LINEAR_MIPMAP_LINEAR: return BGFX_SAMPLER_MIN_ANISOTROPIC;
	case SamplerFilter::LINEAR_MIPMAP_NEAREST: return BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MIP_POINT;
	case SamplerFilter::DONT_CARE: return 0;
	default: ;
	}
	return BGFX_SAMPLER_MIN_POINT;
}

uint32_t UtilsBX::toBXSamplerU(SamplerAddressMode samplerAddressMode, bool isPow2)
{
	if (!isPow2)
		return BGFX_SAMPLER_U_CLAMP;
	// note: will be repeat if not set
	switch (samplerAddressMode)
	{
	case SamplerAddressMode::MIRROR_REPEAT: return BGFX_SAMPLER_U_MIRROR;
	case SamplerAddressMode::CLAMP_TO_EDGE: return BGFX_SAMPLER_U_CLAMP;
	// missing BORDER
	case (SamplerAddressMode)(4) : return BGFX_SAMPLER_U_BORDER;
	default:;
	}
	return 0;
}

uint32_t UtilsBX::toBXSamplerV(SamplerAddressMode samplerAddressMode, bool isPow2)
{
	if (!isPow2)
		return BGFX_SAMPLER_V_CLAMP;
	// note: will be repeat if not set
	switch (samplerAddressMode)
	{
	case SamplerAddressMode::MIRROR_REPEAT: return BGFX_SAMPLER_V_MIRROR;
	case SamplerAddressMode::CLAMP_TO_EDGE: return BGFX_SAMPLER_V_CLAMP;
	// missing BORDER
	case (SamplerAddressMode)(4) : return BGFX_SAMPLER_V_BORDER;
	default:;
	}
	return 0;
}

uint32_t UtilsBX::toBXSamplerUV(SamplerAddressMode samplerAddressMode, bool isPow2)
{
	return toBXSamplerU(samplerAddressMode, isPow2) | toBXSamplerV(samplerAddressMode, isPow2);
}

uint32_t UtilsBX::toBXSampler(const SamplerDescriptor& samplerDescriptor, bool hasMipmaps, bool isPow2)
{
	return 0 |
		toBXSamplerMag(samplerDescriptor.magFilter) |
		toBXSamplerMin(samplerDescriptor.minFilter, hasMipmaps, isPow2) |
		toBXSamplerU(samplerDescriptor.sAddressMode, isPow2) |
		toBXSamplerV(samplerDescriptor.tAddressMode, isPow2);
}

namespace
{
	bgfx::ViewId CURRENT_VIEW = 0;
}

void UtilsBX::setCurrentView(bgfx::ViewId id)
{
	assert(id < bgfx::getCaps()->limits.maxViews);
	CURRENT_VIEW = id;
}

bgfx::ViewId UtilsBX::getCurrentView()
{
	return CURRENT_VIEW;
}

ThreadPool& getThreadPool()
{
	static ThreadPool ins(1);
	return ins;
}

void addThreadTask(const std::function<void()>& task)
{
	getThreadPool().add_task(task);
}

void addThreadTaskSync(const std::function<void()>& task)
{
	auto fu = getThreadPool().add_task_future(task);
	fu.get();
}

CC_BACKEND_END
