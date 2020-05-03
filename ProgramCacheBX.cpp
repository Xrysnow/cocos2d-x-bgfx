#include "renderer/backend/ProgramCache.h"
#include "renderer/backend/Device.h"
#include "renderer/backend/ShaderModule.h"
#include "base/ccMacros.h"
#include "base/CCConfiguration.h"
#include "ProgramBX.h"

namespace std
{
    template <>
    struct hash<cocos2d::backend::ProgramType>
    {
        typedef cocos2d::backend::ProgramType argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& v) const noexcept
        {
            return hash<int>()(static_cast<int>(v));
        }
    };
}

CC_BACKEND_BEGIN

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

	ProgramBX* newProgram(const std::string& varying, const std::string& vert, const std::string& frag)
    {
		return new ProgramBX(vert, frag, varying);
    }
	ProgramBX* newProgram(const std::string& varying, const std::string& vert, const std::string& frag,
		const std::vector<std::string>& def)
    {
		return new ProgramBX(vert, frag, varying, def);
    }
	std::vector<std::string> getLightMacros()
    {
	    const auto conf = Configuration::getInstance();
	    const auto DirLight = "#define MAX_DIRECTIONAL_LIGHT_NUM " + std::to_string(conf->getMaxSupportDirLightInShader());
	    const auto PointLight = "#define MAX_DIRECTIONAL_LIGHT_NUM " + std::to_string(conf->getMaxSupportPointLightInShader());
	    const auto SpotLight = "#define MAX_DIRECTIONAL_LIGHT_NUM " + std::to_string(conf->getMaxSupportSpotLightInShader());
		return { DirLight, PointLight, SpotLight };
    }
	std::vector<std::string> getNormalMappingMacros()
    {
		auto light = getLightMacros();
		light.emplace_back("#define USE_NORMAL_MAPPING 1");
		return light;
    }
}

#define NEW_PROGRAM(_v, _f) newProgram(\
	std::string(_v##_vary, sizeof(_v##_vary)),\
	std::string(_v##_vert, sizeof(_v##_vert)),\
	std::string(_f##_frag, sizeof(_f##_frag)))
#define NEW_PROGRAM_DEF(_v, _f, _def) newProgram(\
	std::string(_v##_vary, sizeof(_v##_vary)),\
	std::string(_v##_vert, sizeof(_v##_vert)),\
	std::string(_f##_frag, sizeof(_f##_frag)),\
	_def)

std::unordered_map<backend::ProgramType, backend::Program*>  ProgramCache::_cachedPrograms;
ProgramCache* ProgramCache::_sharedProgramCache = nullptr;

ProgramCache* ProgramCache::getInstance()
{
    if(!_sharedProgramCache)
    {
        _sharedProgramCache = new (std::nothrow) ProgramCache();
        if(!_sharedProgramCache->init())
        {
            CC_SAFE_RELEASE(_sharedProgramCache);
        }
    }
    return _sharedProgramCache;
}

void ProgramCache::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(_sharedProgramCache);
}

ProgramCache::~ProgramCache()
{
    for(auto& program : _cachedPrograms)
    {
        CC_SAFE_RELEASE(program.second);
    }
    CCLOGINFO("deallocing ProgramCache: %p", this);
    ShaderCache::destroyInstance();
}

bool ProgramCache::init()
{
    addProgram(ProgramType::POSITION_TEXTURE_COLOR);
    addProgram(ProgramType::ETC1);
    addProgram(ProgramType::LABEL_DISTANCE_NORMAL);
    addProgram(ProgramType::LABEL_NORMAL);
    addProgram(ProgramType::LABLE_OUTLINE);
    addProgram(ProgramType::LABLE_DISTANCEFIELD_GLOW);
    addProgram(ProgramType::POSITION_COLOR_LENGTH_TEXTURE);
    addProgram(ProgramType::POSITION_COLOR_TEXTURE_AS_POINTSIZE);
    addProgram(ProgramType::POSITION_COLOR);
    addProgram(ProgramType::POSITION);
    addProgram(ProgramType::LAYER_RADIA_GRADIENT);
    addProgram(ProgramType::POSITION_TEXTURE);
    addProgram(ProgramType::POSITION_TEXTURE_COLOR_ALPHA_TEST);
    addProgram(ProgramType::POSITION_UCOLOR);
    addProgram(ProgramType::ETC1_GRAY);
    addProgram(ProgramType::GRAY_SCALE);
    addProgram(ProgramType::LINE_COLOR_3D);
    addProgram(ProgramType::CAMERA_CLEAR);
    addProgram(ProgramType::SKYBOX_3D);
    addProgram(ProgramType::SKINPOSITION_TEXTURE_3D);
    addProgram(ProgramType::SKINPOSITION_NORMAL_TEXTURE_3D);
    addProgram(ProgramType::POSITION_NORMAL_TEXTURE_3D);
    addProgram(ProgramType::POSITION_TEXTURE_3D);
    addProgram(ProgramType::POSITION_3D);
    addProgram(ProgramType::POSITION_NORMAL_3D);
    addProgram(ProgramType::POSITION_BUMPEDNORMAL_TEXTURE_3D);
    addProgram(ProgramType::SKINPOSITION_BUMPEDNORMAL_TEXTURE_3D);
    addProgram(ProgramType::TERRAIN_3D);
    addProgram(ProgramType::PARTICLE_TEXTURE_3D);
    addProgram(ProgramType::PARTICLE_COLOR_3D);
    return true;
}

void ProgramCache::addProgram(ProgramType type)
{
	ProgramBX* program = nullptr;
    switch (type) {
	case ProgramType::POSITION_TEXTURE_COLOR:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, POSITION_TEXTURE_COLOR);
		break;
	case ProgramType::ETC1:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, ETC1);
		break;
	case ProgramType::LABEL_DISTANCE_NORMAL:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, LABEL_DISTANCE_NORMAL);
		break;
	case ProgramType::LABEL_NORMAL:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, LABEL_NORMAL);
		break;
	case ProgramType::LABLE_OUTLINE:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, LABLE_OUTLINE);
		break;
	case ProgramType::LABLE_DISTANCEFIELD_GLOW:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, LABLE_DISTANCEFIELD_GLOW);
		break;
	case ProgramType::POSITION_COLOR_LENGTH_TEXTURE:
		program = NEW_PROGRAM(POSITION_COLOR_LENGTH_TEXTURE, POSITION_COLOR_LENGTH_TEXTURE);
		break;
	case ProgramType::POSITION_COLOR_TEXTURE_AS_POINTSIZE:
		program = NEW_PROGRAM(POSITION_COLOR_TEXTURE_AS_POINTSIZE, POSITION_COLOR);
		break;
	case ProgramType::POSITION_COLOR:
		program = NEW_PROGRAM(POSITION_COLOR, POSITION_COLOR);
		break;
	case ProgramType::POSITION:
		program = NEW_PROGRAM(POSITION, POSITION);
		break;
	case ProgramType::LAYER_RADIA_GRADIENT:
		program = NEW_PROGRAM(POSITION, LAYER_RADIA_GRADIENT);
		break;
	case ProgramType::POSITION_TEXTURE:
		program = NEW_PROGRAM(POSITION_TEXTURE, POSITION_TEXTURE);
		break;
	case ProgramType::POSITION_TEXTURE_COLOR_ALPHA_TEST:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, POSITION_TEXTURE_COLOR_ALPHA_TEST);
		break;
	case ProgramType::POSITION_UCOLOR:
		program = NEW_PROGRAM(POSITION_UCOLOR, POSITION_UCOLOR);
		break;
	case ProgramType::ETC1_GRAY:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, ETC1_GRAY);
		break;
	case ProgramType::GRAY_SCALE:
		program = NEW_PROGRAM(POSITION_TEXTURE_COLOR, GRAY_SCALE);
		break;
	case ProgramType::LINE_COLOR_3D:
		program = NEW_PROGRAM(LINE_COLOR_3D, LINE_COLOR_3D);
		break;
	case ProgramType::CAMERA_CLEAR:
		program = NEW_PROGRAM(CAMERA_CLEAR, CAMERA_CLEAR);
		break;
	case ProgramType::SKYBOX_3D:
		program = NEW_PROGRAM(SKYBOX_3D, SKYBOX_3D);
		break;
	case ProgramType::SKINPOSITION_TEXTURE_3D:
		program = NEW_PROGRAM(SKINPOSITION_TEXTURE_3D, SKINPOSITION_TEXTURE_3D);
		break;
	case ProgramType::SKINPOSITION_NORMAL_TEXTURE_3D:
		program = NEW_PROGRAM_DEF(SKINPOSITION_NORMAL_TEXTURE_3D, NORMAL_TEXTURE_3D, getLightMacros());
		break;
	case ProgramType::POSITION_NORMAL_TEXTURE_3D:
		program = NEW_PROGRAM_DEF(POSITION_NORMAL_TEXTURE_3D, NORMAL_TEXTURE_3D, getLightMacros());
		break;
	case ProgramType::POSITION_TEXTURE_3D:
		program = NEW_PROGRAM(POSITION_TEXTURE_3D, POSITION_TEXTURE_3D);
		break;
	case ProgramType::POSITION_3D:
		program = NEW_PROGRAM(POSITION_TEXTURE_3D, POSITION_3D);
		break;
	case ProgramType::POSITION_NORMAL_3D:
		program = NEW_PROGRAM_DEF(POSITION_NORMAL_TEXTURE_3D, POSITION_NORMAL_3D, getLightMacros());
		break;
	case ProgramType::POSITION_BUMPEDNORMAL_TEXTURE_3D:
		program = NEW_PROGRAM_DEF(POSITION_NORMAL_TEXTURE_3D, NORMAL_TEXTURE_3D, getNormalMappingMacros());
		break;
	case ProgramType::SKINPOSITION_BUMPEDNORMAL_TEXTURE_3D:
		program = NEW_PROGRAM_DEF(SKINPOSITION_NORMAL_TEXTURE_3D, NORMAL_TEXTURE_3D, getNormalMappingMacros());
		break;
	case ProgramType::TERRAIN_3D:
		program = NEW_PROGRAM(TERRAIN_3D, TERRAIN_3D);
		break;
	case ProgramType::PARTICLE_TEXTURE_3D:
		program = NEW_PROGRAM(PARTICLE_TEXTURE_3D, PARTICLE_TEXTURE_3D);
		break;
	case ProgramType::PARTICLE_COLOR_3D:
		program = NEW_PROGRAM(PARTICLE_COLOR_3D, PARTICLE_COLOR_3D);
		break;
    default:
        CCASSERT(false, "Not built-in program type.");
        return;
    }
	if(!bgfx::isValid(program->getHandle()))
	{
		cocos2d::log("failed to create built-in program %d", (int)type);
	}
    program->setProgramType(type);
    ProgramCache::_cachedPrograms.emplace(type, program);
}

backend::Program* ProgramCache::getBuiltinProgram(ProgramType type) const
{
    const auto& iter = ProgramCache::_cachedPrograms.find(type);
    if (ProgramCache::_cachedPrograms.end() != iter)
    {
        return iter->second;
    }
    return nullptr;
}

void ProgramCache::removeProgram(backend::Program* program)
{
    if (!program)
    {
        return;
    }
    
    for (auto it = _cachedPrograms.cbegin(); it != _cachedPrograms.cend();)
    {
        if (it->second == program)
        {
            it->second->release();
            it = _cachedPrograms.erase(it);
            break;
        }
        else
            ++it;
    }
}

void ProgramCache::removeUnusedProgram()
{
    for (auto iter = _cachedPrograms.cbegin(); iter != _cachedPrograms.cend();)
    {
        auto program = iter->second;
        if (program->getReferenceCount() == 1)
        {
//            CCLOG("cocos2d: TextureCache: removing unused program");
            program->release();
            iter = _cachedPrograms.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void ProgramCache::removeAllPrograms()
{
    for (auto& program : _cachedPrograms)
    {
        program.second->release();
    }
    _cachedPrograms.clear();
}

CC_BACKEND_END
