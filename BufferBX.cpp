#include "BufferBX.h"
#include "base/ccMacros.h"
#include "base/CCDirector.h"
#include "base/CCEventType.h"
#include "base/CCEventDispatcher.h"
#include "UtilsBX.h"

using namespace bgfx;

CC_BACKEND_BEGIN

BufferBX::BufferBX(std::size_t size, BufferType type, BufferUsage usage)
	: Buffer(size, type, usage)
{
	_data = new char[size];
	_bufferAllocated = size;
	_handle.indexBuffer = BGFX_INVALID_HANDLE;
	CCASSERT(size > 0, "invalid parameter 'size'");
	CCASSERT(_data, "failed to allocate buffer");

#if CC_ENABLE_CACHE_TEXTURE_DATA
	_backToForegroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom*) {
		this->reloadBuffer();
	});
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_backToForegroundListener, -1);
#endif
}

BufferBX::~BufferBX()
{
	if (_hasHandle)
	{
		if (_type == BufferType::VERTEX)
		{
			if (_usage == BufferUsage::STATIC)
				destroy(_handle.vertexBuffer);
			else
				destroy(_handle.dynamicVertexBuffer);
		}
		else
		{
			if (_usage == BufferUsage::STATIC)
				destroy(_handle.indexBuffer);
			else
				destroy(_handle.dynamicIndexBuffer);
		}
	}
	delete[] _data;

#if CC_ENABLE_CACHE_TEXTURE_DATA
	Director::getInstance()->getEventDispatcher()->removeEventListener(_backToForegroundListener);
#endif
}

void BufferBX::usingDefaultStoredData(bool needDefaultStoredData)
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
	_needDefaultStoredData = needDefaultStoredData;
#endif
}

void BufferBX::setVertexLayout(const VertexLayout& vertexLayout)
{
	if (_hasHandle)
		return;
	assert(_data && _bufferAllocated > 0);
	// should only be used for vertex buffer
	if (_type == BufferType::VERTEX)
	{
		CCLOG("allocate vertex buffer of size %d", _bufferAllocated);
		if (_usage == BufferUsage::STATIC)
		{
			_handle.vertexBuffer = createVertexBuffer(
				copy(_data, _bufferAllocated), UtilsBX::toBXVertexLayout(vertexLayout));
		}
		else
		{
			_handle.dynamicVertexBuffer = createDynamicVertexBuffer(
				copy(_data, _bufferAllocated), UtilsBX::toBXVertexLayout(vertexLayout));
		}
		_layout = vertexLayout;
		_hasHandle = true;
	}
}

void BufferBX::apply(uint32_t start, uint32_t num, uint8_t stream, VertexLayoutHandle layout)
{
	if (!_hasHandle)
		return;
	if (_type == BufferType::VERTEX)
	{
		if (_usage == BufferUsage::STATIC)
		{
			const auto hdl = _handle.vertexBuffer;
			addThreadTask([=]()
			{
				setVertexBuffer(stream, hdl, start, num, layout);
			});
		}
		else
		{
			const auto hdl = _handle.dynamicVertexBuffer;
			addThreadTask([=]()
			{
				setVertexBuffer(stream, hdl, start, num, layout);
			});
		}
	}
	else
	{
		if (_usage == BufferUsage::STATIC)
		{
			const auto hdl = _handle.indexBuffer;
			addThreadTask([=]() { setIndexBuffer(hdl, start, num); });
		}
		else
		{
			const auto hdl = _handle.dynamicIndexBuffer;
			addThreadTask([=]() { setIndexBuffer(hdl, start, num); });
		}
	}
}

void BufferBX::apply(uint8_t stream)
{
	if (!_hasHandle)
		return;
	if (_type == BufferType::VERTEX)
	{
		if (_usage == BufferUsage::STATIC)
		{
			const auto hdl = _handle.vertexBuffer;
			addThreadTask([=]() { setVertexBuffer(stream, hdl); });
		}
		else
		{
			const auto hdl = _handle.dynamicVertexBuffer;
			addThreadTask([=]() { setVertexBuffer(stream, hdl); });
		}
	}
	else
	{
		if (_usage == BufferUsage::STATIC)
		{
			const auto hdl = _handle.indexBuffer;
			addThreadTask([=]() { setIndexBuffer(hdl); });
		}
		else
		{
			const auto hdl = _handle.dynamicIndexBuffer;
			addThreadTask([=]() { setIndexBuffer(hdl); });
		}
	}
}

#if CC_ENABLE_CACHE_TEXTURE_DATA
void BufferBX::reloadBuffer()
{
	if (!_needDefaultStoredData)
		return;
	_bufferAlreadyFilled = true;
	updateData(_data, _bufferAllocated);
}
#endif

void BufferBX::updateData(void* data, std::size_t size)
{
	assert(size && size <= _size);
	assert(_data);
	if (size == 0 || !data)
		return;
	size = std::min(size, _bufferAllocated);
	if (data != _data)
		memcpy(_data, data, size);
	if (_hasHandle)
	{
		// should be dynamic if allocated
		if (_usage == BufferUsage::STATIC)
			return;
		if (_type == BufferType::VERTEX)
		{
			update(_handle.dynamicVertexBuffer, 0, copy(_data, size));
		}
		else
		{
			update(_handle.dynamicIndexBuffer, 0, copy(_data, size));
		}
	}
	else
	{
		// init index buffer
		if (_type == BufferType::INDEX)
		{
			if (_usage == BufferUsage::STATIC)
			{
				_handle.indexBuffer = createIndexBuffer(copy(_data, _size));
			}
			else
			{
				_handle.dynamicIndexBuffer = createDynamicIndexBuffer(copy(_data, _size));
			}
			_hasHandle = true;
		}
	}
}

void BufferBX::updateSubData(void* data, std::size_t offset, std::size_t size)
{
	CCASSERT(_bufferAllocated != 0, "updateData should be invoke before updateSubData");
	CCASSERT(offset + size <= _bufferAllocated, "buffer size overflow");
	if (offset >= _bufferAllocated || size == 0)
	{
		CCLOG("%s: invalid parameter, size: %d, offset: %d", __FUNCTION__, size, offset);
		return;
	}
	size = std::min(size, _bufferAllocated - offset);
	// should be dynamic
	if (_usage == BufferUsage::STATIC)
	{
		CCLOG("%s: attempt to update a static buffer", __FUNCTION__);
		if(_hasHandle)
			return;
	}
	memcpy(_data + offset, data, size);
	if (_hasHandle)
	{
		if (_type == BufferType::VERTEX)
		{
			update(_handle.dynamicVertexBuffer, offset / _layout.getStride(), copy(_data + offset, size));
		}
		else
		{
			update(_handle.dynamicIndexBuffer, offset / sizeof(uint16_t), copy(_data + offset, size));
		}
	}
}

CC_BACKEND_END
