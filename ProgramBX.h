#pragma once
#include "renderer/backend/Program.h"
#include "renderer/backend/RenderPipelineDescriptor.h"
#include "base/CCEventListenerCustom.h"
#include "bgfx/bgfx.h"

CC_BACKEND_BEGIN

class ShaderModuleBX;

class ProgramBX : public Program
{
public:
	/**
	 * @param vertexShader Specifes the vertex shader source.
	 * @param fragmentShader Specifes the fragment shader source.
	 */
	ProgramBX(const std::string& vertexShader, const std::string& fragmentShader);
	ProgramBX(
		const std::string& vertexShader,
		const std::string& fragmentShader,
		const std::string& varying,
		const std::vector<std::string>& defines = {},
		const std::vector<std::string>& includes = {});
	~ProgramBX();

	/**
	 * Get program object.
	 * @return Program object.
	 */
	bgfx::ProgramHandle getHandle() const { return _handle; }

	/**
	 * Get uniform location by name.
	 * @param uniform Specifies the uniform name.
	 * @return The uniform location.
	 */
	UniformLocation getUniformLocation(const std::string& uniform) const override;

	/**
	 * Get uniform location by engine built-in uniform enum name.
	 * @param name Specifies the engine built-in uniform enum name.
	 * @return The uniform location.
	 */
	UniformLocation getUniformLocation(Uniform name) const override;

	/**
	 * Get attribute location by attribute name.
	 * @param name Specifies the attribute name.
	 * @return The attribute location.
	 */
	int getAttributeLocation(const std::string& name) const override;

	/**
	 * Get attribute location by engine built-in attribute enum name.
	 * @param name Specifies the engine built-in attribute enum name.
	 * @return The attribute location.
	 */
	int getAttributeLocation(Attribute name) const override;

	/**
	 * Get maximum vertex location.
	 * @return Maximum vertex locaiton.
	 */
	int getMaxVertexLocation() const override;

	/**
	 * Get maximum fragment location.
	 * @return Maximum fragment location.
	 */
	int getMaxFragmentLocation() const override;

	/**
	 * Get active vertex attributes.
	 * @return Active vertex attributes. key is active attribute name, Value is corresponding attribute info.
	 */
	const std::unordered_map<std::string, AttributeBindInfo> getActiveAttributes() const override;

	/**
	 * Get uniform buffer size in bytes that can hold all the uniforms.
	 * @param stage Specifies the shader stage. The symbolic constant can be either VERTEX or FRAGMENT.
	 * @return The uniform buffer size in bytes.
	 */
	std::size_t getUniformBufferSize(ShaderStage stage) const override;

	/**
	 * Get a uniformInfo in given location from the specific shader stage.
	 * @param stage Specifies the shader stage. The symbolic constant can be either VERTEX or FRAGMENT.
	 * @param location Specifies the uniform locaion.
	 * @return The uniformInfo.
	 */
	const UniformInfo& getActiveUniformInfo(ShaderStage stage, int location) const override;

	/**
	 * Get all uniformInfos.
	 * @return The uniformInfos.
	 */
	const std::unordered_map<std::string, UniformInfo>& getAllActiveUniformInfo(ShaderStage stage) const override;

	void setUniformAsInt(const std::string& name);

	void applyUniformBuffer(const char* vertBuffer, const char* fragBuffer);
	void applyUniformTextures(
		const std::unordered_map<int, TextureInfo>& vertTextures,
		const std::unordered_map<int, TextureInfo>& fragTextures);

private:
	void compileProgram();
	//bool getAttributeLocation(const std::string& attributeName, unsigned int& location) const;
	void computeUniformInfos();
	void computeLocations();
	void setBuiltinUniform(const std::string& name, Uniform type);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	virtual void reloadProgram();
	virtual int getMappedLocation(int location) const override { return location; }
	virtual int getOriginalLocation(int location) const override { return location; }
	virtual const std::unordered_map<std::string, int> getAllUniformsLocation() const override { return _dummy; }
#endif

	bgfx::ProgramHandle _handle;
	ShaderModuleBX* _vertexShaderModule = nullptr;
	ShaderModuleBX* _fragmentShaderModule = nullptr;

	std::unordered_map<std::string, UniformInfo> _vertInfos;
	std::unordered_map<std::string, UniformInfo> _fragInfos;
	std::unordered_map<std::string, UniformInfo> _infos;
	std::unordered_map<std::string, UniformLocation> _locations;
	std::size_t _vertBufferSize = 0;
	std::size_t _fragBufferSize = 0;
	UniformLocation _builtinUniformLocation[UNIFORM_MAX];
#if CC_ENABLE_CACHE_TEXTURE_DATA
	std::unordered_map<std::string, int> _dummy;
	EventListenerCustom* _backToForegroundListener = nullptr;
#endif
};

CC_BACKEND_END
