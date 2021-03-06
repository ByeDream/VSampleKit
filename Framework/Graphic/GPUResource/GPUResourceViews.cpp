#include "stdafx.h"

#include "GPUResourceViews.h"
#include "GraphicHelpers.h"

using namespace sce;

Framework::BaseShaderView::BaseShaderView(const U8 *pData)
{
	SCE_GNM_ASSERT(pData != nullptr);
	Gnmx::parseShader(&mShaderInfo, pData);
	mHeaderPtr = mShaderInfo.m_shaderStruct;
	mBinaryPtr = mShaderInfo.m_gpuShaderCode;
}

sce::Gnm::SizeAlign Framework::VertexShaderView::getHeaderSizeAlign() const
{
	return Gnm::SizeAlign(mShaderInfo.m_vsShader->computeSize(), Gnm::kAlignmentOfShaderInBytes);
}


sce::Gnm::SizeAlign Framework::VertexShaderView::getFetchShaderSizeAlign() const
{
	return Gnm::SizeAlign(Gnmx::computeVsFetchShaderSize(mShaderInfo.m_vsShader), Gnm::kAlignmentOfShaderInBytes);
}

void Framework::VertexShaderView::assignAddress(void *headerAddr, void *binaryAddr)
{
	mHeaderPtr = headerAddr;
	mBinaryPtr = binaryAddr;
	mObject = (Gnmx::VsShader *)mHeaderPtr;
	mObject->patchShaderGpuAddress((void *)mBinaryPtr);

	Gnmx::generateInputOffsetsCache(&mInputCache, Gnm::kShaderStageVs, mObject);
}

void Framework::VertexShaderView::assignAddress(void *headerAddr, void *binaryAddr, void *fetchAddr)
{
	assignAddress(headerAddr, binaryAddr);
	mFetchAddr = fetchAddr;
	Gnmx::generateVsFetchShader(mFetchAddr, &mModifier, mObject, (Gnm::FetchShaderInstancingMode *)nullptr, 0); // Todo table
	mObject->applyFetchShaderModifier(mModifier);
}

sce::Gnm::SizeAlign Framework::PixelShaderView::getHeaderSizeAlign() const
{
	return sce::Gnm::SizeAlign(mShaderInfo.m_psShader->computeSize(), sce::Gnm::kAlignmentOfShaderInBytes);
}

void Framework::PixelShaderView::assignAddress(void *headerAddr, void *binaryAddr)
{
	mHeaderPtr = headerAddr;
	mBinaryPtr = binaryAddr;
	mObject = (Gnmx::PsShader *)mHeaderPtr;
	mObject->patchShaderGpuAddress((void *)mBinaryPtr);

	Gnmx::generateInputOffsetsCache(&mInputCache, Gnm::kShaderStagePs, mObject);
}

sce::Gnm::SizeAlign Framework::ComputeShaderView::getHeaderSizeAlign() const
{
	return sce::Gnm::SizeAlign(mShaderInfo.m_csShader->computeSize(), sce::Gnm::kAlignmentOfShaderInBytes);
}

void Framework::ComputeShaderView::assignAddress(void *headerAddr, void *binaryAddr)
{
	mHeaderPtr = headerAddr;
	mBinaryPtr = binaryAddr;
	mObject = (Gnmx::CsShader *)mHeaderPtr;
	mObject->patchShaderGpuAddress((void *)mBinaryPtr);

	Gnmx::generateInputOffsetsCache(&mInputCache, Gnm::kShaderStageCs, mObject);
}

Framework::TextureView::TextureView(const Description &desc)
{
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
	spec.m_tileModeHint = desc.mTileMode;
	spec.m_minGpuMode = Gnm::getGpuMode();

	Result ret = mObject.init(&spec);
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

	Gnm::RenderTargetSpec spec;
	spec.init();
	spec.m_width = desc.mWidth;
	spec.m_height = desc.mHeight;
	spec.m_pitch = desc.mPitch;
	spec.m_numSlices = desc.mNumSlices;
	spec.m_colorFormat = desc.mColorFormat;
	spec.m_minGpuMode = Gnm::getGpuMode();
	spec.m_colorTileModeHint = desc.mTileMode;
	spec.m_numSamples = desc.mSamples;
	spec.m_numFragments = desc.mFragments;
	spec.m_flags.enableCmaskFastClear = mUseCMask ? 1 : 0;
	spec.m_flags.enableFmaskCompression = mUseFMask ? 1 : 0;

	Result ret = mObject.init(&spec);
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

	mUseStencil = desc.mUseStencil;
	mUseHTile = desc.mUseHTile;

	Gnm::DepthRenderTargetSpec spec;
	spec.init();
	spec.m_width = desc.mWidth;
	spec.m_height = desc.mHeight;
	spec.m_pitch = desc.mPitch;
	spec.m_numSlices = desc.mNumSlices;
	spec.m_zFormat = desc.mDepthFormat;
	spec.m_stencilFormat = desc.mUseStencil ? Gnm::kStencil8 : Gnm::kStencilInvalid;
	spec.m_tileModeHint = desc.mTileMode;
	spec.m_minGpuMode = Gnm::getGpuMode();
	spec.m_numFragments = desc.mFragments;
	spec.m_flags.enableHtileAcceleration = mUseHTile ? 1 : 0;

	Result ret = mObject.init(&spec);
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



