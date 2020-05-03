#include "renderer/backend/ProgramState.h"
#include "renderer/backend/ProgramCache.h"
#include "renderer/backend/Program.h"
#include "renderer/backend/Texture.h"
#include "renderer/backend/Types.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventType.h"
#include "base/CCDirector.h"
//#include "bgfx/bgfx.h"

#include <algorithm>

//using namespace bgfx;

CC_BACKEND_BEGIN

//static field
std::vector<ProgramState::AutoBindingResolver*> ProgramState::_customAutoBindingResolvers;

TextureInfo::TextureInfo(const std::vector<uint32_t>& _slots, const std::vector<TextureBackend*> _textures)
: slot(_slots)
, textures(_textures)
{
    retainTextures();
}

TextureInfo::TextureInfo(const TextureInfo &other)
    : slot(other.slot)
    , textures(other.textures)
#if CC_ENABLE_CACHE_TEXTURE_DATA
    , location(other.location)
#endif
{
    retainTextures();
}

TextureInfo::~TextureInfo()
{
    releaseTextures();
}

void TextureInfo::retainTextures()
{
    for (auto& texture : textures)
        CC_SAFE_RETAIN(texture);
}

void TextureInfo::releaseTextures()
{
    for (auto& texture : textures)
        CC_SAFE_RELEASE(texture);
}

TextureInfo& TextureInfo::operator=(TextureInfo&& rhs)
{
    if (this != &rhs)
    {
        slot = rhs.slot;
        
        rhs.retainTextures();
        releaseTextures();
        textures = rhs.textures;
        
        //release the textures before cleaning the vertor
        rhs.releaseTextures();
        rhs.textures.clear();

#if CC_ENABLE_CACHE_TEXTURE_DATA
        location = rhs.location;
#endif
    }
    return *this;
}

TextureInfo& TextureInfo::operator=(const TextureInfo& rhs)
{
    if (this != &rhs)
    {
        slot = rhs.slot;
        textures = rhs.textures;
        retainTextures();

#if CC_ENABLE_CACHE_TEXTURE_DATA
        location = rhs.location;
#endif
    }
    return *this;
}

ProgramState::ProgramState(Program* program)
{
    init(program);
}

bool ProgramState::init(Program* program)
{
    CC_SAFE_RETAIN(program);
    _program = program;
    _vertexUniformBufferSize = _program->getUniformBufferSize(ShaderStage::VERTEX);
    _vertexUniformBuffer = new char[_vertexUniformBufferSize];
    memset(_vertexUniformBuffer, 0, _vertexUniformBufferSize);
    _fragmentUniformBufferSize = _program->getUniformBufferSize(ShaderStage::FRAGMENT);
    _fragmentUniformBuffer = new char[_fragmentUniformBufferSize];
    memset(_fragmentUniformBuffer, 0, _fragmentUniformBufferSize);

#if CC_ENABLE_CACHE_TEXTURE_DATA
    _backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED,
		[this](EventCustom*)
	{
        this->resetUniforms();
    });
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
		_backToForegroundListener, -1);
#endif
    return true;
}

void ProgramState::resetUniforms()
{
}

ProgramState::ProgramState()
{
}

ProgramState::~ProgramState()
{
    CC_SAFE_RELEASE(_program);
    CC_SAFE_DELETE_ARRAY(_vertexUniformBuffer);
    CC_SAFE_DELETE_ARRAY(_fragmentUniformBuffer);
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(
		_backToForegroundListener);
#endif
}

ProgramState *ProgramState::clone() const
{
    ProgramState *cp = new ProgramState();
    cp->_program = _program;
    cp->_vertexUniformBufferSize = _vertexUniformBufferSize;
    cp->_fragmentUniformBufferSize = _fragmentUniformBufferSize;
    cp->_vertexTextureInfos = _vertexTextureInfos;
    cp->_fragmentTextureInfos = _fragmentTextureInfos;
    cp->_vertexUniformBuffer = new char[_vertexUniformBufferSize];
    memcpy(cp->_vertexUniformBuffer, _vertexUniformBuffer, _vertexUniformBufferSize);
    cp->_vertexLayout = _vertexLayout;
    cp->_fragmentUniformBuffer = new char[_fragmentUniformBufferSize];
    memcpy(cp->_fragmentUniformBuffer, _fragmentUniformBuffer, _fragmentUniformBufferSize);
    CC_SAFE_RETAIN(cp->_program);

    return cp;
}

UniformLocation ProgramState::getUniformLocation(Uniform name) const
{
    return _program->getUniformLocation(name);
}

UniformLocation ProgramState::getUniformLocation(const std::string& uniform) const
{
    return _program->getUniformLocation(uniform);
}

void ProgramState::setCallbackUniform(
	const UniformLocation& uniformLocation,
	const UniformCallback& callback)
{
    _callbackUniforms[uniformLocation] = callback;
}

void ProgramState::setUniform(const UniformLocation& uniformLocation, const void* data, std::size_t size)
{
    switch (uniformLocation.shaderStage)
    {
        case ShaderStage::VERTEX:
            setVertexUniform(uniformLocation.location[0], data, size, 0);
            break;
        case ShaderStage::FRAGMENT:
            setFragmentUniform(uniformLocation.location[1], data, size);
            break;
        case ShaderStage::VERTEX_AND_FRAGMENT:
            setVertexUniform(uniformLocation.location[0], data, size, 0);
            setFragmentUniform(uniformLocation.location[1], data, size);
            break;
        default:
            break;
    }
}

void ProgramState::setVertexUniform(int location, const void* data, std::size_t size, std::size_t /*offset*/)
{
    if(location < 0)
        return;
    memcpy(_vertexUniformBuffer + location, data, size);
}

void ProgramState::setFragmentUniform(int location, const void* data, std::size_t size)
{
    if(location < 0)
        return;
	memcpy(_fragmentUniformBuffer + location, data, size);
}

void ProgramState::setTexture(const UniformLocation& uniformLocation, uint32_t slot, TextureBackend* texture)
{
    switch (uniformLocation.shaderStage)
    {
        case ShaderStage::VERTEX:
            setTexture(uniformLocation.location[0], slot, texture, _vertexTextureInfos);
            break;
        case ShaderStage::FRAGMENT:
            setTexture(uniformLocation.location[1], slot, texture, _fragmentTextureInfos);
            break;
        case ShaderStage::VERTEX_AND_FRAGMENT:
            setTexture(uniformLocation.location[0], slot, texture, _vertexTextureInfos);
            setTexture(uniformLocation.location[1], slot, texture, _fragmentTextureInfos);
            break;
        default:
            break;
    }
}

void ProgramState::setTextureArray(
	const UniformLocation& uniformLocation,
	const std::vector<uint32_t>& slots,
	const std::vector<TextureBackend*> textures)
{
    switch (uniformLocation.shaderStage)
    {
        case ShaderStage::VERTEX:
            setTextureArray(uniformLocation.location[0], slots, textures, _vertexTextureInfos);
            break;
        case ShaderStage::FRAGMENT:
            setTextureArray(uniformLocation.location[1], slots, textures, _fragmentTextureInfos);
            break;
        case ShaderStage::VERTEX_AND_FRAGMENT:
            setTextureArray(uniformLocation.location[0], slots, textures, _vertexTextureInfos);
            setTextureArray(uniformLocation.location[1], slots, textures, _fragmentTextureInfos);
            break;
        default:
            break;
    }
}

void ProgramState::setTexture(int location,
	uint32_t slot,
	TextureBackend* texture,
	std::unordered_map<int, TextureInfo>& textureInfo)
{
    if(location < 0)
        return;
    TextureInfo& info = textureInfo[location];
    info.releaseTextures();
    info.slot = {slot};
    info.textures = {texture};
    info.retainTextures();
#if CC_ENABLE_CACHE_TEXTURE_DATA
    info.location = location;
#endif
}

void ProgramState::setTextureArray(int location,
	const std::vector<uint32_t>& slots,
	const std::vector<TextureBackend*> textures,
	std::unordered_map<int, TextureInfo>& textureInfo)
{
    assert(slots.size() == textures.size());
    TextureInfo& info = textureInfo[location];
    info.releaseTextures();
    info.slot = slots;
    info.textures = textures;
    info.retainTextures();
#if CC_ENABLE_CACHE_TEXTURE_DATA
    info.location = location;
#endif
}

void ProgramState::setParameterAutoBinding(const std::string &uniform, const std::string &autoBinding)
{
    _autoBindings.emplace(uniform, autoBinding);
    applyAutoBinding(uniform, autoBinding);
}

void ProgramState::applyAutoBinding(const std::string &uniformName, const std::string &autoBinding)
{
    bool resolved = false;
    for (const auto resolver : _customAutoBindingResolvers)
    {
        resolved = resolver->resolveAutoBinding(this, uniformName, autoBinding);
        if (resolved) break;
    }
}

ProgramState::AutoBindingResolver::AutoBindingResolver()
{
    _customAutoBindingResolvers.emplace_back(this);
}

ProgramState::AutoBindingResolver::~AutoBindingResolver()
{
    auto &list = _customAutoBindingResolvers;
    list.erase(std::remove(list.begin(), list.end(), this), list.end());
}

void ProgramState::getVertexUniformBuffer(char** buffer, std::size_t& size) const
{
    *buffer = _vertexUniformBuffer;
    size = _vertexUniformBufferSize;
}

void ProgramState::getFragmentUniformBuffer(char** buffer, std::size_t& size) const
{
    *buffer = _fragmentUniformBuffer;
    size = _fragmentUniformBufferSize;
}

CC_BACKEND_END

