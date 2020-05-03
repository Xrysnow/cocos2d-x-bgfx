#pragma once
#include "renderer/backend/ShaderModule.h"
#include "bgfx/bgfx.h"
#include "bx/file.h"
#include <vector>

namespace bgfx
{
	struct Options
	{
		Options();

		void dump();

		char shaderType;
		std::string platform;
		std::string profile;

		std::string	inputFilePath;
		std::string	outputFilePath;

		std::vector<std::string> includeDirs;
		std::vector<std::string> defines;
		std::vector<std::string> dependencies;

		bool disasm;
		bool raw;
		bool preprocessOnly;
		bool depends;

		bool debugInformation;

		bool avoidFlowControl;
		bool noPreshader;
		bool partialPrecision;
		bool preferFlowControl;
		bool backwardsCompatibility;
		bool warningsAreErrors;
		bool keepIntermediate;

		bool optimize;
		uint32_t optimizationLevel;
	};
	bool compileShader(const char* _varying, const char* _comment, char* _shader, uint32_t _shaderLen, Options& _options, bx::FileWriter* _writer);
}

CC_BACKEND_BEGIN

class ShaderModuleBX : public ShaderModule
{
public:
	/**
	 * @param stage Specifies whether is vertex shader or fragment shader.
	 * @param source Specifies shader source.
	 */
	ShaderModuleBX(ShaderStage stage, const std::string& source);
	ShaderModuleBX(ShaderStage stage, const std::string& source,
		const std::string& varying,
		const std::vector<std::string>& defines = {},
		const std::vector<std::string>& includes = {});
	~ShaderModuleBX();

	/**
	 * Get shader object.
	 * @return Shader object.
	 */
	bgfx::ShaderHandle getHandle() const { return _handle; }
	static void setDefaultVarying();
	static void setDefaultVarying(const std::string& varying);

private:
	void compileShader(ShaderStage stage, const std::string& source);
	void compileShader(ShaderStage stage, const std::string& source,
		const std::string& varying,
		const std::vector<std::string>& defines = {},
		const std::vector<std::string>& includes = {});
	char* getErrorLog(bgfx::ShaderHandle shader) const;
	void deleteShader();

	bgfx::ShaderHandle _handle;
	static std::string DEFAULT_VARYING;
	friend class ProgramBX;
};

CC_BACKEND_END
