#include "stdafx.h"

#include "Swapchain.h"
#include "GraphicDevice.h"
#include "Application.h"
#include "OutputDevice.h"
#include "Memory/Allocators.h"
#include "RenderSet.h"

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

	mColorAAType = AA_NONE; // TODO AA for swapchain
	mDepthAAType = AA_NONE;

	mNumSwappedBuffers = mDevice->getApplication()->getConfig()->mNumberOfSwappedBuffers;
	SCE_GNM_ASSERT(mNumSwappedBuffers > 0 && mNumSwappedBuffers <= MAX_NUMBER_OF_SWAPPED_BUFFERS);

	// alloc label pool
	allocators->allocate((void**)&mLabelPool, SCE_KERNEL_WB_ONION, sizeof(U64) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(U64), Gnm::kResourceTypeLabel, &mLabelHandle, "Swapchain labels");
	allocators->allocate((void**)&mLabelForPreparePool, SCE_KERNEL_WB_ONION, sizeof(uint64_t) * MAX_NUMBER_OF_SWAPPED_BUFFERS, sizeof(uint64_t), Gnm::kResourceTypeLabel, &mLabelForPrepareFlipHandle, "Swapchain labels for prepare flip");
	SCE_GNM_ASSERT(mLabelPool != nullptr);
	SCE_GNM_ASSERT(mLabelForPreparePool != nullptr);

	initSwappedBuffers();
}

void Framework::Swapchain::deinit(Allocators *allocators)
{
}

void Framework::Swapchain::initSwappedBuffers()
{
	for (auto i = 0; i < mNumSwappedBuffers; i++)
	{
		RenderSurface::Description _colorSurfaceDesc;
		_colorSurfaceDesc.mWidth = mTargetWidth;
		_colorSurfaceDesc.mHeight = mTargetHeight;
		_colorSurfaceDesc.mFormat = Gnm::kDataFormatB8G8R8A8UnormSrgb;
		_colorSurfaceDesc.mAAType = mColorAAType;
		_colorSurfaceDesc.mType = GpuAddress::kSurfaceTypeColorTargetDisplayable;
		_colorSurfaceDesc.mName = ::sBufferNames[i].mColorBufferName;

		RenderSurface::Description _depthSurfaceDesc;
		_colorSurfaceDesc.mWidth = mTargetWidth;
		_colorSurfaceDesc.mHeight = mTargetHeight;
		_colorSurfaceDesc.mFormat = Gnm::DataFormat::build(Gnm::kZFormat32Float);;
		_colorSurfaceDesc.mAAType = mDepthAAType;
		_colorSurfaceDesc.mType = GpuAddress::kSurfaceTypeDepthOnlyTarget;
		_colorSurfaceDesc.mName = ::sBufferNames[i].mDepthBufferName;

		mSwappedBuffers[i] = new RenderSet;
		mDevice->createRenderSet(mSwappedBuffers[i], &_depthSurfaceDesc, &_colorSurfaceDesc);
	}
}
