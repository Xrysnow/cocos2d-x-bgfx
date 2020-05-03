#pragma once
#include "renderer/backend/Buffer.h"
#include "renderer/backend/VertexLayout.h"
#include "bgfx/bgfx.h"

CC_BACKEND_BEGIN

class BufferBX : public Buffer
{
public:
	/**
	 * @param size Specifies the size in bytes of the buffer object's new data store.
	 * @param type Specifies the target buffer object. The symbolic constant must be BufferType::VERTEX or BufferType::INDEX.
	 * @param usage Specifies the expected usage pattern of the data store. The symbolic constant must be BufferUsage::STATIC, BufferUsage::DYNAMIC.
	 */
	BufferBX(std::size_t size, BufferType type, BufferUsage usage);
	~BufferBX();

	/**
	 * @brief Update buffer data
	 * @param data Specifies a pointer to data that will be copied into the data store for initialization.
	 * @param size Specifies the size in bytes of the data store region being replaced.
	 * @see `updateSubData(void* data, unsigned int offset, unsigned int size)`
	 */
	virtual void updateData(void* data, std::size_t size) override;

	/**
	 * @brief Update buffer sub-region data
	 * @param data Specifies a pointer to the new data that will be copied into the data store.
	 * @param offset Specifies the offset into the buffer object's data store where data replacement will begin, measured in bytes.
	 * @param size Specifies the size in bytes of the data store region being replaced.
	 * @see `updateData(void* data, unsigned int size)`
	 */
	virtual void updateSubData(void* data, std::size_t offset, std::size_t size) override;

	/**
	 * Static buffer data will automatically stored when it comes to foreground.
	 * This interface is used to indicate whether external data needs to be used to update the buffer(false) instead of using the default stored data(true).
	 * @param needDefaultStoredData Specifies whether to use the default stored data.
	 */
	virtual void usingDefaultStoredData(bool needDefaultStoredData) override;

	void setVertexLayout(const VertexLayout& vertexLayout);
	void apply(uint32_t start, uint32_t num, uint8_t stream, bgfx::VertexLayoutHandle layout = BGFX_INVALID_HANDLE);
	void apply(uint8_t stream);

private:
#if CC_ENABLE_CACHE_TEXTURE_DATA
	void reloadBuffer();
	bool _bufferAlreadyFilled = false;
	EventListenerCustom* _backToForegroundListener = nullptr;
#endif

	union BufferHandle
	{
		bgfx::IndexBufferHandle indexBuffer;
		bgfx::VertexBufferHandle vertexBuffer;
		bgfx::DynamicIndexBufferHandle dynamicIndexBuffer;
		bgfx::DynamicVertexBufferHandle dynamicVertexBuffer;
	};
	BufferHandle _handle;
	bool _hasHandle = false;
	std::size_t _bufferAllocated = 0;
	char* _data = nullptr;
	VertexLayout _layout;
	bool _needDefaultStoredData = true;
};

CC_BACKEND_END
