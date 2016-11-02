#include "stdafx.h"

#include "Swapchain.h"
#include "GraphicDevice.h"
#include "Application.h"
#include "OutputDevice.h"
#include "Memory/Allocators.h"
#include "RenderSet.h"
#include "Texture.h"
#include "GPUResourceViews.h"

using namespace sce;

namespace {
	struct BufferName
	{
		const char *mColorBufferName;
		const char *mDepthBufferName;
	};

	BufferName sBufferNames[Framework::Swapchain::MAX_NUMBER_OF_SWAPPED_BUFFERS] =
	{
		{ "Swapchain color Buffer 0", "Swapchain depth Buffer 0" },
		{ "Swapchain color Buffer 1", "Swapchain depth Buffer 1" },
		{ "Swapchain color Buffer 2", "Swapchain depth Buffer 2" },
		{ "Swapchain color Buffer 3", "Swapchain depth Buffer 3" },
		{ "Swapchain color Buffer 4", "Swapchain depth Buffer 4" },
		{ "Swapchain color Buffer 5", "Swapchain depth Buffer 5" },
		{ "Swapchain color Buffer 6", "Swapchain depth Buffer 6" },
		{ "Swapchain color Buffer 7", "Swapchain depth Buffer 7" }
	};
}

Framework::Swapchain::Swapchain(GraphicDevice *device)
	: mDevice(device)
{
	memset(mSwappedBuffers, 0, sizeof(mSwappedBuffers));
	memset(mSwapchainRenderSets, 0, sizeof(mSwapchainRenderSets));
}

Framework::Swapchain::~Swapchain()
{

}

void Framework::Swapchain::init(Allocators *allocators)
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

	mNumSwappedBuffers = mDevice->getApplication()->getConfig()->mNumberOfSwappedBuffers;
	SCE_GNM_ASSERT(mNumSwappedBuffers > 0 && mNumSwappedBuffers <= MAX_NUMBER_OF_SWAPPED_BUFFERS);

	// alloc label pool
	allocators->allocate((void**)&mFrameLabelPool, SCE_KERNEL_WB_ONION, sizeof(U64) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(U64), Gnm::kResourceTypeLabel, &mLabelHandle, "Swapchain labels");
	allocators->allocate((void**)&mFlippingLabelPool, SCE_KERNEL_WB_ONION, sizeof(uint64_t) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(uint64_t), Gnm::kResourceTypeLabel, &mLabelForPrepareFlipHandle, "Swapchain labels for prepare flip");
	SCE_GNM_ASSERT(mFrameLabelPool != nullptr);
	SCE_GNM_ASSERT(mFlippingLabelPool != nullptr);

	initSwappedBuffers();
}

void Framework::Swapchain::deinit(Allocators *allocators)
{
}

void Framework::Swapchain::flip()
{
	// flip
// 	gnmContext->KickCommandBuffer(GnmContext::SUBMIT_AND_FLIP,
// 		m_VideoHandle,
// 		flipMode,
// 		m_CurrentPresentBuffer,
// 		m_FrameIndex);

// 	GnmContext* mainContext = GfxexGraphicDevice::GetInstance()->GetDevice()->GetGNMContext();
// 	mainContext->initializeDefaultHardwareState();
// 	mainContext->waitUntilSafeForRendering(m_VideoHandle, m_CurrentPresentBuffer);
}

void Framework::Swapchain::initSwappedBuffers()
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

		mSwapchainRenderSets[i] = new RenderSet;
		mDevice->createRenderSet(mSwapchainRenderSets[i], &_depthSurfaceDesc, &_colorSurfaceDesc);
		mSwappedBuffers[i] = mSwapchainRenderSets[i]->getColorSurface(0)->getBaseAddress();

		mFrameLabelPool[i]			= MAX_VALUE_64;
		mFlippingLabelPool[i]		= MAX_VALUE_64;
		mExpectedLabel[i]			= MAX_VALUE_64;
	}

	mBackBufferIndex = 0;
	mFrontBufferIndex = (mBackBufferIndex + 1) % mNumSwappedBuffers;
	mBackBuffer = mSwapchainRenderSets[mBackBufferIndex];

	// register buffer chain to output device
	RenderTargetView *_view = typeCast<BaseTargetView, RenderTargetView>(mBackBuffer->getColorSurface(0)->getTexture()->getTargetView());
	mDevice->getOutput()->registerBufferChain(mSwappedBuffers, mNumSwappedBuffers, _view);
}
