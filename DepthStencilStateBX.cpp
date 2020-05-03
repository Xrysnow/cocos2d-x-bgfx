#include "DepthStencilStateBX.h"
#include "UtilsBX.h"
#include "base/CCDirector.h"

using namespace bgfx;

CC_BACKEND_BEGIN

void DepthStencilStateBX::reset()
{
	addThreadTask([]()
	{
		setStencil(BGFX_STENCIL_NONE);
	});
}

DepthStencilStateBX::DepthStencilStateBX(const DepthStencilDescriptor& descriptor)
	: DepthStencilState(descriptor)
{
}

void DepthStencilStateBX::apply(unsigned stencilReferenceValueFront, unsigned stencilReferenceValueBack)
{
	_state = 0;
	if (_depthStencilInfo.depthTestEnabled)
		_state |= UtilsBX::toBXStateDepthTest(_depthStencilInfo.depthCompareFunction);
	if (_depthStencilInfo.depthWriteEnabled)
		_state |= BGFX_STATE_WRITE_Z;
	if (_depthStencilInfo.stencilTestEnabled)
	{
		// note: no write mask in bgfx
        //glStencilMask(_depthStencilInfo.frontFaceStencil.writeMask);
		auto& desc = _depthStencilInfo.frontFaceStencil;
		uint32_t stencil = 0;
		stencil |= UtilsBX::toBXStencilTest(desc.stencilCompareFunction);
		stencil |= BGFX_STENCIL_FUNC_REF(stencilReferenceValueFront);
		stencil |= BGFX_STENCIL_FUNC_RMASK(desc.readMask);
		stencil |= UtilsBX::toBXStencilOpFailS(desc.stencilFailureOperation);
		stencil |= UtilsBX::toBXStencilOpFailZ(desc.depthFailureOperation);
		stencil |= UtilsBX::toBXStencilOpPassZ(desc.depthStencilPassOperation);
		if (_isBackFrontStencilEqual)
		{
			addThreadTask([=]()
			{
				setStencil(stencil);
			});
		}
		else
		{
			auto& descBack = _depthStencilInfo.backFaceStencil;
			uint32_t stencilBack = 0;
			stencilBack |= UtilsBX::toBXStencilTest(descBack.stencilCompareFunction);
			stencilBack |= BGFX_STENCIL_FUNC_REF(stencilReferenceValueBack);
			stencilBack |= BGFX_STENCIL_FUNC_RMASK(descBack.readMask);
			stencilBack |= UtilsBX::toBXStencilOpFailS(descBack.stencilFailureOperation);
			stencilBack |= UtilsBX::toBXStencilOpFailZ(descBack.depthFailureOperation);
			stencilBack |= UtilsBX::toBXStencilOpPassZ(descBack.depthStencilPassOperation);
			addThreadTask([=]()
			{
				setStencil(stencil, stencilBack);
			});
		}
	}
}

uint64_t DepthStencilStateBX::getStateMask()
{
	return BGFX_STATE_DEPTH_TEST_MASK | BGFX_STATE_WRITE_Z;
}

CC_BACKEND_END
