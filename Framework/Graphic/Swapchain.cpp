#include "stdafx.h"

#include "Swapchain.h"
#include "GraphicDevice.h"
#include "Application.h"
#include "OutputDevice.h"
#include "Memory/Allocators.h"
#include "RenderSet.h"
#include "GPUResource/Texture.h"
#include "GPUResource/GPUResourceViews.h"
#include "ChunkBasedRenderContext/RenderContext.h"
#include "GraphicHelpers.h"
#include "GPUFence.h"
#include "GPUResource/GPUResourceManager.h"

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
	SCE_GNM_ASSERT_MSG(mFrameLabelPool == nullptr, "deinit me before destroy me");
}

void Framework::SwapChain::init(const Description& desc, Allocators *allocators)
{
	mDesc = desc;

	SCE_GNM_ASSERT(mDevice != nullptr);
	const OutputDevice *_output = mDevice->getOutput();
	const Rect &_wndRect = _output->getWindowRect();
	mTargetWidth = _wndRect.right - _wndRect.left;
	mTargetHeight = _wndRect.bottom - _wndRect.top;

	// alloc label pool
	allocators->allocate((void**)&mFrameLabelPool, SCE_KERNEL_WB_ONION, sizeof(U64) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(U64), Gnm::kResourceTypeLabel, &mFrameLabellHandle, "SwapChain frame labels");
	allocators->allocate((void**)&mFlippingLabelPool, SCE_KERNEL_WB_ONION, sizeof(uint64_t) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(uint64_t), Gnm::kResourceTypeLabel, &mFlippingLabelHandle, "SwapChain flipping labels");
	SCE_GNM_ASSERT(mFrameLabelPool != nullptr);
	SCE_GNM_ASSERT(mFlippingLabelPool != nullptr);

	mEopEventQueue = new EopEventQueue("SwapChian main thread queue");

	initSwappedBuffers();
}

void Framework::SwapChain::deinit(Allocators *allocators)
{
	deinitSwappedBuffers();

	SAFE_DELETE(mEopEventQueue);

	allocators->release((void *)mFlippingLabelPool, SCE_KERNEL_WB_ONION, &mFlippingLabelHandle);
	mFlippingLabelPool = nullptr;
	allocators->release((void *)mFrameLabelPool, SCE_KERNEL_WB_ONION, &mFrameLabellHandle);
	mFrameLabelPool = nullptr;
}

void Framework::SwapChain::flip()
{
	submitFrame();
	synchronizeFrameToGPU();
	prepareFrame();
}

void Framework::SwapChain::initSwappedBuffers()
{
	for (auto i = 0; i < mDesc.mNumSwappedBuffers; i++)
	{
		RenderSurface::Description _colorSurfaceDesc;
		_colorSurfaceDesc.mWidth = mTargetWidth;
		_colorSurfaceDesc.mHeight = mTargetHeight;
		_colorSurfaceDesc.mFormat = mDesc.mColorFormat;
		_colorSurfaceDesc.mAAType = mDesc.mColorAAType;
		_colorSurfaceDesc.mType = GpuAddress::kSurfaceTypeColorTargetDisplayable;
		_colorSurfaceDesc.mIsDynamicDisplayableColorTarget = mDesc.mIsDynamic;
		_colorSurfaceDesc.mName = ::sBufferNames[i].mColorBufferName;

		RenderSurface::Description _depthSurfaceDesc;
		_colorSurfaceDesc.mWidth = mTargetWidth;
		_colorSurfaceDesc.mHeight = mTargetHeight;
		_colorSurfaceDesc.mFormat = Gnm::DataFormat::build(mDesc.mDepthFormat);
		_colorSurfaceDesc.mAAType = mDesc.mDepthAAType;
		_colorSurfaceDesc.mType = GpuAddress::kSurfaceTypeDepthOnlyTarget;
		_colorSurfaceDesc.mEnableStencil = mDesc.mUseStencil;
		_colorSurfaceDesc.mName = ::sBufferNames[i].mDepthBufferName;

		mSwapChainRenderSets[i] = new RenderSet;
		mDevice->allocRenderSet(mSwapChainRenderSets[i], (mDesc.mUseDepth ? &_depthSurfaceDesc : nullptr), &_colorSurfaceDesc);
		mSwappedBuffers[i] = mSwapChainRenderSets[i]->getColorSurface(0)->getBaseAddress();

		mFrameLabelPool[i]			= MAX_VALUE_64;
		mFlippingLabelPool[i]		= MAX_VALUE_64;
		mExpectedLabel[i]			= MAX_VALUE_64;
	}

	mCurrentBufferIndex = 0;
	mNextBufferIndex = (mCurrentBufferIndex + 1) % mDesc.mNumSwappedBuffers;
	mCurrentBuffer = mSwapChainRenderSets[mCurrentBufferIndex];

	// register buffer chain to output device
	RenderTargetView *_view = typeCast<BaseTargetView, RenderTargetView>(mCurrentBuffer->getColorSurface(0)->getTexture()->getTargetView());
	mDevice->getOutput()->registerBufferChain(mSwappedBuffers, mDesc.mNumSwappedBuffers, _view);
}

void Framework::SwapChain::deinitSwappedBuffers()
{
	mDevice->getOutput()->unregisterBufferChain();

	memset(mSwappedBuffers, 0, sizeof(mSwappedBuffers));
	mCurrentBuffer = nullptr;

	for (auto i = 0; i < mDesc.mNumSwappedBuffers; i++)
	{
		mDevice->releaseRenderSet(mSwapChainRenderSets[i]);
		SAFE_DELETE(mSwapChainRenderSets[i]);
	}
}

void Framework::SwapChain::submitFrame()
{
	RenderContext *_immediateContext = mDevice->getImmediateContext();

	// For frame synchronization
	mExpectedLabel[mCurrentBufferIndex] = mFrameCounter;
	_immediateContext->appendLabelAtEOPWithInterrupt(const_cast<U64 *>(mFrameLabelPool + mCurrentBufferIndex), mExpectedLabel[mCurrentBufferIndex]); //TODO try no interrupt version

																																					 // TODO dump buffer before flip
	_immediateContext->submitAndFlip(mDesc.mAsynchronous); // TODO CPU flip

	// Signals the system that every graphics and asynchronous compute command buffer for this frame has been submitted.
	// If the system must hibernate, then it will logically occur at this time.Because of this, any command buffer submitted prior
	Gnm::submitDone();
}

void Framework::SwapChain::synchronizeFrameToGPU()
{
	U32 _targetSyncBufferIndex = mDesc.mAsynchronous ? mNextBufferIndex : mCurrentBufferIndex;
	stallUntilGPUIsNotUsingBuffer(mEopEventQueue, _targetSyncBufferIndex);
}

void Framework::SwapChain::prepareFrame()
{
	advance();

	mDevice->rollImmediateContext();
	mDevice->rollDeferreContext();

	GPUFenceManager::getInstance()->flip();
	GPUResourceManager::getInstance()->flip();
	// TODO
	// 	g_PhysMemAllocator->Flip();
}

void Framework::SwapChain::advance()
{
	mFrameCounter++;

	mCurrentBufferIndex = (mCurrentBufferIndex + 1) % mDesc.mNumSwappedBuffers;
	mNextBufferIndex = (mCurrentBufferIndex + 1) % mDesc.mNumSwappedBuffers;
	mCurrentBuffer = mSwapChainRenderSets[mCurrentBufferIndex];
}

void Framework::SwapChain::stallUntilGPUIsNotUsingBuffer(EopEventQueue *eopEventQueue, U32 bufferIndex)
{
	SCE_GNM_ASSERT_MSG(bufferIndex >= 0 && bufferIndex < mDesc.mNumSwappedBuffers, "bufferIndex must be between 0 and %d.", mDesc.mNumSwappedBuffers - 1);
	while (mFrameLabelPool[bufferIndex] != mExpectedLabel[bufferIndex])
		eopEventQueue->waitForEvent();

	// if (kGpuEop == m_config.m_whoQueuesFlips)
	{
		volatile U32 spin = 0;
		while (mFlippingLabelPool[bufferIndex] != mExpectedLabel[bufferIndex])
			++spin;
	}
}
