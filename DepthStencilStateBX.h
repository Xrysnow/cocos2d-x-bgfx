#pragma once
#include "renderer/backend/DepthStencilState.h"
#include "bgfx/bgfx.h"

CC_BACKEND_BEGIN

class DepthStencilStateBX : public DepthStencilState
{
public:
	/// Reset to default state.
	static void reset();

	/**
	 * @param descriptor Specifies the depth and stencil status.
	 */
	DepthStencilStateBX(const DepthStencilDescriptor& descriptor);

	/**
	 * Set depth and stencil status to pipeline.
	 * @param stencilReferenceValueFront Specifies front stencil reference value.
	 * @param stencilReferenceValueBack Specifies back stencil reference value.
	 */
	void apply(unsigned int stencilReferenceValueFront, unsigned int stencilReferenceValueBack);

	uint64_t getState() const { return _state; }
	static uint64_t getStateMask();
private:
	uint64_t _state = 0;
	friend class CommandBufferBX;
};

CC_BACKEND_END
