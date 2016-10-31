#include "stdafx.h"

#include "RenderSurface.h"
#include "RenderableTexture.h"
#include "GraphicHelpers.h"
#include "GPUResourceViews.h"
#include "RenderContext.h"

using namespace sce;

Framework::RenderSurface::RenderSurface()
{

}

Framework::RenderSurface::~RenderSurface()
{
	SCE_GNM_ASSERT_MSG(mTexture == nullptr, "deinit me[%s] before destroy me", mTexture->getDescription().mName);
}

void Framework::RenderSurface::init(const Description& desc, Allocators *allocators, const U8 *pData)
{
	mAAType = desc.mAAType;

	GpuAddress::SurfaceType _type = desc.mType;
	Gnm::NumFragments _fragments = getFragmentsFromAAType(mAAType);
	Gnm::NumSamples _samples = getSamplesFromAAType(mAAType);

	Result ret = GpuAddress::computeSurfaceTileMode(Gnm::getGpuMode(), &mTileMode, _type, desc.mFormat, convertNumFragments(_fragments)); //TODO mFragments
	SCE_GNM_ASSERT_MSG(ret == (Result)GpuAddress::kStatusSuccess, "Compute surface tile mode failed.");

	Texture::Description _desc;
	_desc.mWidth				= desc.mWidth;
	_desc.mHeight				= desc.mHeight;
	_desc.mDepth				= desc.mDepth;
	_desc.mMipLevels			= desc.mMipLevels;
	_desc.mFormat				= desc.mFormat;
	_desc.mTileMode				= mTileMode;
	_desc.mFragments			= _fragments;
	_desc.mName					= desc.mName;

	// 	if (desc.mIsDynamic)
	// 		_tileMode = Gnm::kTileModeDisplay_LinearAligned;

	// TODO check kSurfaceTypeDepthOnlyTarget stencil ?
	switch (_type)
	{
	case GpuAddress::kSurfaceTypeColorTarget:
	case GpuAddress::kSurfaceTypeColorTargetDisplayable:
		{
			if (mAAType != AA_NONE)
			{
				SCE_GNM_ASSERT_MSG(desc.mEnableCMask && desc.mEnableFMask, "Enable cmask & fmask if you need msaa");
			}
			mTexture = new RenderableTextureColor(_type == GpuAddress::kSurfaceTypeColorTargetDisplayable, desc.mEnableCMask, desc.mEnableFMask, _samples);
			_desc.mTexType = Gnm::kTextureType2d;
			_desc.mIsDynamic = false;
		}
		break;
	case GpuAddress::kSurfaceTypeDepthOnlyTarget:
		{
			mTexture = new RenderableTextureDepthStencil(desc.mEnableHTile, desc.mEnableStencil);
			_desc.mTexType = Gnm::kTextureType2d;
			_desc.mIsDynamic = false;
		}
		break;
	case GpuAddress::kSurfaceTypeTextureFlat:
	case GpuAddress::kSurfaceTypeRwTextureFlat:
		{
			mTexture = new Texture();
			_desc.mTexType = Gnm::kTextureType2d; //TODO ignore 1D texture at the moment
			_desc.mIsDynamic = (_type == GpuAddress::kSurfaceTypeRwTextureFlat);
		}
		break;
	case GpuAddress::kSurfaceTypeTextureVolume:
	case GpuAddress::kSurfaceTypeRwTextureVolume:
		{
			mTexture = new Texture();
			_desc.mTexType = Gnm::kTextureType3d;
			_desc.mIsDynamic = (_type == GpuAddress::kSurfaceTypeRwTextureVolume);
		}
		break;
	case GpuAddress::kSurfaceTypeTextureCubemap:
	case GpuAddress::kSurfaceTypeRwTextureCubemap:
		{
			mTexture = new Texture();
			_desc.mTexType = Gnm::kTextureTypeCubemap;
			_desc.mIsDynamic = (_type == GpuAddress::kSurfaceTypeRwTextureCubemap);
		}
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}

	mTexture->init(_desc, allocators, pData);

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
