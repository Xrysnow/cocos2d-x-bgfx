#include "DeviceInfoBX.h"
#include "bgfx/bgfx.h"
#include "UtilsBX.h"

using namespace bgfx;

CC_BACKEND_BEGIN

bool DeviceInfoBX::init()
{
	_maxAttributes = Attrib::Count;
	_maxTextureUnits = 8;
	_maxTextureSize = getCaps()->limits.maxTextureSize;
	_maxSamplesAllowed = getCaps()->limits.maxTextureSamplers;
	_supported = getCaps()->supported;
	return true;
}

const char* DeviceInfoBX::getVendor() const
{
	switch (getCaps()->vendorId)
	{
	case BGFX_PCI_ID_NONE: return "AUTO";
	case BGFX_PCI_ID_SOFTWARE_RASTERIZER: return "SOFTWARE";
	case BGFX_PCI_ID_AMD: return "AMD";
	case BGFX_PCI_ID_INTEL: return "INTEL";
	case BGFX_PCI_ID_NVIDIA: return "NVIDIA";
	default: ;
	}
	return "UNKNOWN";
}

const char* DeviceInfoBX::getRenderer() const
{
	return getRendererName(getRendererType());
}

const char* DeviceInfoBX::getVersion() const
{
	return "";
}

const char* DeviceInfoBX::getExtension() const
{
	return "";
}

static bool checkFromat(TextureFormat::Enum fmt)
{
	return getCaps()->formats[fmt] != BGFX_CAPS_FORMAT_TEXTURE_NONE;
}

bool DeviceInfoBX::checkForFeatureSupported(FeatureType feature)
{
	switch (feature)
	{
	case FeatureType::ETC1: return checkFromat(TextureFormat::ETC1);
	case FeatureType::S3TC: return checkFromat(TextureFormat::BC1);
	case FeatureType::AMD_COMPRESSED_ATC: return checkFromat(TextureFormat::ATC);
	case FeatureType::PVRTC: return checkFromat(TextureFormat::PTC12);
	case FeatureType::IMG_FORMAT_BGRA8888: return checkFromat(TextureFormat::BGRA8);
	//case FeatureType::DISCARD_FRAMEBUFFER: break;
	case FeatureType::PACKED_DEPTH_STENCIL: return checkFromat(TextureFormat::D24S8);
	//case FeatureType::VAO: break;
	//case FeatureType::MAPBUFFER: break;
	case FeatureType::DEPTH24: return checkFromat(TextureFormat::D24);
	//case FeatureType::ASTC: break;
	default: ;
	}
	return false;
}

bool DeviceInfoBX::checkPixelFormatFormat(PixelFormat pixelFormat)
{
	return getCaps()->formats[UtilsBX::toBXTextureFormat(pixelFormat)] != BGFX_CAPS_FORMAT_TEXTURE_NONE;
}

uint8_t DeviceInfoBX::getGPUCount() const
{
	return getCaps()->numGPUs;
}

CC_BACKEND_END
