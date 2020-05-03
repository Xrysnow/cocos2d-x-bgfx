#pragma once
#include "renderer/backend/Device.h"

CC_BACKEND_BEGIN

class DeviceBX : public Device
{
public:
	DeviceBX();
	~DeviceBX();

	/**
	 * New a CommandBuffer object, not auto released.
	 * @return A CommandBuffer object.
	 */
	virtual CommandBuffer* newCommandBuffer() override;

	/**
	 * New a Buffer object, not auto released.
	 * @param size Specifies the size in bytes of the buffer object's new data store.
	 * @param type Specifies the target buffer object. The symbolic constant must be BufferType::VERTEX or BufferType::INDEX.
	 * @param usage Specifies the expected usage pattern of the data store. The symbolic constant must be BufferUsage::STATIC, BufferUsage::DYNAMIC.
	 * @return A Buffer object.
	 */
	virtual Buffer* newBuffer(std::size_t size, BufferType type, BufferUsage usage) override;

	/**
	 * New a TextureBackend object, not auto released.
	 * @param descriptor Specifies texture description.
	 * @return A TextureBackend object.
	 */
	virtual TextureBackend* newTexture(const TextureDescriptor& descriptor) override;

	/**
	 * Create an auto released DepthStencilState object.
	 * @param descriptor Specifies depth and stencil description.
	 * @return An auto release DepthStencilState object.
	 */
	virtual DepthStencilState* createDepthStencilState(const DepthStencilDescriptor& descriptor) override;

	/**
	 * New a RenderPipeline object, not auto released.
	 * @param descriptor Specifies render pipeline description.
	 * @return A RenderPipeline object.
	 */
	virtual RenderPipeline* newRenderPipeline() override;

	/**
	 * Design for metal.
	 */
	virtual void setFrameBufferOnly(bool frameBufferOnly) override {}

	/**
	 * New a Program, not auto released.
	 * @param vertexShader Specifes this is a vertex shader source.
	 * @param fragmentShader Specifes this is a fragment shader source.
	 * @return A Program instance.
	 */
	virtual Program* newProgram(const std::string& vertexShader, const std::string& fragmentShader) override;

protected:
	/**
	 * New a shaderModule, not auto released.
	 * @param stage Specifies whether is vertex shader or fragment shader.
	 * @param source Specifies shader source.
	 * @return A ShaderModule object.
	 */
	virtual ShaderModule* newShaderModule(ShaderStage stage, const std::string& source) override;
};

CC_BACKEND_END
