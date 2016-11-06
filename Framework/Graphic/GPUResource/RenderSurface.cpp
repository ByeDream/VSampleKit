#include "stdafx.h"

#include "RenderSurface.h"
#include "RenderableTexture.h"
#include "GraphicHelpers.h"
#include "GPUResourceViews.h"
#include "ChunkBasedRenderContext/RenderContext.h"

using namespace sce;

Framework::RenderSurface::RenderSurface()
{

}

Framework::RenderSurface::~RenderSurface()
{
	SCE_GNM_ASSERT_MSG(mTexture == nullptr, "deinit me[%s] before destroy me", mTexture->getDescription().mName);
}

void Framework::RenderSurface::init(const BaseGPUResource::Description *desc, Allocators *allocators)
{
	const RenderSurface::Description *_desc = typeCast<BaseGPUResource::Description, RenderSurface::Description>(desc);

	mAAType = _desc->mAAType;

	GpuAddress::SurfaceType _type = _desc->mType;
	Gnm::NumFragments _fragments = getFragmentsFromAAType(mAAType);
	Gnm::NumSamples _samples = getSamplesFromAAType(mAAType);

	Result ret = GpuAddress::computeSurfaceTileMode(Gnm::getGpuMode(), &mTileMode, _type, _desc->mFormat, convertNumFragments(_fragments)); //TODO mFragments
	SCE_GNM_ASSERT_MSG(ret == (Result)GpuAddress::kStatusSuccess, "Compute surface tile mode failed.");

	Texture::Description _textureDesc;
	_textureDesc.mWidth				= _desc->mWidth;
	_textureDesc.mHeight			= _desc->mHeight;
	_textureDesc.mDepth				= _desc->mDepth;
	_textureDesc.mPitch				= 0;
	_textureDesc.mNumSlices			= 1; // TODO support texture array
	_textureDesc.mMipLevels			= _desc->mMipLevels;
	_textureDesc.mFormat			= _desc->mFormat;
	_textureDesc.mTileMode			= mTileMode;
	_textureDesc.mFragments			= _fragments;
	_textureDesc.mName				= _desc->mName;

	sce::Gnm::TextureType _2DTextureType;
	if (_textureDesc.mNumSlices == 1)
	{
		_2DTextureType = (_textureDesc.mFragments == Gnm::kNumFragments1) ? Gnm::kTextureType2d : Gnm::kTextureType2dMsaa;
	}
	else if (_textureDesc.mNumSlices > 1)
	{
		_2DTextureType = (_textureDesc.mFragments == Gnm::kNumFragments1) ? Gnm::kTextureType2dArray : Gnm::kTextureType2dArrayMsaa;
	}
	else
	{
		SCE_GNM_ASSERT(false);
	}


	// TODO check kSurfaceTypeDepthOnlyTarget stencil ?

	//TODO  kTextureType1d 
	switch (_type)
	{
	case GpuAddress::kSurfaceTypeColorTarget:
	case GpuAddress::kSurfaceTypeColorTargetDisplayable:
	{
		if (mAAType != AA_NONE)
		{
			SCE_GNM_ASSERT_MSG(_desc->mEnableCMask && _desc->mEnableFMask, "Enable cmask & fmask if you need msaa");
		}
		mTexture = new RenderableTextureColor(_type == GpuAddress::kSurfaceTypeColorTargetDisplayable, _desc->mEnableCMask, _desc->mEnableFMask, _samples);
		_textureDesc.mTexType = _2DTextureType;
		if ((_type == GpuAddress::kSurfaceTypeColorTargetDisplayable) && _desc->mIsDynamicDisplayableColorTarget)
		{
			mTileMode = Gnm::kTileModeDisplay_LinearAligned;
			_textureDesc.mTileMode = mTileMode;
			_textureDesc.mIsDynamic = true;
		}
		else
		{
			_textureDesc.mIsDynamic = false;
		}
	}
	break;
	case GpuAddress::kSurfaceTypeDepthOnlyTarget:
	{
		mTexture = new RenderableTextureDepthStencil(_desc->mEnableHTile, _desc->mEnableStencil);
		_textureDesc.mTexType = _2DTextureType;
		_textureDesc.mIsDynamic = false;
	}
	break;
	case GpuAddress::kSurfaceTypeTextureFlat:
	case GpuAddress::kSurfaceTypeRwTextureFlat:
	{
		mTexture = new Texture();
		_textureDesc.mTexType = _2DTextureType;
		_textureDesc.mIsDynamic = (_type == GpuAddress::kSurfaceTypeRwTextureFlat);
	}
	break;
	case GpuAddress::kSurfaceTypeTextureVolume:
	case GpuAddress::kSurfaceTypeRwTextureVolume:
	{
		mTexture = new Texture();
		_textureDesc.mTexType = Gnm::kTextureType3d;
		_textureDesc.mIsDynamic = (_type == GpuAddress::kSurfaceTypeRwTextureVolume);
	}
	break;
	case GpuAddress::kSurfaceTypeTextureCubemap:
	case GpuAddress::kSurfaceTypeRwTextureCubemap:
	{
		mTexture = new Texture();
		_textureDesc.mTexType = Gnm::kTextureTypeCubemap;
		_textureDesc.mIsDynamic = (_type == GpuAddress::kSurfaceTypeRwTextureCubemap);
	}
	break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}

	mTexture->init(_textureDesc, allocators, _desc->mSrcData);

	// TODO sampler
	//m_GfxTexture->GetPtr()->SetTextureAddress(GFX_TEX_ADDRESS_CLAMP, GFX_TEX_ADDRESS_CLAMP, GFX_TEX_ADDRESS_CLAMP);
	//m_GfxTexture->GetPtr()->SetTextureFilter(GFX_TEX_FILTER_LINEAR, GFX_TEX_FILTER_LINEAR, GFX_TEX_FILTER_NONE);
}

void Framework::RenderSurface::deinit(Allocators *allocators)
{
	mTexture->deinit(allocators);
	SAFE_DELETE(mTexture);
}



void Framework::RenderSurface::bindAsSampler(RenderContext *context, U32 soltID) const
{
	SCE_GNM_ASSERT(context != nullptr);
	context->setTextureSurface(soltID, this);
}

void Framework::RenderSurface::bindAsRenderTarget(RenderContext *context, U32 soltID) const
{
	SCE_GNM_ASSERT(context != nullptr);
	context->setRenderTargetSurface(soltID, this);
}

void Framework::RenderSurface::bindAsDepthStencilTarget(RenderContext *context) const
{
	SCE_GNM_ASSERT(context != nullptr);
	context->setDepthStencilTargetSurface(this);
}

bool Framework::RenderSurface::isFormat32() const
{
	Gnm::SurfaceFormat fmt = mTexture->getDescription().mFormat.getSurfaceFormat();
	return fmt == sce::Gnm::kSurfaceFormat32 ||
		fmt == sce::Gnm::kSurfaceFormat32_32 ||
		fmt == sce::Gnm::kSurfaceFormat32_32_32 ||
		fmt == sce::Gnm::kSurfaceFormat32_32_32_32 ||
		fmt == sce::Gnm::kSurfaceFormat24_8 ||
		fmt == sce::Gnm::kSurfaceFormatX24_8_32;
}

void * Framework::RenderSurface::getBaseAddress() const
{
	SCE_GNM_ASSERT(mTexture != nullptr);
	return mTexture->getShaderResourceView()->getInternalObj()->getBaseAddress();
}
