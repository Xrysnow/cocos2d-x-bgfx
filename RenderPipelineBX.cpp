#include "RenderPipelineBX.h"
#include "ShaderModuleBX.h"
#include "DepthStencilStateBX.h"
#include "ProgramBX.h"
#include "UtilsBX.h"

using namespace bgfx;

CC_BACKEND_BEGIN

RenderPipelineBX::~RenderPipelineBX()
{
	CC_SAFE_RELEASE(_programGL);
}

void RenderPipelineBX::update(
	const PipelineDescriptor& pipelineDescirptor,
	const RenderPassDescriptor& renderpassDescriptor)
{
	if (_programGL != pipelineDescirptor.programState->getProgram())
	{
		CC_SAFE_RELEASE(_programGL);
		_programGL = static_cast<ProgramBX*>(pipelineDescirptor.programState->getProgram());
		CC_SAFE_RETAIN(_programGL);
	}

	updateBlendState(pipelineDescirptor.blendDescriptor);
}

uint64_t RenderPipelineBX::getStateMask()
{
	return BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_EQUATION_MASK | BGFX_STATE_BLEND_MASK;
}

void RenderPipelineBX::updateBlendState(const BlendDescriptor& descriptor)
{
	_state = UtilsBX::toBXStateBlend(descriptor);
}

CC_BACKEND_END
