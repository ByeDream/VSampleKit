#pragma once

#include "GPUResource.h"

namespace Framework
{
	class Allocators;
	class Texture;
	class RenderContext;
	struct TextureSourcePixelData;

	class RenderSurface : public BaseGPUResource
	{
	public:
		struct Description : public BaseGPUResource::Description
		{
			U32								mWidth{ 0 };
			U32								mHeight{ 0 };
			U32								mDepth{ 1 };
			U32								mMipLevels{ 1 };

			bool							mEnableCMask{ false };
			bool							mEnableFMask{ false };
			bool							mEnableHTile{ true };
			bool							mEnableStencil{ true };
			bool							mIsDynamicDisplayableColorTarget{ false };

			sce::Gnm::DataFormat			mFormat{ sce::Gnm::kDataFormatInvalid };
			AntiAliasingType				mAAType{ AA_NONE };
			sce::GpuAddress::SurfaceType	mType{ sce::GpuAddress::kSurfaceTypeTextureFlat };

			TextureSourcePixelData *		mSrcData{ nullptr };

			const char *					mName{ nullptr };
		};

		RenderSurface();
		virtual ~RenderSurface();

		virtual void						init(const BaseGPUResource::Description *desc, Allocators *allocators);
		virtual void						deinit(Allocators *allocators);

		virtual void						bindAsSampler(RenderContext *context, U32 soltID) const;
		virtual void						bindAsRenderTarget(RenderContext *context, U32 soltID) const;
		virtual void						bindAsDepthStencilTarget(RenderContext *context) const;

		virtual bool						isFormat32() const;

		inline const Texture *				getTexture() const { return mTexture; }
		inline Texture *					getTexture() { return mTexture; }
		inline sce::Gnm::TileMode			getTileMode() const { return mTileMode; }
		inline AntiAliasingType				getAAType() const { return mAAType; }

		void *								getBaseAddress() const;
		//TODO for size override operator > >= < <=
		//TODO inline SurfaceSet GetSurfaceSet() { return mSet; } // or the name is enough
	protected:
		Texture *							mTexture{ nullptr };
		sce::Gnm::TileMode					mTileMode{ sce::Gnm::kTileModeThin_2dThin };
		AntiAliasingType					mAAType{ AA_NONE };
	};
}
