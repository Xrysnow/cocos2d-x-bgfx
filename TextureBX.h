#pragma once
#include "renderer/backend/Texture.h"
#include "base/CCEventListenerCustom.h"
#include "bgfx/bgfx.h"

CC_BACKEND_BEGIN

class Texture2DBX : public Texture2DBackend
{
public:
	/**
	 * @param descriptor Specifies the texture description.
	 */
	Texture2DBX(const TextureDescriptor& descriptor);
	~Texture2DBX();

	/**
	 * Update a two-dimensional texture image
	 * @param data Specifies a pointer to the image data in memory.
	 * @param width Specifies the width of the texture image.
	 * @param height Specifies the height of the texture image.
	 * @param level Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
	 */
	void updateData(uint8_t* data, std::size_t width, std::size_t height, std::size_t level) override;

	/**
	 * Update a two-dimensional texture image in a compressed format
	 * @param data Specifies a pointer to the compressed image data in memory.
	 * @param width Specifies the width of the texture image.
	 * @param height Specifies the height of the texture image.
	 * @param dataLen Specifies the totoal size of compressed image in bytes.
	 * @param level Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
	 */
	void updateCompressedData(uint8_t* data, std::size_t width, std::size_t height, std::size_t dataLen, std::size_t level) override;

	/**
	 * Update a two-dimensional texture subimage
	 * @param xoffset Specifies a texel offset in the x direction within the texture array.
	 * @param yoffset Specifies a texel offset in the y direction within the texture array.
	 * @param width Specifies the width of the texture subimage.
	 * @param height Specifies the height of the texture subimage.
	 * @param level Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
	 * @param data Specifies a pointer to the image data in memory.
	 */
	void updateSubData(std::size_t xoffset, std::size_t yoffset, std::size_t width, std::size_t height, std::size_t level, uint8_t* data) override;

	/**
	 * Update a two-dimensional texture subimage in a compressed format
	 * @param xoffset Specifies a texel offset in the x direction within the texture array.
	 * @param yoffset Specifies a texel offset in the y direction within the texture array.
	 * @param width Specifies the width of the texture subimage.
	 * @param height Specifies the height of the texture subimage.
	 * @param dataLen Specifies the totoal size of compressed subimage in bytes.
	 * @param level Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
	 * @param data Specifies a pointer to the compressed image data in memory.
	 */
	void updateCompressedSubData(std::size_t xoffset, std::size_t yoffset, std::size_t width, std::size_t height, std::size_t dataLen, std::size_t level, uint8_t* data) override;

	/**
	 * Update sampler
	 * @param sampler Specifies the sampler descriptor.
	 */
	void updateSamplerDescriptor(const SamplerDescriptor &sampler)  override;

	/**
	 * Read a block of pixels from the drawable texture
	 * @param x,y Specify the window coordinates of the first pixel that is read from the drawable texture. This location is the lower left corner of a rectangular block of pixels.
	 * @param width,height Specify the dimensions of the pixel rectangle. width and height of one correspond to a single pixel.
	 * @param flipImage Specifies if needs to flip the image.
	 * @param callback Specifies a call back function to deal with the image.
	 */
	void getBytes(std::size_t x, std::size_t y, std::size_t width, std::size_t height, bool flipImage, std::function<void(const unsigned char*, std::size_t, std::size_t)> callback) override;

	/**
	 * Generate mipmaps.
	 */
	void generateMipmaps() override;

	/**
	 * Update texture description.
	 * @param descriptor Specifies texture and sampler descriptor.
	 */
	void updateTextureDescriptor(const TextureDescriptor& descriptor) override;

	/**
	 * Get texture object.
	 * @return Texture object.
	 */
	bgfx::TextureHandle getHandle() const { return _handle; }

	/**
	 * Set texture to pipeline
	 * @param index Specifies the texture image unit selector.
	 */
	void apply(int index);

	void updateData(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t level, const bgfx::Memory* data);

	uint32_t getSamplerFlag() const { return _sampler; }
	bool isSamplerChanged() const { return _samplerChanged; }
	void getSize(uint32_t& width, uint32_t& height) const { width = _width; height = _height; }

private:
	void initWithZeros();
	void checkLevel(std::size_t level);
	void checkTexture();
	
	bgfx::TextureHandle _handle;
	bgfx::TextureFormat::Enum _format = bgfx::TextureFormat::RGBA8;
	bgfx::TextureInfo _info;
	uint32_t _sampler = 0;
	bool _samplerChanged = false;
	bool _isPow2 = false;
	bool _dirty = true;
	EventListener* _backToForegroundListener = nullptr;
};

/**
 * Texture cube.
 */
class TextureCubeBX : public TextureCubemapBackend
{
public:
	/**
	 * @param descriptor Specifies the texture description.
	 */
	TextureCubeBX(const TextureDescriptor& descriptor);
	~TextureCubeBX();

	/**
	 * Update sampler
	 * @param sampler Specifies the sampler descriptor.
	 */
	void updateSamplerDescriptor(const SamplerDescriptor &sampler) override;

	/**
	 * Update texutre cube data in give slice side.
	 * @param side Specifies which slice texture of cube to be update.
	 * @param data Specifies a pointer to the image data in memory.
	 */
	void updateFaceData(TextureCubeFace side, void *data) override;

	/**
	 * Read a block of pixels from the drawable texture
	 * @param x,y Specify the window coordinates of the first pixel that is read from the drawable texture. This location is the lower left corner of a rectangular block of pixels.
	 * @param width,height Specify the dimensions of the pixel rectangle. width and height of one correspond to a single pixel.
	 * @param flipImage Specifies if needs to flip the image.
	 * @param callback
	 */
	void getBytes(std::size_t x, std::size_t y, std::size_t width, std::size_t height, bool flipImage, std::function<void(const unsigned char*, std::size_t, std::size_t)> callback) override;

	/// Generate mipmaps.
	void generateMipmaps() override;

	/**
	 * Update texture description.
	 * @param descriptor Specifies texture and sampler descriptor.
	 */
	void updateTextureDescriptor(const TextureDescriptor& descriptor) override;

	/**
	 * Get texture object.
	 * @return Texture object.
	 */
	bgfx::TextureHandle getHandle() const { return _handle; }

	/**
	 * Set texture to pipeline
	 * @param index Specifies the texture image unit selector.
	 */
	void apply(int index);

	void updateData(TextureCubeFace side, const bgfx::Memory* data);

	uint32_t getSamplerFlag() const { return _sampler; }
	bool isSamplerChanged() const { return _samplerChanged; }
	void getSize(uint32_t& width, uint32_t& height) const { width = _width; height = _height; }

private:
	void checkTexture();

	bgfx::TextureHandle _handle;
	bgfx::TextureFormat::Enum _format = bgfx::TextureFormat::RGBA8;
	uint32_t _sampler = 0;
	bool _samplerChanged = false;
	bool _isPow2 = false;
	bool _dirty = true;
	EventListener* _backToForegroundListener = nullptr;
};

CC_BACKEND_END
