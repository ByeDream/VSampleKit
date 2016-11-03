#include "stdafx.h"

#include "RenderContext.h"
#include "GraphicDevice.h"
#include "GPUResourceViews.h"
#include "RenderSurface.h"
#include "RenderableTexture.h"
#include "Swapchain.h"
#include "OutputDevice.h"

Framework::RenderContext::RenderContext(GraphicDevice *device)
	: mDevice(device)
{

}

Framework::RenderContext::~RenderContext()
{

}

void Framework::RenderContext::roll()
{
	// stall the CPU until the GPU is finished with the context chunk we're entering into
}

void Framework::RenderContext::reset()
{


	//if (isMainContext)
	{
		//->initializeDefaultHardwareState();
		//->waitUntilSafeForRendering(mDevice->getOutput()->getHandle(), mDevice->getSwapChain()->getCurrentBufferIndex());
	}
}

void Framework::RenderContext::submitAndFlip(bool asynchronous)
{
	OutputDevice::DeviceHandle _handle = mDevice->getOutput()->getHandle();
	SceVideoOutFlipMode _flipMode = mDevice->getSwapChain()->getDescription().mFilpMode;
	U32 _displayBufferIndex = mDevice->getSwapChain()->getCurrentBufferIndex();
	// A user - provided argument with no internal meaning.The <c><i>flipArg< / i>< / c> associated with the most recently completed flip is
	// included in the <c>SceVideoOutFlipStatus< / c> object retrieved by <c>sceVideoOutGetFlipStatus() < / c > ; it could therefore
	// be used to uniquely identify each flip.
	U64 _flipArg = mDevice->getSwapChain()->getFrameCount();



	// TODO parse and report the validation error
	//SCE_GNM_ASSERT_MSG(ret == sce::Gnm::kSubmissionSuccess, "Command buffer validation error.");
}

void Framework::RenderContext::appendLabelAtEOPWithInterrupt(void *dstGpuAddr, U64 value)
{
	SCE_GNM_ASSERT(dstGpuAddr != nullptr);
	//->writeImmediateAtEndOfPipeWithInterrupt(Gnm::kEopFlushCbDbCaches, dstGpuAddr, value, Gnm::kCacheActionNone);
}

void Framework::RenderContext::appendLabelAtEOP(void *dstGpuAddr, U64 value)
{
	SCE_GNM_ASSERT(dstGpuAddr != nullptr);
	//->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, dstGpuAddr, value, Gnm::kCacheActionNone);
}

void Framework::RenderContext::setTextureSurface(U32 soltID, const RenderSurface *surface)
{
	TextureView *_view = surface->getTexture()->getShaderResourceView();
	SCE_GNM_ASSERT_MSG(_view != nullptr, "failed to get correct view from surface");
}

void Framework::RenderContext::setRenderTargetSurface(U32 soltID, const RenderSurface *surface)
{
	RenderTargetView *_view = typeCast<BaseTargetView, RenderTargetView>(surface->getTexture()->getTargetView());
	SCE_GNM_ASSERT_MSG(_view != nullptr, "failed to get correct view from surface");
}

void Framework::RenderContext::setDepthStencilTargetSurface(const RenderSurface *surface)
{
	DepthStencilView *_view = typeCast<BaseTargetView, DepthStencilView>(surface->getTexture()->getTargetView());
	SCE_GNM_ASSERT_MSG(_view != nullptr, "failed to get correct view from surface");
}

void Framework::RenderContext::setViewport(U32 x, U32 y, U32 width, U32 height, Float32 minz /*= 0.0f*/, Float32 maxz /*= 1.0f*/)
{

}
