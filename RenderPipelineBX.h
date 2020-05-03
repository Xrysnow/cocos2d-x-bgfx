#pragma once
#include "renderer/backend/RenderPipeline.h"
#include "renderer/backend/RenderPipelineDescriptor.h"
#include "bgfx/bgfx.h"

CC_BACKEND_BEGIN

class ProgramBX;

class RenderPipelineBX : public RenderPipeline
{
public:
	RenderPipelineBX() = default;
	~RenderPipelineBX();

	void update(
		const PipelineDescriptor & pipelineDescirptor,
		const RenderPassDescriptor& renderpassDescriptor) override;
	/**
	 * Get program instance.
	 * @return Program instance.
	 */
	ProgramBX* getProgram() const { return _programGL; }
	uint64_t getState() const { return _state; }
	static uint64_t getStateMask();

private:
	void updateBlendState(const BlendDescriptor& descriptor);

	ProgramBX* _programGL = nullptr;
	uint64_t _state = 0;
};

CC_BACKEND_END
