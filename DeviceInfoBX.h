#pragma once

#include "renderer/backend/DeviceInfo.h"
#include "renderer/backend/Types.h"

CC_BACKEND_BEGIN

class DeviceInfoBX : public DeviceInfo
{
public:
	DeviceInfoBX() = default;
	virtual ~DeviceInfoBX() = default;

	/**
	 * Gather features and implementation limits
	 */
	bool init() override;

	/**
	 * Get vendor device name.
	 * @return Vendor device name.
	 */
	const char* getVendor() const override;

	/**
	 * Get the full name of the vendor device.
	 * @return The full name of the vendor device.
	 */
	const char* getRenderer() const override;

	/**
	 * Get version name.
	 * @return Version name.
	 */
	const char* getVersion() const override;

	/**
	 * get OpenGL ES extensions.
	 * @return Extension supported by OpenGL ES.
	 */
	const char* getExtension() const override;

	/**
	 * Check if feature supported by OpenGL ES.
	 * @param feature Specify feature to be query.
	 * @return true if the feature is supported, false otherwise.
	 */
	bool checkForFeatureSupported(FeatureType feature) override;

	bool checkPixelFormatFormat(PixelFormat pixelFormat);
	uint8_t getGPUCount() const;
private:
	uint64_t _supported = 0;
};

CC_BACKEND_END
