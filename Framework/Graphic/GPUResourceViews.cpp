#include "stdafx.h"

#include "GPUResourceViews.h"
#include "GraphicHelpers.h"

using namespace sce;

Framework::TextureView::TextureView(const Description &desc)
{
	GpuAddress::SurfaceType _type = getSurfaceTypeFromTextureType(desc.mTexType, desc.mIsDynamic);
	Gnm::TileMode _tileMode;
	Gnm::GpuMode _mode = Gnm::getGpuMode();

	Result ret = GpuAddress::computeSurfaceTileMode(_mode, &_tileMode, _type, desc.mFormat, _fragments);
	SCE_GNM_ASSERT_MSG(ret == (Result)GpuAddress::kStatusSuccess, "Create TextureView failed.");

	Gnm::TextureSpec spec;
	spec.init();
	spec.m_width = desc.mWidth;
	spec.m_height = desc.mHeight;
	spec.m_depth = desc.mDepth;
	spec.m_pitch = desc.mPitch;
	spec.m_numFragments = desc.mFragments;
	spec.m_textureType = desc.mTexType;
	spec.m_numMipLevels = desc.mMipLevels;
	spec.m_numSlices = desc.mNumSlices;
	spec.m_format = desc.mFormat;
	spec.m_tileModeHint = _tileMode;
	spec.m_minGpuMode = _mode;

	ret = mObject.init(&spec);
	SCE_GNM_ASSERT_MSG(ret == SCE_OK, "Create TextureView failed.");
}

void Framework::TextureView::assignAddress(void *baseAddr)
{
	SCE_GNM_ASSERT(baseAddr != nullptr);
	mObject.setBaseAddress(baseAddr);
}

Framework::RenderTargetView::RenderTargetView(const BaseTargetView::Description &desc)
{
	SCE_GNM_ASSERT(desc.mColorFormat != Gnm::kDataFormatInvalid);

	mUseCMask = desc.mUseCMask;
	mUseFMask = desc.mUseFMask;

	GpuAddress::SurfaceType _surfaceType = desc.mIsDisplayable ? GpuAddress::kSurfaceTypeColorTargetDisplayable : GpuAddress::kSurfaceTypeColorTarget;
	Gnm::GpuMode _mode = Gnm::getGpuMode();

	Gnm::TileMode _tileMode;
	Result ret = GpuAddress::computeSurfaceTileMode(_mode, &_tileMode, _surfaceType, desc.mColorFormat, convertNumFragments(desc.mFragments));
	SCE_GNM_ASSERT_MSG(ret == (Result)GpuAddress::kStatusSuccess, "Create RenderTargetView failed.");
	
	if (desc.mIsDynamic)
		_tileMode = Gnm::kTileModeDisplay_LinearAligned;

	Gnm::RenderTargetSpec spec;
	spec.init();
	spec.m_width = desc.mWidth;
	spec.m_height = desc.mHeight;
	spec.m_pitch = desc.mPitch;
	spec.m_numSlices = desc.mNumSlices;
	spec.m_colorFormat = desc.mColorFormat;
	spec.m_minGpuMode = _mode;
	spec.m_colorTileModeHint = _tileMode;
	spec.m_numSamples = desc.mSamples;
	spec.m_numFragments = desc.mFragments;
	spec.m_flags.enableCmaskFastClear = mUseCMask ? 1 : 0;
	spec.m_flags.enableFmaskCompression = mUseFMask ? 1 : 0;

	ret = mObject.init(&spec);
	SCE_GNM_ASSERT_MSG(ret == SCE_OK, "Create RenderTargetView failed.");
}

void Framework::RenderTargetView::assignAddress(void *colorAddr, void *cMaskAddr /*= nullptr*/, void *fMaskAddr /*= nullptr*/)
{
	SCE_GNM_ASSERT((mUseCMask && cMaskAddr != nullptr) || (!mUseCMask && cMaskAddr == nullptr));
	SCE_GNM_ASSERT((mUseFMask && fMaskAddr != nullptr) || (!mUseFMask && fMaskAddr == nullptr));
	mObject.setAddresses(colorAddr, cMaskAddr, fMaskAddr);
}

Framework::DepthStencilView::DepthStencilView(const BaseTargetView::Description &desc)
{
	SCE_GNM_ASSERT(desc.mDepthFormat != Gnm::kZFormatInvalid);

	mUseStencil = (desc.mStencilFormat != Gnm::kStencilInvalid);
	mUseHTile = desc.mUseHTile;

	Gnm::DataFormat _depthFormat = Gnm::DataFormat::build(desc.mDepthFormat);
	Gnm::GpuMode _mode = Gnm::getGpuMode();

	Gnm::TileMode _tileMode;
	// TODO check kSurfaceTypeDepthOnlyTarget stencil ?
	Result ret = GpuAddress::computeSurfaceTileMode(_mode, &_tileMode, GpuAddress::kSurfaceTypeDepthOnlyTarget1, _depthFormat, convertNumFragments(desc.mFragments));
	SCE_GNM_ASSERT_MSG(ret == (Result)GpuAddress::kStatusSuccess, "Create DepthStencilView failed.");

	Gnm::DepthRenderTargetSpec spec;
	spec.init();
	spec.m_width = desc.mWidth;
	spec.m_height = desc.mHeight;
	spec.m_pitch = desc.mPitch;
	spec.m_numSlices = desc.mNumSlices;
	spec.m_zFormat = desc.mDepthFormat;
	spec.m_stencilFormat = desc.mStencilFormat;
	spec.m_tileModeHint = _tileMode;
	spec.m_minGpuMode = _mode;
	spec.m_numFragments = desc.mFragments;
	spec.m_flags.enableHtileAcceleration = mUseHTile ? 1 : 0;

	ret = mObject.init(&spec);
	SCE_GNM_ASSERT_MSG(ret == SCE_OK, "Create DepthStencilView failed.");
}

void Framework::DepthStencilView::assignAddress(void *depthAddr, void *stencilAddr /*= nullptr*/, void *hTileAddr /*= nullptr*/)
{
	SCE_GNM_ASSERT((mUseStencil && stencilAddr != nullptr) || (!mUseStencil && stencilAddr == nullptr));
	SCE_GNM_ASSERT((mUseHTile && hTileAddr != nullptr) || (!mUseHTile && hTileAddr == nullptr));

	mObject.setAddresses(depthAddr, stencilAddr);
	mObject.setHtileAddress(hTileAddr);

	if (mUseHTile)
	{
		mObject.setHtileStencilDisable(!mUseStencil);
		mObject.setHtileAccelerationEnable(true);
	}
}
