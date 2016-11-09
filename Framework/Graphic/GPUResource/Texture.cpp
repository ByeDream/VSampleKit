#include "stdafx.h"

#include "Texture.h"

#include "Memory/Allocators.h"
#include "GPUResourceViews.h"

using namespace sce;

Framework::Texture::Texture()
{

}

Framework::Texture::~Texture()
{
	SCE_GNM_ASSERT_MSG(mGpuBaseAddr == nullptr, "deinit me[%s] before destroy me", mDesc.mName);
}

void Framework::Texture::init(const Description& desc, Allocators *allocators, const TextureSourcePixelData *srcData)
{
	mDesc = desc;
	
	createTextureView();
	allocMemory(allocators);
	if (srcData != nullptr)
	{
		transferData(srcData);
	}
}

void Framework::Texture::deinit(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);
	allocators->release(mGpuBaseAddr, mGpuMemType, &mHandle);
	mGpuBaseAddr = nullptr;
	SAFE_DELETE(mTextureView);
}

Framework::U32 Framework::Texture::getTotalNumSlices() const
{
	U32 ret = 1;
	switch (mDesc.mTexType)
	{
	case Gnm::kTextureType3d:
		ret = mDesc.mDepth;
		break;
	case Gnm::kTextureTypeCubemap:
		ret = 6;
		break;
	case Gnm::kTextureType1dArray:
	case Gnm::kTextureType2dArray:
		ret = mDesc.mNumSlices;
		break;
	case Gnm::kTextureType2dMsaa:
		ret = mDesc.mFragments;
		break;
	case Gnm::kTextureType2dArrayMsaa:
		ret = mDesc.mFragments * mDesc.mNumSlices;
		break;
	default:
		break;
	}
	return ret;
}

void Framework::Texture::createTextureView()
{
	SCE_GNM_ASSERT(mDesc.mWidth > 0 && mDesc.mHeight > 0 && mDesc.mDepth > 0);
	SCE_GNM_ASSERT(mDesc.mMipLevels > 0);

	// create shader resource view
	TextureView::Description _desc;
	_desc.mWidth		= mDesc.mWidth;
	_desc.mHeight		= mDesc.mHeight;
	_desc.mDepth		= mDesc.mDepth;
	_desc.mMipLevels	= mDesc.mMipLevels;
	_desc.mPitch		= mDesc.mPitch;
	_desc.mNumSlices	= mDesc.mNumSlices;
	_desc.mFragments	= mDesc.mFragments;
	_desc.mFormat		= mDesc.mFormat;
	_desc.mTileMode		= mDesc.mTileMode;
	_desc.mTexType		= mDesc.mTexType;

	mTextureView = new TextureView(_desc);
}

void Framework::Texture::allocMemory(Allocators *allocators)
{
	SCE_GNM_ASSERT(mTextureView != nullptr);
	SCE_GNM_ASSERT(allocators != nullptr);

	Gnm::SizeAlign _gpuMemAlign = mTextureView->getSizeAlign();

	mGpuMemType = mDesc.mIsDynamic ? SCE_KERNEL_WB_ONION : SCE_KERNEL_WC_GARLIC;
	allocators->allocate(&mGpuBaseAddr, mGpuMemType, _gpuMemAlign, Gnm::kResourceTypeTextureBaseAddress, &mHandle, mDesc.mName);
	SCE_GNM_ASSERT_MSG(mGpuBaseAddr != nullptr, "Out of memory");

	mTextureView->assignAddress(mGpuBaseAddr);
}

void Framework::Texture::transferData(const TextureSourcePixelData *srcData)
{
	// TODO lock / unlock texture
	SCE_GNM_ASSERT(srcData != nullptr);

	Gnm::Texture *_texture = mTextureView->getInternalObj();

	U32 _totalNumSlices = getTotalNumSlices();
	GpuAddress::TilingParameters _tilingParam;
	for (U32 mipLevel = 0; mipLevel < mDesc.mMipLevels; mipLevel++)
	{
		for (U32 arraySlice = 0; arraySlice < _totalNumSlices; arraySlice++)
		{
			U64 _mipOffset;
			U64 _mipSize;
			GpuAddress::computeTextureSurfaceOffsetAndSize(&_mipOffset, &_mipSize, _texture, mipLevel, arraySlice);

			U32 _srcPixelsOffset = (*(srcData->mOffsetSolver))(mDesc, mipLevel, arraySlice);

			_tilingParam.initFromTexture(_texture, mipLevel, arraySlice);
			Result ret = GpuAddress::tileSurface((U8 *)mGpuBaseAddr + _mipOffset, srcData->mDataPtr + _srcPixelsOffset, &_tilingParam);
			SCE_GNM_ASSERT(ret == (Result)GpuAddress::kStatusSuccess);
		}
	}
}

