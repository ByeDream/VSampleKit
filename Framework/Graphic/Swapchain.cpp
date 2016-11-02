#include "stdafx.h"

#include "Swapchain.h"
#include "GraphicDevice.h"
#include "Application.h"
#include "OutputDevice.h"
#include "Memory/Allocators.h"
#include "RenderSet.h"
#include "Texture.h"
#include "GPUResourceViews.h"
#include "RenderContext.h"
#include "GraphicHelpers.h"

using namespace sce;

namespace {
	struct BufferName
	{
		const char *mColorBufferName;
		const char *mDepthBufferName;
	};

	BufferName sBufferNames[Framework::SwapChain::MAX_NUMBER_OF_SWAPPED_BUFFERS] =
	{
		{ "SwapChain color Buffer 0", "SwapChain depth Buffer 0" },
		{ "SwapChain color Buffer 1", "SwapChain depth Buffer 1" },
		{ "SwapChain color Buffer 2", "SwapChain depth Buffer 2" },
		{ "SwapChain color Buffer 3", "SwapChain depth Buffer 3" },
		{ "SwapChain color Buffer 4", "SwapChain depth Buffer 4" },
		{ "SwapChain color Buffer 5", "SwapChain depth Buffer 5" },
		{ "SwapChain color Buffer 6", "SwapChain depth Buffer 6" },
		{ "SwapChain color Buffer 7", "SwapChain depth Buffer 7" }
	};
}

Framework::SwapChain::SwapChain(GraphicDevice *device)
	: mDevice(device)
{
	memset(mSwappedBuffers, 0, sizeof(mSwappedBuffers));
	memset(mSwapChainRenderSets, 0, sizeof(mSwapChainRenderSets));
}

Framework::SwapChain::~SwapChain()
{

}

void Framework::SwapChain::init(Allocators *allocators)
{
	SCE_GNM_ASSERT(mDevice != nullptr);
	const OutputDevice *_output = mDevice->getOutput();
	const Rect &_wndRect = _output->getWindowRect();
	mTargetWidth = _wndRect.right - _wndRect.left;
	mTargetHeight = _wndRect.bottom - _wndRect.top;

	mUseDepth = true;
	mUseStencil = false;

	mColorAAType = AA_NONE; // TODO AA for swapchain
	mDepthAAType = AA_NONE;

	mUseDynamicBuffers = false;

	mColorFormat = Gnm::kDataFormatB8G8R8A8UnormSrgb;
	mDepthFormat = Gnm::kZFormat32Float;

	mFilpMode = SCE_VIDEO_OUT_FLIP_MODE_VSYNC;
	mAsynchronous = true;

	mNumSwappedBuffers = mDevice->getApplication()->getConfig()->mNumberOfSwappedBuffers;
	SCE_GNM_ASSERT(mNumSwappedBuffers > 0 && mNumSwappedBuffers <= MAX_NUMBER_OF_SWAPPED_BUFFERS);

	// alloc label pool
	allocators->allocate((void**)&mFrameLabelPool, SCE_KERNEL_WB_ONION, sizeof(U64) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(U64), Gnm::kResourceTypeLabel, &mLabelHandle, "SwapChain frame labels");
	allocators->allocate((void**)&mFlippingLabelPool, SCE_KERNEL_WB_ONION, sizeof(uint64_t) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(uint64_t), Gnm::kResourceTypeLabel, &mLabelForPrepareFlipHandle, "SwapChain flipping labels");
	SCE_GNM_ASSERT(mFrameLabelPool != nullptr);
	SCE_GNM_ASSERT(mFlippingLabelPool != nullptr);

	mEopEventQueue = new EopEventQueue("SwapChian main thread queue");

	initSwappedBuffers();
}

void Framework::SwapChain::deinit(Allocators *allocators)
{
}

void Framework::SwapChain::flip()
{
	RenderContext *_immediateContext = mDevice->getImmediateContext();

	// For frame synchronization
	mExpectedLabel[mCurrentBufferIndex] = mFrameCounter;
	_immediateContext->appendLabelAtEOPWithInterrupt(const_cast<U64 *>(mFrameLabelPool + mCurrentBufferIndex), mExpectedLabel[mCurrentBufferIndex]); //TODO try no interrupt version

	// TODO dump buffer before flip
	_immediateContext->submitAndFlip(); // TODO CPU flip

	// Signals the system that every graphics and asynchronous compute command buffer for this frame has been submitted.
	// If the system must hibernate, then it will logically occur at this time.Because of this, any command buffer submitted prior
	Gnm::submitDone();

	if (mAsynchronous)
	{
		// asynchronous, sync with next buffer
		advance();
	}
	else
	{

	}
	

	mDevice->rollImmediateContext();
	mDevice->rollDeferreContext();

// 	g_PhysMemAllocator->Flip();
// 	OrbisGPUFenceManager::GetInstance().Flip();
}

void Framework::SwapChain::initSwappedBuffers()
{
	for (auto i = 0; i < mNumSwappedBuffers; i++)
	{
		RenderSurface::Description _colorSurfaceDesc;
		_colorSurfaceDesc.mWidth = mTargetWidth;
		_colorSurfaceDesc.mHeight = mTargetHeight;
		_colorSurfaceDesc.mFormat = mColorFormat;
		_colorSurfaceDesc.mAAType = mColorAAType;
		_colorSurfaceDesc.mType = GpuAddress::kSurfaceTypeColorTargetDisplayable;
		_colorSurfaceDesc.mIsDynamicDisplayableColorTarget = mUseDynamicBuffers;
		_colorSurfaceDesc.mName = ::sBufferNames[i].mColorBufferName;

		RenderSurface::Description _depthSurfaceDesc;
		_colorSurfaceDesc.mWidth = mTargetWidth;
		_colorSurfaceDesc.mHeight = mTargetHeight;
		_colorSurfaceDesc.mFormat = Gnm::DataFormat::build(mDepthFormat);
		_colorSurfaceDesc.mAAType = mDepthAAType;
		_colorSurfaceDesc.mType = GpuAddress::kSurfaceTypeDepthOnlyTarget;
		_colorSurfaceDesc.mEnableStencil = mUseStencil;
		_colorSurfaceDesc.mName = ::sBufferNames[i].mDepthBufferName;

		mSwapChainRenderSets[i] = new RenderSet;
		mDevice->createRenderSet(mSwapChainRenderSets[i], &_depthSurfaceDesc, &_colorSurfaceDesc);
		mSwappedBuffers[i] = mSwapChainRenderSets[i]->getColorSurface(0)->getBaseAddress();

		mFrameLabelPool[i]			= MAX_VALUE_64;
		mFlippingLabelPool[i]		= MAX_VALUE_64;
		mExpectedLabel[i]			= MAX_VALUE_64;
	}

	mCurrentBufferIndex = 0;
	mNextBufferIndex = (mCurrentBufferIndex + 1) % mNumSwappedBuffers;
	mCurrentBuffer = mSwapChainRenderSets[mCurrentBufferIndex];

	// register buffer chain to output device
	RenderTargetView *_view = typeCast<BaseTargetView, RenderTargetView>(mCurrentBuffer->getColorSurface(0)->getTexture()->getTargetView());
	mDevice->getOutput()->registerBufferChain(mSwappedBuffers, mNumSwappedBuffers, _view);
}

void Framework::SwapChain::advance()
{
	mFrameCounter++;

	mCurrentBufferIndex = (mCurrentBufferIndex + 1) % mNumSwappedBuffers;
	mNextBufferIndex = (mCurrentBufferIndex + 1) % mNumSwappedBuffers;
	mCurrentBuffer = mSwapChainRenderSets[mCurrentBufferIndex];
}
