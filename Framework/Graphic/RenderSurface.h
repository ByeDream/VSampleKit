#pragma once

namespace Framework
{
	class Allocators;
	class Texture;

	class RenderSurface
	{
	public:

		struct Description
		{
			U32								mWidth{ 0 };
			U32								mHeight{ 0 };
			U32								mDepth{ 1 };
			U32								mMipLevels{ 1 };

			bool							mIsDynamic{ false };
			bool							mEnableCMask{ false };
			bool							mEnableFMask{ false };
			bool							mEnableHTile{ true };
			bool							mEnableStencil{ true };

			sce::Gnm::DataFormat			mFormat{ sce::Gnm::kDataFormatInvalid };
			AntiAliasingType				mAAType{ AA_NONE };
			sce::GpuAddress::SurfaceType	mType{ sce::GpuAddress::kSurfaceTypeTextureFlat };

			const char *					mName{ nullptr };
		};

		RenderSurface();
		virtual ~RenderSurface();

		virtual void						init(const Description& desc, Allocators *allocators, const U8 *pData);
		virtual void						deinit(Allocators *allocators);

	protected:
		Texture *							mTexture{ nullptr };
		sce::Gnm::TileMode					mTileMode{ sce::Gnm::kInvaildTileMode };
	};
}
