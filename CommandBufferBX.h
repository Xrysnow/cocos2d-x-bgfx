#pragma once
#include "renderer/backend/Macros.h"
#include "renderer/backend/CommandBuffer.h"
#include "base/CCEventListenerCustom.h"
#include "math/CCGeometry.h"
#include "bgfx/bgfx.h"

CC_BACKEND_BEGIN

class BufferBX;
class RenderPipelineBX;
class ProgramBX;
class DepthStencilStateBX;

class CommandBufferBX final : public CommandBuffer
{
public:
	CommandBufferBX();
	~CommandBufferBX();

	/// @name Setters & Getters
	/**
	 * @brief Indicate the begining of a frame
	 */
	void beginFrame() override;

	/**
	 * Begin a render pass, initial color, depth and stencil attachment.
	 * @param descriptor Specifies a group of render targets that hold the results of a render pass.
	 */
	void beginRenderPass(const RenderPassDescriptor& descriptor) override;

	/**
	 * Sets the current render pipeline state object.
	 * @param renderPipeline An object that contains the graphics functions and configuration state used in a render pass.
	 */
	void setRenderPipeline(RenderPipeline* renderPipeline) override;

	/**
	 * Fixed-function state
	 * @param x The x coordinate of the upper-left corner of the viewport.
	 * @param y The y coordinate of the upper-left corner of the viewport.
	 * @param w The width of the viewport, in pixels.
	 * @param h The height of the viewport, in pixels.
	 */
	void setViewport(int x, int y, unsigned int w, unsigned int h) override;

	/**
	 * Fixed-function state
	 * @param mode Controls if primitives are culled when front facing, back facing, or not culled at all.
	 */
	void setCullMode(CullMode mode) override;

	/**
	 * Fixed-function state
	 * @param winding The winding order of front-facing primitives.
	 */
	void setWinding(Winding winding) override;

	/**
	 * Set a global buffer for all vertex shaders at the given bind point index 0.
	 * @param buffer The vertex buffer to be setted in the buffer argument table.
	 */
	void setVertexBuffer(Buffer* buffer) override;

	/**
	 * Set unifroms and textures
	 * @param programState A programState object that hold the uniform and texture data.
	 */
	void setProgramState(ProgramState* programState) override;

	/**
	 * Set indexes when drawing primitives with index list
	 * @ buffer A buffer object that the device will read indexes from.
	 * @ see `drawElements(PrimitiveType primitiveType, IndexFormat indexType, unsigned int count, unsigned int offset)`
	 */
	void setIndexBuffer(Buffer* buffer) override;

	/**
	 * Draw primitives without an index list.
	 * @param primitiveType The type of primitives that elements are assembled into.
	 * @param start For each instance, the first index to draw
	 * @param count For each instance, the number of indexes to draw
	 * @see `drawElements(PrimitiveType primitiveType, IndexFormat indexType, unsigned int count, unsigned int offset)`
	 */
	void drawArrays(PrimitiveType primitiveType, std::size_t start, std::size_t count) override;

	/**
	 * Draw primitives with an index list.
	 * @param primitiveType The type of primitives that elements are assembled into.
	 * @param indexType The type if indexes, either 16 bit integer or 32 bit integer.
	 * @param count The number of indexes to read from the index buffer for each instance.
	 * @param offset Byte offset within indexBuffer to start reading indexes from.
	 * @see `setIndexBuffer(Buffer* buffer)`
	 * @see `drawArrays(PrimitiveType primitiveType, unsigned int start,  unsigned int count)`
	*/
	void drawElements(PrimitiveType primitiveType, IndexFormat indexType, std::size_t count, std::size_t offset) override;

	/**
	 * Do some resources release.
	 */
	void endRenderPass() override;

	/**
	 * Present a drawable and commit a command buffer so it can be executed as soon as possible.
	 */
	void endFrame() override;

	/**
	 * Fixed-function state
	 * @param lineWidth Specifies the width of rasterized lines.
	 */
	void setLineWidth(float lineWidth) override;

	/**
	 * Fixed-function state
	 * @param x, y Specifies the lower left corner of the scissor box
	 * @param width Specifies the width of the scissor box
	 * @param height Specifies the height of the scissor box
	 */
	void setScissorRect(bool isEnabled, float x, float y, float width, float height) override;

	/**
	 * Set depthStencil status
	 * @param depthStencilState Specifies the depth and stencil status
	 */
	void setDepthStencilState(DepthStencilState* depthStencilState) override;

	/**
	 * Get a screen snapshot
	 * @param callback A callback to deal with screen snapshot image.
	 */
	void captureScreen(std::function<void(const unsigned char*, int, int)> callback) override;

	void printCurrentFrame() { _print = true; }

private:
	struct Viewport
	{
		int x = 0;
		int y = 0;
		unsigned int w = 0;
		unsigned int h = 0;
	};

	void prepareDrawing();
	void bindVertexBuffer(ProgramBX* program) const;
	void setUniforms(ProgramBX* program) const;
	void cleanResources();
	void applyRenderPassDescriptor(const RenderPassDescriptor& descirptor);
	void updateScissor();

	bgfx::ViewId _currentView = 0;
	RenderPassDescriptor _lastRPD;
	// The frame buffer generated by engine. All frame buffer other than default frame buffer share it.
	bgfx::FrameBufferHandle _generatedFBO = BGFX_INVALID_HANDLE;
	bgfx::FrameBufferHandle _defaultFBO = BGFX_INVALID_HANDLE;
	bgfx::FrameBufferHandle _currentFBO = BGFX_INVALID_HANDLE;

	struct Attach
	{
		void* colorTexture = nullptr;
		void* dsTexture = nullptr;
		bgfx::TextureHandle color = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle ds = BGFX_INVALID_HANDLE;
	};
	std::vector<std::pair<Attach, bgfx::FrameBufferHandle>> _attachments;
	std::vector<std::pair<Attach, bgfx::FrameBufferHandle>> _attachmentsOld;

	BufferBX* _vertexBuffer = nullptr;
	ProgramState* _programState = nullptr;
	BufferBX* _indexBuffer = nullptr;
	RenderPipelineBX* _renderPipeline = nullptr;

	CullMode _cullMode = CullMode::NONE;
	Winding _winding = Winding::COUNTER_CLOCK_WISE;
	DepthStencilStateBX* _depthStencilStateGL = nullptr;
	uint64_t _state = 0;

	Viewport _viewPort;
	float _scissor[4];
	bool _scissorEnabled = false;
	Rect _scissorRect;

	Vec4 _vpTramsform;
	bgfx::UniformHandle _vpHandle;

	bool _print = false;

#if CC_ENABLE_CACHE_TEXTURE_DATA
	EventListenerCustom* _backToForegroundListener = nullptr;
#endif
};

CC_BACKEND_END
