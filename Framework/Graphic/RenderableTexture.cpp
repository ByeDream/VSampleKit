#include "stdafx.h"

#include "RenderableTexture.h"

#include "Memory/Allocators.h"
#include "GPUResourceViews.h"

using namespace sce;

void Framework::RenderableTexture::init(const Texture::Description &desc, Allocators *allocators, const U8 *pData)
{
	super::init(desc, allocators, pData);
	createTargetView();
	allocMemory(allocators);
}

void Framework::RenderableTexture::deinit(Allocators *allocators)
{
	SAFE_DELETE(mTargetView);
	super::deinit(allocators);
}

// Framework::RenderableTexture * Framework::RenderableTexture::CreateRenderableTextureColor(const Texture::Description &desc, Allocators *allocators, bool isDisplayable /*= false*/)
// {
// 	bool _useCMask = (desc.mAAType != AA_NONE);
// 	bool _useFMask = (desc.mAAType != AA_NONE);
// 
// 	RenderableTexture *_texture = new RenderableTextureColor(isDisplayable, _useCMask, _useFMask);
// 	_texture->init(desc, allocators);
// 	return _texture;
// }

// Framework::RenderableTexture * Framework::RenderableTexture::CreateRenderableTextureDepthStencil(const Texture::Description &desc, Allocators *allocators, bool isUsingHTile /*= true*/, bool isUsingStencil /*= true*/)
// {
// 	RenderableTexture *_texture = new RenderableTextureDepthStencil(isUsingHTile, isUsingStencil);
// 	_texture->init(desc, allocators);
// 	return _texture;
// }

Framework::RenderableTextureColor::RenderableTextureColor(bool isDisplayable, bool isUsingCMask, bool isUsingFMask, sce::Gnm::NumSamples samples)
	: mIsDisplayable(isDisplayable)
	, mIsUsingCMask(isUsingCMask)
	, mIsUsingFMask(isUsingFMask)
	, mSamples(samples)
{

}

Framework::RenderableTextureColor::~RenderableTextureColor()
{
	SCE_GNM_ASSERT_MSG(mCMaskGpuMemAddr == nullptr && mFMaskGpuMemAddr == nullptr, "deinit me[%s] before destroy me", mDesc.mName);
}

void Framework::RenderableTextureColor::deinit(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);
	allocators->release(mFMaskGpuMemAddr, mGpuMemType, &mFMaskHandle);
	mFMaskGpuMemAddr = nullptr;
	allocators->release(mFMaskGpuMemAddr, mGpuMemType, &mFMaskHandle);
	mFMaskGpuMemAddr = nullptr;
	super::deinit(allocators);
}

void Framework::RenderableTextureColor::createTargetView()
{
	SCE_GNM_ASSERT(mTargetView == nullptr);

	BaseTargetView::Description _desc;
	_desc.mWidth				= mDesc.mWidth;
	_desc.mHeight				= mDesc.mHeight;
	_desc.mSamples				= mSamples;
	_desc.mFragments			= mDesc.mFragments;
	_desc.mColorFormat			= mDesc.mFormat;
	_desc.mTileMode				= mDesc.mTileMode;
	_desc.mIsDisplayable		= mIsDisplayable;
	_desc.mUseCMask				= mIsUsingCMask;
	_desc.mUseFMask				= mIsUsingFMask;

	mTargetView = new RenderTargetView(_desc);
	SCE_GNM_ASSERT(mTargetView != nullptr);
}

void Framework::RenderableTextureColor::allocMemory(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);

	SCE_GNM_ASSERT(mShaderResourceView != nullptr);
	SCE_GNM_ASSERT(mGpuBaseAddr != nullptr);

	SCE_GNM_ASSERT(mTargetView != nullptr);
	SCE_GNM_ASSERT(mCMaskGpuMemAddr == nullptr);
	SCE_GNM_ASSERT(mFMaskGpuMemAddr == nullptr);

	RenderTargetView *_renderTargetView = typeCast<BaseTargetView, RenderTargetView>(mTargetView);

	Gnm::SizeAlign _shaderResourceSizeAlign = mShaderResourceView->getSizeAlign();
	Gnm::SizeAlign _colorSizeAlign = _renderTargetView->getColorSizeAlign();

	SCE_GNM_ASSERT(_colorSizeAlign.m_size == _shaderResourceSizeAlign.m_size && _colorSizeAlign.m_align == _shaderResourceSizeAlign.m_align);

	if (_renderTargetView->isUsingCMask())
	{
		Gnm::SizeAlign _cMaskSizeAlign = _renderTargetView->getCMaskSizeAlign();
		allocators->allocate(&mCMaskGpuMemAddr, mGpuMemType, _cMaskSizeAlign, Gnm::kResourceTypeRenderTargetCMaskAddress, &mCMaskHandle, "%s_cmask", mDesc.mName);
		SCE_GNM_ASSERT_MSG(mCMaskGpuMemAddr != nullptr, "Out of memory");
	}

	if (_renderTargetView->isUsingFMask())
	{
		Gnm::SizeAlign _fMaskSizeAlign = _renderTargetView->getFMaskSizeAlign();
		allocators->allocate(&mFMaskGpuMemAddr, mGpuMemType, _fMaskSizeAlign, Gnm::kResourceTypeRenderTargetFMaskAddress, &mFMaskHandle, "%s_fmask", mDesc.mName);
		SCE_GNM_ASSERT_MSG(mFMaskGpuMemAddr != nullptr, "Out of memory");
	}

	_renderTargetView->assignAddress(mGpuBaseAddr, mCMaskGpuMemAddr, mFMaskGpuMemAddr);
}

Framework::RenderableTextureDepthStencil::RenderableTextureDepthStencil(bool isUsingHTile, bool isUsingStencil)
	: mIsUsingHTile(isUsingHTile)
	, mIsUsingStencil(isUsingStencil)
{

}

Framework::RenderableTextureDepthStencil::~RenderableTextureDepthStencil()
{
	SCE_GNM_ASSERT_MSG(mStencilGpuMemAddr == nullptr && mHTileGpuMemAddr == nullptr, "deinit me[%s] before destroy me", mDesc.mName);
}

void Framework::RenderableTextureDepthStencil::deinit(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);
	allocators->release(mHTileGpuMemAddr, mGpuMemType, &mHTileHandle);
	mHTileGpuMemAddr = nullptr;
	allocators->release(mStencilGpuMemAddr, mGpuMemType, &mStencilHandle);
	mStencilGpuMemAddr = nullptr;
	super::deinit(allocators);
}

void Framework::RenderableTextureDepthStencil::createTargetView()
{
	SCE_GNM_ASSERT(mTargetView == nullptr);

	BaseTargetView::Description _desc;
	_desc.mWidth				= mDesc.mWidth;
	_desc.mHeight				= mDesc.mHeight;
	_desc.mFragments			= mDesc.mFragments;
	_desc.mDepthFormat			= mDesc.mFormat.getZFormat();
	_desc.mStencilFormat		= mIsUsingStencil ? Gnm::kStencil8 : Gnm::kStencilInvalid;
	_desc.mTileMode				= mDesc.mTileMode;
	_desc.mUseHTile				= mIsUsingHTile;

	mTargetView = new DepthStencilView(_desc);
	SCE_GNM_ASSERT(mTargetView != nullptr);
}

void Framework::RenderableTextureDepthStencil::allocMemory(Allocators *allocators)
{
	SCE_GNM_ASSERT(allocators != nullptr);

	SCE_GNM_ASSERT(mShaderResourceView != nullptr);
	SCE_GNM_ASSERT(mGpuBaseAddr != nullptr);

	SCE_GNM_ASSERT(mTargetView != nullptr);
	SCE_GNM_ASSERT(mStencilGpuMemAddr == nullptr);
	SCE_GNM_ASSERT(mHTileGpuMemAddr == nullptr);

	DepthStencilView *_depthStencilView = typeCast<BaseTargetView, DepthStencilView>(mTargetView);

	Gnm::SizeAlign _shaderResourceSizeAlign = mShaderResourceView->getSizeAlign();
	Gnm::SizeAlign _depthSizeAlign = _depthStencilView->getDepthSizeAlign();

	SCE_GNM_ASSERT(_depthSizeAlign.m_size == _shaderResourceSizeAlign.m_size && _depthSizeAlign.m_align == _shaderResourceSizeAlign.m_align);

	if (_depthStencilView->isUsingStencil())
	{
		Gnm::SizeAlign _stencilSizeAlign = _depthStencilView->getStencilSizeAlign();
		allocators->allocate(&mStencilGpuMemAddr, mGpuMemType, _stencilSizeAlign, Gnm::kResourceTypeDepthRenderTargetStencilAddress, &mStencilHandle, "%s_stencil", mDesc.mName);
		SCE_GNM_ASSERT_MSG(mStencilGpuMemAddr != nullptr, "Out of memory");
	}

	if (_depthStencilView->isUsingeHTile())
	{
		Gnm::SizeAlign _hTileSizeAlign = _depthStencilView->getHTileSizeAlign();
		allocators->allocate(&mHTileGpuMemAddr, mGpuMemType, _hTileSizeAlign, Gnm::kResourceTypeDepthRenderTargetHTileAddress, &mHTileHandle, "%s_hTile", mDesc.mName);
		SCE_GNM_ASSERT_MSG(mHTileGpuMemAddr != nullptr, "Out of memory");
	}

	_depthStencilView->assignAddress(mGpuBaseAddr, mStencilGpuMemAddr, mHTileGpuMemAddr);
}
