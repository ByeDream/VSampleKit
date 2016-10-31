#include "stdafx.h"

#include "RenderSurface.h"
#include "RenderableTexture.h"
#include "GraphicHelpers.h"

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
	GpuAddress::SurfaceType _type = desc.mType;
	Gnm::NumFragments _fragments = getFragmentsFromAAType(desc.mAAType);
	Gnm::NumSamples _samples = getSamplesFromAAType(desc.mAAType);

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
			if (desc.mAAType != AA_NONE)
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
}

void Framework::RenderSurface::deinit(Allocators *allocators)
{
	mTexture->deinit(allocators);
	SAFE_DELETE(mTexture);
}
