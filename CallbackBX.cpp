#include "CallbackBX.h"
#include "cocos2d.h"
#include "bx/debug.h"
#include "bimg/bimg.h"
#include <string>

CC_BACKEND_BEGIN

namespace
{
	bool TRACE_ENABLE = false;
}

CallbackBX* CallbackBX::getInstance()
{
	static CallbackBX ins;
	return &ins;
}

CallbackBX::~CallbackBX()
{
}

void CallbackBX::fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str)
{
	auto fmt = "BGFX Fatal %s (in %s, %d)";
	switch (_code)
	{
	case bgfx::Fatal::DebugCheck:
		bx::debugBreak();
		break;
	case bgfx::Fatal::InvalidShader:
		fmt = "BGFX Fatal InvalidShader: %s (in %s, %d)";
		break;
	case bgfx::Fatal::UnableToInitialize:
		fmt = "BGFX Fatal UnableToInitialize: %s (in %s, %d)";
		break;
	case bgfx::Fatal::UnableToCreateTexture:
		fmt = "BGFX Fatal UnableToCreateTexture: %s (in %s, %d)";
		break;
	case bgfx::Fatal::DeviceLost:
		fmt = "BGFX Fatal DeviceLost: %s (in %s, %d)";
		break;
	case bgfx::Fatal::Count: break;
	default: ;
	}
	const auto info = StringUtils::format(fmt, _str, _filePath, _line);
	//bx::debugPrintf("%s", info.c_str());
	cocos2d::log("%s", info.c_str());
	cocos2d::ccMessageBox(info.c_str(), "BGFX Fatal");
	if (_code != bgfx::Fatal::DebugCheck)
		std::abort();
}

void CallbackBX::traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList)
{
	if (!TRACE_ENABLE)
		return;
	char temp[2048];
	char* out = temp;
	va_list argListCopy;
	va_copy(argListCopy, _argList);
	int32_t len = bx::snprintf(out, sizeof(temp), "%s (%d): ", _filePath, _line);
	int32_t total = len + bx::vsnprintf(out + len, sizeof(temp) - len, _format, argListCopy);
	va_end(argListCopy);
	if ((int32_t)sizeof(temp) < total)
	{
		out = (char*)alloca(total + 1);
		bx::memCopy(out, temp, len);
		bx::vsnprintf(out + len, total - len, _format, _argList);
	}
	out[total] = '\0';
	//bx::debugOutput(out);
	cocos2d::log("%s", out);
}

void CallbackBX::profilerBegin(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line)
{
}

void CallbackBX::profilerBeginLiteral(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line)
{
}

void CallbackBX::profilerEnd()
{
}

uint32_t CallbackBX::cacheReadSize(uint64_t _id)
{
	return 0;
}

bool CallbackBX::cacheRead(uint64_t _id, void* _data, uint32_t _size)
{
	return false;
}

void CallbackBX::cacheWrite(uint64_t _id, const void* _data, uint32_t _size)
{
}

void CallbackBX::screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch,
	const void* _data, uint32_t _size, bool _yflip)
{
	if (captureFunctions.empty())
		return;
	const auto f = captureFunctions.front();
	captureFunctions.pop();
	if (!f)
		return;

	const auto bytePerPix = _size / _pitch / _height;
	const auto bufferSize = _height * _height * bytePerPix;
	const auto stride = _width * bytePerPix;
	std::shared_ptr<uint8_t> buffer(new uint8_t[bufferSize], [](uint8_t* p) { CC_SAFE_DELETE_ARRAY(p); });
	auto buf = buffer.get();
	if (!_yflip)
	{
		for (int i = 0; i < _height; ++i)
			std::memcpy(buf + i * stride, (uint8_t*)_data + i * _pitch, stride);
	}
	else
	{
		buf += (_height - 1)*stride;
		for (int i = 0; i < _height; ++i)
			std::memcpy(buf - i * stride, (uint8_t*)_data + i * _pitch, stride);
	}
	f(buffer.get(), _width, _height);
}

void CallbackBX::captureBegin(uint32_t _width, uint32_t _height, uint32_t _pitch, bgfx::TextureFormat::Enum _format,
	bool _yflip)
{
}

void CallbackBX::captureEnd()
{
}

void CallbackBX::captureFrame(const void* _data, uint32_t _size)
{
}

void CallbackBX::pushCaptureCallback(const CaptureFn& callback)
{
	captureFunctions.push(callback);
}

void CallbackBX::setTraceEnable(bool enable)
{
	TRACE_ENABLE = enable;
}

CC_BACKEND_END
