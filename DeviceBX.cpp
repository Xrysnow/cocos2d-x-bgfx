#include "DeviceBX.h"
#include "RenderPipelineBX.h"
#include "BufferBX.h"
#include "ShaderModuleBX.h"
#include "CommandBufferBX.h"
#include "TextureBX.h"
#include "DepthStencilStateBX.h"
#include "ProgramBX.h"
#include "DeviceInfoBX.h"

CC_BACKEND_BEGIN

Device* Device::getInstance()
{
	if (!_instance)
		_instance = new (std::nothrow) DeviceBX();
	return _instance;
}

DeviceBX::DeviceBX()
{
	_deviceInfo = new (std::nothrow) DeviceInfoBX();
	if (!_deviceInfo || _deviceInfo->init() == false)
	{
		delete _deviceInfo;
		_deviceInfo = nullptr;
	}
}

DeviceBX::~DeviceBX()
{
	ProgramCache::destroyInstance();
	delete _deviceInfo;
	_deviceInfo = nullptr;
}

CommandBuffer* DeviceBX::newCommandBuffer()
{
	return new (std::nothrow) CommandBufferBX();
}

Buffer* DeviceBX::newBuffer(std::size_t size, BufferType type, BufferUsage usage)
{
	return new (std::nothrow) BufferBX(size, type, usage);
}

TextureBackend* DeviceBX::newTexture(const TextureDescriptor& descriptor)
{
	switch (descriptor.textureType)
	{
	case TextureType::TEXTURE_2D:
		return new (std::nothrow) Texture2DBX(descriptor);
	case TextureType::TEXTURE_CUBE:
		return new (std::nothrow) TextureCubeBX(descriptor);
	default:
		return nullptr;
	}
}

ShaderModule* DeviceBX::newShaderModule(ShaderStage stage, const std::string& source)
{
	return new (std::nothrow) ShaderModuleBX(stage, source);
}

DepthStencilState* DeviceBX::createDepthStencilState(const DepthStencilDescriptor& descriptor)
{
	auto ret = new (std::nothrow) DepthStencilStateBX(descriptor);
	if (ret)
		ret->autorelease();
	return ret;
}

RenderPipeline* DeviceBX::newRenderPipeline()
{
	return new (std::nothrow) RenderPipelineBX();
}

Program* DeviceBX::newProgram(const std::string& vertexShader, const std::string& fragmentShader)
{
	return new (std::nothrow) ProgramBX(vertexShader, fragmentShader);
}

CC_BACKEND_END
