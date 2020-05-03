#include "ShaderModuleBX.h"
#include "ccMacros.h"
#include "bgfx_shader.h"
#include "renderer/ccShaders.h"
#include <algorithm>

#define BGFX_SHADER_BIN_VERSION 6
#define BGFX_CHUNK_MAGIC_CSH BX_MAKEFOURCC('C', 'S', 'H', BGFX_SHADER_BIN_VERSION)
#define BGFX_CHUNK_MAGIC_FSH BX_MAKEFOURCC('F', 'S', 'H', BGFX_SHADER_BIN_VERSION)
#define BGFX_CHUNK_MAGIC_VSH BX_MAKEFOURCC('V', 'S', 'H', BGFX_SHADER_BIN_VERSION)

using namespace bgfx;

namespace
{
#include "shaders/CAMERA_CLEAR.frag"
#include "shaders/CAMERA_CLEAR.vary"
#include "shaders/CAMERA_CLEAR.vert"
#include "shaders/ETC1.frag"
#include "shaders/ETC1_GRAY.frag"
#include "shaders/GRAY_SCALE.frag"
#include "shaders/LABEL_DISTANCE_NORMAL.frag"
#include "shaders/LABEL_NORMAL.frag"
#include "shaders/LABLE_DISTANCEFIELD_GLOW.frag"
#include "shaders/LABLE_OUTLINE.frag"
#include "shaders/LAYER_RADIA_GRADIENT.frag"
#include "shaders/LINE_COLOR_3D.frag"
#include "shaders/LINE_COLOR_3D.vary"
#include "shaders/LINE_COLOR_3D.vert"
#include "shaders/NORMAL_TEXTURE_3D.frag"
#include "shaders/PARTICLE_COLOR_3D.frag"
#include "shaders/PARTICLE_COLOR_3D.vary"
#include "shaders/PARTICLE_COLOR_3D.vert"
#include "shaders/PARTICLE_TEXTURE_3D.frag"
#include "shaders/PARTICLE_TEXTURE_3D.vary"
#include "shaders/PARTICLE_TEXTURE_3D.vert"
#include "shaders/POSITION.frag"
#include "shaders/POSITION.vary"
#include "shaders/POSITION.vert"
#include "shaders/POSITION_3D.frag"
#include "shaders/POSITION_COLOR.frag"
#include "shaders/POSITION_COLOR.vary"
#include "shaders/POSITION_COLOR.vert"
#include "shaders/POSITION_COLOR_LENGTH_TEXTURE.frag"
#include "shaders/POSITION_COLOR_LENGTH_TEXTURE.vary"
#include "shaders/POSITION_COLOR_LENGTH_TEXTURE.vert"
#include "shaders/POSITION_COLOR_TEXTURE_AS_POINTSIZE.vary"
#include "shaders/POSITION_COLOR_TEXTURE_AS_POINTSIZE.vert"
#include "shaders/POSITION_NORMAL_3D.frag"
#include "shaders/POSITION_NORMAL_TEXTURE_3D.vary"
#include "shaders/POSITION_NORMAL_TEXTURE_3D.vert"
#include "shaders/POSITION_TEXTURE.frag"
#include "shaders/POSITION_TEXTURE.vary"
#include "shaders/POSITION_TEXTURE.vert"
#include "shaders/POSITION_TEXTURE_3D.frag"
#include "shaders/POSITION_TEXTURE_3D.vary"
#include "shaders/POSITION_TEXTURE_3D.vert"
#include "shaders/POSITION_TEXTURE_COLOR.frag"
#include "shaders/POSITION_TEXTURE_COLOR.vary"
#include "shaders/POSITION_TEXTURE_COLOR.vert"
#include "shaders/POSITION_TEXTURE_COLOR_ALPHA_TEST.frag"
#include "shaders/POSITION_UCOLOR.frag"
#include "shaders/POSITION_UCOLOR.vary"
#include "shaders/POSITION_UCOLOR.vert"
#include "shaders/SKINPOSITION_NORMAL_TEXTURE_3D.vary"
#include "shaders/SKINPOSITION_NORMAL_TEXTURE_3D.vert"
#include "shaders/SKINPOSITION_TEXTURE_3D.frag"
#include "shaders/SKINPOSITION_TEXTURE_3D.vary"
#include "shaders/SKINPOSITION_TEXTURE_3D.vert"
#include "shaders/SKYBOX_3D.frag"
#include "shaders/SKYBOX_3D.vary"
#include "shaders/SKYBOX_3D.vert"
#include "shaders/TERRAIN_3D.frag"
#include "shaders/TERRAIN_3D.vary"
#include "shaders/TERRAIN_3D.vert"
}

static const std::unordered_map<std::string, std::string> BgfxFragShaderReplace = {
	{ cocos2d::positionColor_frag, POSITION_COLOR_frag },
	{ cocos2d::positionTexture_frag, POSITION_TEXTURE_frag },
	{ cocos2d::positionTextureColor_frag, POSITION_TEXTURE_COLOR_frag },
	{ cocos2d::positionTextureColorAlphaTest_frag, POSITION_TEXTURE_COLOR_ALPHA_TEST_frag },
	{ cocos2d::label_normal_frag, LABEL_NORMAL_frag },
	{ cocos2d::label_distanceNormal_frag, LABEL_DISTANCE_NORMAL_frag },
	{ cocos2d::labelOutline_frag, LABLE_OUTLINE_frag },
	{ cocos2d::labelDistanceFieldGlow_frag, LABLE_DISTANCEFIELD_GLOW_frag },
	{ cocos2d::lineColor3D_frag, LINE_COLOR_3D_frag },
	{ cocos2d::positionColorLengthTexture_frag, POSITION_COLOR_LENGTH_TEXTURE_frag },
	{ cocos2d::layer_radialGradient_frag, LAYER_RADIA_GRADIENT_frag },
	{ cocos2d::grayScale_frag, GRAY_SCALE_frag },
	// missing positionTextureUColor_frag
	{ cocos2d::positionUColor_frag, POSITION_UCOLOR_frag },
	{ cocos2d::etc1_frag, ETC1_frag },
	{ cocos2d::etc1Gray_frag, ETC1_GRAY_frag },
	{ cocos2d::cameraClear_frag, CAMERA_CLEAR_frag },
	{ cocos2d::CC3D_color_frag, POSITION_3D_frag },
	{ cocos2d::CC3D_colorNormal_frag, POSITION_NORMAL_3D_frag },
	{ cocos2d::CC3D_colorNormalTexture_frag, NORMAL_TEXTURE_3D_frag },
	{ cocos2d::CC3D_colorTexture_frag, POSITION_TEXTURE_3D_frag },
	{ cocos2d::CC3D_particleTexture_frag, PARTICLE_TEXTURE_3D_frag },
	{ cocos2d::CC3D_particleColor_frag, PARTICLE_COLOR_3D_frag },
	{ cocos2d::CC3D_skybox_frag, SKYBOX_3D_frag },
	{ cocos2d::CC3D_terrain_frag, TERRAIN_3D_frag },
};
static const std::unordered_map<std::string, std::pair<std::string, std::string>> BgfxVertShaderReplace = {
	{ cocos2d::positionColor_vert, { POSITION_COLOR_vert, POSITION_COLOR_vary } },
	{ cocos2d::positionTexture_vert, { POSITION_TEXTURE_vert, POSITION_TEXTURE_vary } },
	{ cocos2d::positionTextureColor_vert, { POSITION_TEXTURE_COLOR_vert, POSITION_TEXTURE_COLOR_vary } },
	{ cocos2d::lineColor3D_vert, { LINE_COLOR_3D_vert, LINE_COLOR_3D_vary } },
	{ cocos2d::positionColorLengthTexture_vert, { POSITION_COLOR_LENGTH_TEXTURE_vert, POSITION_COLOR_LENGTH_TEXTURE_vary } },
	{ cocos2d::positionColorTextureAsPointsize_vert, { POSITION_COLOR_TEXTURE_AS_POINTSIZE_vert, POSITION_COLOR_TEXTURE_AS_POINTSIZE_vary } },
	{ cocos2d::position_vert, { POSITION_vert, POSITION_vary } },
	// missing positionNoMVP_vert
	// missing positionTextureUColor_vert
	{ cocos2d::positionUColor_vert, { POSITION_UCOLOR_vert, POSITION_UCOLOR_vary } },
	{ cocos2d::cameraClear_vert, { CAMERA_CLEAR_vert, CAMERA_CLEAR_vary } },
	{ cocos2d::CC3D_particle_vert, { PARTICLE_TEXTURE_3D_vert, PARTICLE_TEXTURE_3D_vary } },
	{ cocos2d::CC3D_positionNormalTexture_vert, { POSITION_NORMAL_TEXTURE_3D_vert, POSITION_NORMAL_TEXTURE_3D_vary } },
	{ cocos2d::CC3D_skinPositionNormalTexture_vert, { SKINPOSITION_NORMAL_TEXTURE_3D_vert, SKINPOSITION_NORMAL_TEXTURE_3D_vary } },
	{ cocos2d::CC3D_positionTexture_vert, { POSITION_TEXTURE_3D_vert, POSITION_TEXTURE_3D_vary } },
	{ cocos2d::CC3D_skinPositionTexture_vert, { SKINPOSITION_TEXTURE_3D_vert, SKINPOSITION_TEXTURE_3D_vary } },
	{ cocos2d::CC3D_skybox_vert, { SKYBOX_3D_vert, SKYBOX_3D_vary } },
	{ cocos2d::CC3D_terrain_vert, { TERRAIN_3D_vert, TERRAIN_3D_vary } },
};

static const std::string SHADER_MACROS = { BGFX_SHADER_MACROS, sizeof(BGFX_SHADER_MACROS) };
std::string cocos2d::backend::ShaderModuleBX::DEFAULT_VARYING = POSITION_TEXTURE_COLOR_vary;
static const std::string DefaultVert = POSITION_TEXTURE_COLOR_vert;
static const std::string DefaultFrag = POSITION_TEXTURE_COLOR_frag;
static const std::string DefaultVertHeader = "$input a_position, a_color0, a_texcoord0\n$output v_color0, v_texcoord0";
static const std::string DefaultFragHeader = "$input v_color0, v_texcoord0";

class StringWriter : public bx::FileWriter
{
public:
	StringWriter(){}
	virtual ~StringWriter(){}

	bool open(const bx::FilePath& _filePath, bool _append, bx::Error* _err) override
	{
		path = _filePath.getCPtr();
		return true;
	}

	void close() override {}

	int64_t seek(int64_t _offset = 0, bx::Whence::Enum _whence = bx::Whence::Current) override
	{
		switch (_whence)
		{
		case bx::Whence::Begin: cur = _offset; break;
		case bx::Whence::Current: cur += _offset; break;
		case bx::Whence::End: cur = buf.size() + _offset; break;
		default: ;
		}
		cur = std::max((int64_t)0, std::min(cur, (int64_t)buf.size()));
		return cur;
	}

	int32_t write(const void* _data, int32_t _size, bx::Error* _err) override
	{
		buf.reserve(cur + _size);
		buf.insert(cur, (const char*)_data, _size);
		cur += _size;
		return _size;
	}

	std::string path;
	std::string buf;
	int64_t cur = 0;
};

CC_BACKEND_BEGIN

ShaderModuleBX::ShaderModuleBX(ShaderStage stage, const std::string& source)
: ShaderModule(stage)
{
	_handle = BGFX_INVALID_HANDLE;
	compileShader(stage, source);
}

ShaderModuleBX::ShaderModuleBX(ShaderStage stage, const std::string& source, const std::string& varying,
	const std::vector<std::string>& defines, const std::vector<std::string>& includes)
: ShaderModule(stage)
{
	_handle = BGFX_INVALID_HANDLE;
	compileShader(stage, source, varying, defines, includes);
}

ShaderModuleBX::~ShaderModuleBX()
{
	deleteShader();
}

void ShaderModuleBX::setDefaultVarying()
{
	DEFAULT_VARYING = POSITION_TEXTURE_COLOR_vary;
}

void ShaderModuleBX::setDefaultVarying(const std::string& varying)
{
	DEFAULT_VARYING = varying;
}

void ShaderModuleBX::compileShader(ShaderStage stage, const std::string& source)
{
	compileShader(stage, source, DEFAULT_VARYING);
}

void ShaderModuleBX::compileShader(ShaderStage stage, const std::string& source, const std::string& varying,
	const std::vector<std::string>& defines, const std::vector<std::string>& includes)
{
	if (source.size() > 4)
	{
		uint32_t header = 0;
		std::memcpy(&header, source.c_str(), sizeof(uint32_t));
		if (header == BGFX_CHUNK_MAGIC_CSH
			|| header == BGFX_CHUNK_MAGIC_FSH
			|| header == BGFX_CHUNK_MAGIC_VSH)
		{
			_handle = createShader(copy(source.data(), source.size()));
			if (!bgfx::isValid(_handle))
			{
				CCLOG("cocos2d: ERROR: Failed to compile shader");
				//CCASSERT(false, "Shader compile failed!");
			}
			return;
		}
	}

	Options op;
	op.disasm = false;
	op.raw = false;
#if defined(COCOS2D_DEBUG) && COCOS2D_DEBUG > 0
	op.debugInformation = true;
#endif
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	op.platform = "windows";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	op.platform = "android";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
	op.platform = "ios";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
	op.platform = "osx";
#elif CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
	op.platform = "linux";
#endif

	static std::unordered_map<std::string, std::string> GL_VER = {
		{ "OpenGL 2.1", "120" },
		{ "OpenGL 3.1", "140" },
		{ "OpenGL 3.2", "150" },
		{ "OpenGL 3.3", "330" },
		{ "OpenGL 4.0", "400" },
		{ "OpenGL 4.1", "410" },
		{ "OpenGL 4.2", "420" },
		{ "OpenGL 4.3", "430" },
		{ "OpenGL 4.4", "440" },
		{ "OpenGL 4.5", "450" },
		{ "OpenGL 4.6", "460" },
	};
	switch (getRendererType())
	{
	case RendererType::Noop: break;
	case RendererType::Direct3D9:
		if(stage == ShaderStage::VERTEX)
			op.profile = "vs_3_0";
		else
			op.profile = "ps_3_0";
		break;
	case RendererType::Direct3D11:
	case RendererType::Direct3D12:
		if (stage == ShaderStage::VERTEX)
			op.profile = "vs_5_0";
		else
			op.profile = "ps_5_0";
		break;
	case RendererType::Gnm: break;
	case RendererType::Metal: op.profile = "metal"; break;
	case RendererType::Nvn: break;
	case RendererType::OpenGLES: break;
	case RendererType::OpenGL:
		{
			const auto it = GL_VER.find(getRendererName(RendererType::OpenGL));
			if (it != GL_VER.end())
				op.profile = it->second;
		}
		break;
	case RendererType::Vulkan: op.profile = "spirv"; break;
	case RendererType::Count: break;
	default:;
	}
	op.defines = defines;
	//TODO: includes

	auto src = source;
	auto vary = varying.empty() ? DEFAULT_VARYING.c_str() : varying.c_str();
	// replace internal shaders
	if (stage == ShaderStage::VERTEX)
	{
		const auto it = BgfxVertShaderReplace.find(src);
		if (it != BgfxVertShaderReplace.end())
		{
			src = it->second.first;
			vary = it->second.second.c_str();
		}
	}
	else
	{
		const auto it = BgfxFragShaderReplace.find(src);
		if (it != BgfxFragShaderReplace.end())
			src = it->second;
	}
	if (src.empty())
	{
		if (stage == ShaderStage::VERTEX)
			src = DefaultVert;
		else
			src = DefaultFrag;
	}
	if (src.size() > 3
		&& src[0] == '\xef'
		&&  src[1] == '\xbb'
		&&  src[2] == '\xbf')
	{
		src = src.substr(3);
	}
	FILE* f = nullptr;
	do
	{
		op.shaderType = '\0';
		switch (stage)
		{
		case ShaderStage::VERTEX: op.shaderType = 'v'; break;
		case ShaderStage::FRAGMENT: op.shaderType = 'f'; break;
		default: ;
		}
		if (op.shaderType == '\0')
			break;

		// insert after header
		const auto pos1 = src.find('\n');
		if (pos1 == std::string::npos)
			break;
		auto line1 = src.substr(0, pos1 + 1);
		const auto pos2 = src.find('\n', pos1 + 1);
		if (pos2 == std::string::npos)
			break;
		auto line2 = src.substr(pos1 + 1, pos2 - pos1);
		const auto rest = src.substr(pos2 + 1);
		if (line1[0] == '$')
		{
			if (line2[0] == '$')
				src = line1 + line2 + SHADER_MACROS + rest;
			else
				src = line1 + SHADER_MACROS + line2 + rest;
		}
		else
		{
			if (stage == ShaderStage::VERTEX)
				src = DefaultVertHeader + "\n" + SHADER_MACROS + line1 + line2 + rest;
			else
				src = DefaultFragHeader + "\n" + SHADER_MACROS + line1 + line2 + rest;
		}
		src.push_back('\n');
		StringWriter writer;
		const size_t padding = 16384;
		// data is deleted in bgfx::compileShader
		char* data = new char[src.size() + padding];
		std::memcpy(data, src.c_str(), src.size());
		std::memset(&data[src.size()], 0, padding);

		auto tmpname = std::tmpnam(nullptr);
		f = nullptr;
		if (tmpname)
			f = std::fopen(tmpname, "wb+");
		if (!tmpname || !f)
			break;
		std::fwrite(data, src.size() + padding, 1, f);
		op.inputFilePath = tmpname;

		if (!bgfx::compileShader(
			vary, "", data, (uint32_t)src.size(), op, &writer))
			break;
		_handle = createShader(copy(writer.buf.c_str(), writer.buf.size()));
		if (!bgfx::isValid(_handle))
			break;
		std::fclose(f);
		return;
	} while (false);
	if (f)
		std::fclose(f);
	cocos2d::log("cocos2d: ERROR: Failed to compile shader");
	CCASSERT(false, "Shader compile failed!");
}

char* ShaderModuleBX::getErrorLog(ShaderHandle shader) const
{
	return (char*)"";
}

void ShaderModuleBX::deleteShader()
{
	if (bgfx::isValid(_handle))
	{
		destroy(_handle);
	}
}

CC_BACKEND_END
