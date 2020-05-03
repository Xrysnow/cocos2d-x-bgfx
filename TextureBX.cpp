#include "TextureBX.h"
#include "UtilsBX.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventType.h"
#include "base/CCDirector.h"

using namespace bgfx;

#define ISPOW2(n) (((n) & (n-1)) == 0)

CC_BACKEND_BEGIN

namespace
{
	bool isMipmapEnabled(SamplerFilter samplerFilter)
	{
		switch (samplerFilter)
		{
		case SamplerFilter::NEAREST_MIPMAP_NEAREST:
		case SamplerFilter::NEAREST_MIPMAP_LINEAR:
		case SamplerFilter::LINEAR_MIPMAP_LINEAR:
		case SamplerFilter::LINEAR_MIPMAP_NEAREST:
			return true;
		default: ;
		}
		return false;
	}
	void readBGFXTexture(TextureHandle handle, TextureHandle dst,
		size_t tex_width , size_t tex_height, size_t bitsPerElement,
		size_t x, size_t y, size_t width, size_t height, bool flipImage,
		const std::function<void(const unsigned char*, size_t, size_t)>& callback)
	{
		if (!callback)
			return;
		if (width == 0 || height == 0 || x + width >= tex_width || y + height >= tex_height
			|| !isValid(handle) || !isValid(dst))
		{
			callback(nullptr, 0, 0);
			return;
		}
		auto image = new unsigned char[tex_width * tex_height * bitsPerElement / 8];
		addThreadTask([=]()
		{
			blit(0, dst, 0, 0, handle);
			readTexture(handle, image);
		});
		const auto nFrame = Director::getInstance()->getTotalFrames();
		EventListenerCustom* listener;
		const auto bytePerRowClipped = width * bitsPerElement / 8;
		const auto offsetX = x * bitsPerElement / 8;
		auto cb = [=, &image, &listener](EventCustom*)
		{
			const auto curFrame = Director::getInstance()->getTotalFrames();
			if (curFrame == nFrame + 2 && image)
			{
				auto clipped = new unsigned char[bytePerRowClipped * height];
				if (!flipImage)
				{
					for (size_t i = 0; i < height; ++i)
					{
						memcpy(&clipped[i * bytePerRowClipped],
							&image[(i + y) * bytePerRowClipped + offsetX],
							bytePerRowClipped);
					}
				}
				else
				{
					for (size_t i = 0; i < height; ++i)
					{
						memcpy(&clipped[(height - i - 1) * bytePerRowClipped],
							&image[(i + y) * bytePerRowClipped + offsetX],
							bytePerRowClipped);
					}
				}
				callback(clipped, width, height);
				CC_SAFE_DELETE_ARRAY(clipped);
				CC_SAFE_DELETE_ARRAY(image);
				Director::getInstance()->getEventDispatcher()->removeEventListener(listener);
			}
			else if ((curFrame<nFrame || curFrame>nFrame + 2) && image)
			{
				CC_SAFE_DELETE_ARRAY(image);
				Director::getInstance()->getEventDispatcher()->removeEventListener(listener);
			}
		};
		listener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(
			Director::EVENT_BEFORE_DRAW, cb);
	}
}

Texture2DBX::Texture2DBX(const TextureDescriptor& descriptor)
: Texture2DBackend(descriptor)
{
	_handle = BGFX_INVALID_HANDLE;
	_isPow2 = ISPOW2(_width) && ISPOW2(_height);
	_format = UtilsBX::toBXTextureFormat(descriptor.textureFormat, &_isCompressed);
	_hasMipmaps = isMipmapEnabled(descriptor.samplerDescriptor.minFilter);
	_sampler = UtilsBX::toBXSampler(descriptor.samplerDescriptor, _hasMipmaps, _isPow2);
	checkTexture();
	initWithZeros();

#if CC_ENABLE_CACHE_TEXTURE_DATA
	// Listen this event to restored texture id after coming to foreground on Android.
	_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED,
		[this](EventCustom*)
	{
		this->_dirty = true;
		this->checkTexture();
		this->initWithZeros();
	});
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
		_backToForegroundListener, -1);
#endif
}

Texture2DBX::~Texture2DBX()
{
	if (isValid(_handle))
	{
		destroy(_handle);
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
	Director::getInstance()->getEventDispatcher()->removeEventListener(
		_backToForegroundListener);
#endif
}

void Texture2DBX::updateData(uint8_t* data,
	std::size_t width, std::size_t height, std::size_t level)
{
	if (_isCompressed)
		return;
	checkLevel(level);
	updateTexture2D(_handle, 0, uint8_t(level),
		0, 0,
		uint16_t(width), uint16_t(height),
		copy(data, width * height * _bitsPerElement / 8));
}

void Texture2DBX::updateCompressedData(uint8_t* data,
	std::size_t width, std::size_t height, std::size_t dataLen, std::size_t level)
{
	if (!_isCompressed)
		return;
	checkLevel(level);
	updateTexture2D(_handle, 0, uint8_t(level),
		0, 0,
		uint16_t(width), uint16_t(height),
		copy(data, dataLen));
}

void Texture2DBX::updateSubData(
	std::size_t xoffset, std::size_t yoffset,
	std::size_t width, std::size_t height,
	std::size_t level, uint8_t* data)
{
	if (_isCompressed)
		return;
	checkLevel(level);
	updateTexture2D(_handle, 0, uint8_t(level),
		uint16_t(xoffset), uint16_t(yoffset),
		uint16_t(width), uint16_t(height),
		copy(data, width * height * _bitsPerElement / 8));
}

void Texture2DBX::updateCompressedSubData(std::size_t xoffset, std::size_t yoffset, std::size_t width,
	std::size_t height, std::size_t dataLen, std::size_t level, uint8_t* data)
{
	if (!_isCompressed)
		return;
	checkLevel(level);
	updateTexture2D(_handle, 0, uint8_t(level),
		uint16_t(xoffset), uint16_t(yoffset),
		uint16_t(width), uint16_t(height),
		copy(data, dataLen));
}

void Texture2DBX::updateSamplerDescriptor(const SamplerDescriptor& sampler)
{
	const auto hasMipmaps = isMipmapEnabled(sampler.minFilter);
	if (hasMipmaps != _hasMipmaps)
	{
		_dirty = true;
		_hasMipmaps = hasMipmaps;
		checkTexture();
	}
	const auto sampler_ = UtilsBX::toBXSampler(sampler, _hasMipmaps, _isPow2);
	if (sampler_ != _sampler)
	{
		//_dirty = true;
		_samplerChanged = true;
		_sampler = sampler_;
	}
}

void Texture2DBX::getBytes(std::size_t x, std::size_t y, std::size_t width, std::size_t height, bool flipImage,
	std::function<void(const unsigned char*, std::size_t, std::size_t)> callback)
{
	const auto dst = createTexture2D(uint16_t(_width), uint16_t(_height), false, 1, _format, BGFX_TEXTURE_READ_BACK);
	readBGFXTexture(_handle, dst, _width, _height, _bitsPerElement, x, y, width, height, flipImage, callback);
}

void Texture2DBX::generateMipmaps()
{
	if (TextureUsage::RENDER_TARGET == _textureUsage)
		return;
	if (!_hasMipmaps)
	{
		_dirty = true;
		_hasMipmaps = true;
		checkTexture();
	}
}

void Texture2DBX::updateTextureDescriptor(const TextureDescriptor& descriptor)
{
	const auto old_textureType = _textureType;
	const auto old_textureFormat = _textureFormat;
	const auto old_textureUsage = _textureUsage;
	const auto old_width = _width;
	const auto old_height = _height;

	TextureBackend::updateTextureDescriptor(descriptor);

	if (old_textureType != _textureType ||
		old_textureFormat != _textureFormat ||
		old_textureUsage != _textureUsage ||
		old_width != _width ||
		old_height != _height)
	{
		_dirty = true;
	}

	_format = UtilsBX::toBXTextureFormat(_textureFormat, &_isCompressed);
	auto& sampler = descriptor.samplerDescriptor;
	const auto hasMipmaps = isMipmapEnabled(sampler.minFilter);
	if (hasMipmaps != _hasMipmaps)
	{
		_dirty = true;
		_hasMipmaps = hasMipmaps;
	}
	const auto sampler_ = UtilsBX::toBXSampler(sampler, _hasMipmaps, _isPow2);
	if (sampler_ != _sampler)
	{
		//_dirty = true;
		_samplerChanged = true;
		_sampler = sampler_;
	}
	checkTexture();

	// Update data here because `updateData()` may not be invoked later.
	// For example, a texture used as depth buffer will not invoke updateData().
	initWithZeros();
}

void Texture2DBX::apply(int index)
{
	checkTexture();
}

void Texture2DBX::updateData(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t level,
	const Memory* data)
{
	checkLevel(level);
	updateTexture2D(_handle, 0, level,
		x, y,
		width, height,
		data);
}

void Texture2DBX::initWithZeros()
{
	const auto size = _width * _height * _bitsPerElement / 8;
	if (!isValid(_handle) || size == 0)
		return;
	CCASSERT(size == _info.storageSize, "size mismatch");
	auto mem = alloc(size);
	memset(mem->data, 0, mem->size);
	updateTexture2D(_handle, 0, 0,
		0, 0,
		uint16_t(_width), uint16_t(_height),
		mem);
}

void Texture2DBX::checkLevel(std::size_t level)
{
	if (!_hasMipmaps && level > 0)
	{
		_dirty = true;
		_hasMipmaps = true;
		checkTexture();
	}
}

void Texture2DBX::checkTexture()
{
	if (!_dirty && isValid(_handle))
		return;
	if (_width * _height == 0)
		return;
	if (isValid(_handle))
	{
		CCLOG("destory old texture");
		destroy(_handle);
	}
	//NOTE: BGFX_TEXTURE_READ_BACK is not for TextureUsage::READ
	auto flags = BGFX_TEXTURE_NONE;
	if (_textureUsage == TextureUsage::RENDER_TARGET)
		flags |= BGFX_TEXTURE_RT;
	flags |= _sampler;
	_hasMipmaps = false;
	calcTextureSize(_info, _width, _height, 1, false, _hasMipmaps, 1, _format);
	_handle = createTexture2D(_width, _height, _hasMipmaps, 1, _format, flags);
	_dirty = false;
	_samplerChanged = false;
}

TextureCubeBX::TextureCubeBX(const TextureDescriptor& descriptor)
: TextureCubemapBackend(descriptor)
{
	assert(_width == _height);
	_textureType = TextureType::TEXTURE_CUBE;
	_handle = BGFX_INVALID_HANDLE;
	_isPow2 = ISPOW2(_width) && ISPOW2(_height);
	_format = UtilsBX::toBXTextureFormat(descriptor.textureFormat, &_isCompressed);
	_hasMipmaps = isMipmapEnabled(descriptor.samplerDescriptor.minFilter);
	_sampler = UtilsBX::toBXSampler(descriptor.samplerDescriptor, _hasMipmaps, _isPow2);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	// Listen this event to restored texture id after coming to foreground on Android.
	_backToForegroundListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND,
		[this](EventCustom*)
	{
		this->_dirty = true;
		this->checkTexture();
	});
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
		_backToForegroundListener, -1);
#endif
}

TextureCubeBX::~TextureCubeBX()
{
	if (isValid(_handle))
	{
		destroy(_handle);
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
	Director::getInstance()->getEventDispatcher()->removeEventListener(
		_backToForegroundListener);
#endif
}

void TextureCubeBX::updateSamplerDescriptor(const SamplerDescriptor& sampler)
{
	const auto hasMipmaps = isMipmapEnabled(sampler.minFilter);
	if (hasMipmaps != _hasMipmaps)
	{
		_dirty = true;
		_hasMipmaps = hasMipmaps;
		checkTexture();
	}
	const auto sampler_ = UtilsBX::toBXSampler(sampler, _hasMipmaps, _isPow2);
	if (sampler_ != _sampler)
	{
		//_dirty = true;
		_samplerChanged = true;
		_sampler = sampler_;
	}
}

void TextureCubeBX::updateFaceData(TextureCubeFace side, void* data)
{
	checkTexture();
	updateTextureCube(_handle, 0, uint8_t(side), 0, 0, 0, _width, _height,
		copy(data, _width * _height * _bitsPerElement / 8));
}

void TextureCubeBX::getBytes(std::size_t x, std::size_t y, std::size_t width, std::size_t height, bool flipImage,
	std::function<void(const unsigned char*, std::size_t, std::size_t)> callback)
{
	//TODO: need check
	const auto dst = createTexture2D(uint16_t(_width), uint16_t(_height), false, 1, _format, BGFX_TEXTURE_READ_BACK);
	readBGFXTexture(_handle, dst, _width, _height, _bitsPerElement, x, y, width, height, flipImage, callback);
}

void TextureCubeBX::generateMipmaps()
{
	if (TextureUsage::RENDER_TARGET == _textureUsage)
		return;
	if (!_hasMipmaps)
	{
		_dirty = true;
		_hasMipmaps = true;
		checkTexture();
	}
}

void TextureCubeBX::updateTextureDescriptor(const TextureDescriptor& descriptor)
{
	// only change format
	const auto old_textureFormat = _textureFormat;
	_textureFormat = descriptor.textureFormat;
	if (old_textureFormat != _textureFormat)
		_dirty = true;
	_format = UtilsBX::toBXTextureFormat(_textureFormat, &_isCompressed);
	auto& sampler = descriptor.samplerDescriptor;
	const auto hasMipmaps = isMipmapEnabled(sampler.minFilter);
	if (hasMipmaps != _hasMipmaps)
	{
		_dirty = true;
		_hasMipmaps = hasMipmaps;
	}
	const auto sampler_ = UtilsBX::toBXSampler(sampler, _hasMipmaps, _isPow2);
	if (sampler_ != _sampler)
	{
		//_dirty = true;
		_samplerChanged = true;
		_sampler = sampler_;
	}
	checkTexture();
}

void TextureCubeBX::apply(int index)
{
	checkTexture();
}

void TextureCubeBX::updateData(TextureCubeFace side, const Memory* data)
{
	assert(data->size == _width * _height * _bitsPerElement / 8);
	checkTexture();
	updateTextureCube(_handle, 0, uint8_t(side), 0, 0, 0, _width, _height, data);
}

void TextureCubeBX::checkTexture()
{
	if (!_dirty && isValid(_handle))
		return;
	if (_width == 0)
		return;
	if (isValid(_handle))
		destroy(_handle);
	auto flags = BGFX_TEXTURE_NONE;
	if (_textureUsage == TextureUsage::RENDER_TARGET)
		flags |= BGFX_TEXTURE_RT;
	_handle = createTextureCube(_width, _hasMipmaps, 1, _format, flags | _sampler);
	_dirty = false;
	_samplerChanged = false;
}

CC_BACKEND_END
