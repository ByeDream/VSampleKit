#include "stdafx.h"

#include "Texture.h"

#include "Memory/Allocators.h"
#include "GPUResourceViews.h"
#include "GraphicHelpers.h"

using namespace sce;

Framework::Texture::Texture()
{

}

Framework::Texture::~Texture()
{
	SCE_GNM_ASSERT_MSG(mGpuBaseAddr == nullptr, "deinit me[%s] before destroy me", mDesc.mName);
}

void Framework::Texture::init(const Description& desc, Allocators *allocators, const U8 *pData)
{
	mDesc = desc;
	
	createShaderResourceView();
	allocMemory(allocators);
	if (pData != nullptr)
	{
		transferData(pData);
	}
}

void Framework::Texture::deinit(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);
	allocators->release(mGpuBaseAddr, mGpuMemType, &mHandle);
	mGpuBaseAddr = nullptr;
	SAFE_DELETE(mShaderResourceView);
}

Framework::Texture * Framework::Texture::createTexture(const Description& desc, Allocators *allocators, const U8 *pData /*= nullptr*/)
{
	Texture *_texture = new Texture;
	_texture->init(desc, allocators, pData);
	return _texture;
}

Framework::Texture * Framework::Texture::createTextureFromFile(const char *filePath, Allocators *allocators)
{
	FileIO _file(filePath);
	_file.load();

	Description _desc;
	U8 *_pixelData = nullptr;
	
	parseTexture(_file.getBuffer(), &_desc, &_pixelData);
	SCE_GNM_ASSERT(_pixelData != nullptr);
	_desc.mName = _file.getName();
	return createTexture(_desc, allocators, _pixelData);
}

bool Framework::Texture::saveTextureToFile(const char *filePath, Texture *texture)
{
	// TODO
	return false;
}

void Framework::Texture::createShaderResourceView()
{
	SCE_GNM_ASSERT(mDesc.mWidth > 0 && mDesc.mHeight > 0 && mDesc.mDepth > 0);
	SCE_GNM_ASSERT(mDesc.mMipLevels > 0);

	Gnm::NumFragments _fragments = getFragmentsFromAAType(mDesc.mAAType);

	// create shader resource view
	TextureView::Description _desc;
	_desc.mWidth		= mDesc.mWidth;
	_desc.mHeight		= mDesc.mHeight;
	_desc.mDepth		= mDesc.mDepth;
	_desc.mMipLevels	= mDesc.mMipLevels;
	_desc.mPitch		= 0;
	_desc.mNumSlices	= 1;
	_desc.mFragments	= _fragments;
	_desc.mFormat		= mDesc.mFormat;
	_desc.mTexType		= mDesc.mTexType;
	_desc.mIsDynamic	= mDesc.mIsDynamic;

	mShaderResourceView = new TextureView(_desc);
}

void Framework::Texture::allocMemory(Allocators *allocators)
{
	SCE_GNM_ASSERT(mShaderResourceView != nullptr);
	SCE_GNM_ASSERT(allocators != nullptr);

	Gnm::SizeAlign _gpuMemAlign = mShaderResourceView->getSizeAlign();

	mGpuMemType = mDesc.mIsDynamic ? SCE_KERNEL_WB_ONION : SCE_KERNEL_WC_GARLIC;
	allocators->allocate(&mGpuBaseAddr, mGpuMemType, _gpuMemAlign, Gnm::kResourceTypeTextureBaseAddress, &mHandle, mDesc.mName);
	SCE_GNM_ASSERT_MSG(mGpuBaseAddr != nullptr, "Out of memory");

	mShaderResourceView->assignAddress(mGpuBaseAddr);
}

void Framework::Texture::transferData(const U8 *pData)
{
	SCE_GNM_ASSERT(pData != nullptr);

	// TODO only support 1D & 2D texture with no mipmap at the moment.
	SCE_GNM_ASSERT_MSG(mDesc.mTexType == Gnm::kTextureType1d || mDesc.mTexType == Gnm::kTextureType2d, "Not support yet");
	SCE_GNM_ASSERT_MSG(mDesc.mTexType == Gnm::kTextureType1d || mDesc.mTexType == Gnm::kTextureType2d, "Not support yet");
	SCE_GNM_ASSERT_MSG(mDesc.mMipLevels == 1, "Not support yet");

	Gnm::Texture *_texture = mShaderResourceView->getInternalObj();

	//U32 _width = mShaderResourceView->getInternalObj()->getWidth();
	//uint32_t _height = mShaderResourceView->getInternalObj()->getHeight();

	//for (auto i = 0; i < mDesc.mMipLevels; i++)
	{
		GpuAddress::TilingParameters _tilingParam;
		U64 _mipOffset;
		U64 _mipSize;
		GpuAddress::computeTextureSurfaceOffsetAndSize(&_mipOffset, &_mipSize, _texture, 0, 0);
		//uint32_t srcPixelsOffset = ComputeScimitarTextureMapDataOffset(width, height, 1, 1, outTexture->getDataFormat(), false, 0, 0);
		U32 _srcPixelsOffset = 0;

		_tilingParam.initFromTexture(_texture, 0, 0);
		Result ret = GpuAddress::tileSurface((U8 *)mGpuBaseAddr + _mipOffset, pData + _srcPixelsOffset, &_tilingParam);
		SCE_GNM_ASSERT(ret == (Result)GpuAddress::kStatusSuccess);
	}
}

void Framework::Texture::parseTexture(const U8 *fileBuffer, Description *out_desc, U8 **out_pixelData)
{
	SCE_GNM_ASSERT(fileBuffer != nullptr && out_desc != nullptr && out_pixelData != nullptr);

	// TODO support other format, only support TGA at the moment.
	out_desc->mWidth		= (U32)tgaGetWidth(fileBuffer);
	out_desc->mHeight		= (U32)tgaGetHeight(fileBuffer);
	out_desc->mFormat		= Gnm::kDataFormatR8G8B8A8Unorm;

	*out_pixelData = (U8 *)tgaRead(fileBuffer, TGA_READER_ABGR);
}

