#pragma once

namespace Framework
{
	class Allocators;
	class Texture;
	class RenderContext;

	class RenderSurface
	{
	public:
		typedef U32 Handle;
		enum
		{
			kInvalidRenderSurfaceHandle = MAX_VALUE_32,
		};

		struct Description
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

			const char *					mName{ nullptr };
		};

		RenderSurface();
		virtual ~RenderSurface();

		virtual void						init(const Description& desc, Allocators *allocators, const U8 *pData);
		virtual void						deinit(Allocators *allocators);

		virtual void						bindAsSampler(RenderContext *context, U32 soltID) const;
		virtual void						bindAsRenderTarget(RenderContext *context, U32 soltID) const;
		virtual void						bindAsDepthStencilTarget(RenderContext *context) const;

		virtual bool						isFormat32() const;

		inline void							setHandle(Handle handle) { mHandle = handle; }
		inline Handle						getHandle() const { return mHandle; }
		inline const Texture *				getTexture() const { return mTexture; }
		inline Texture *					getTexture() { return mTexture; }
		inline sce::Gnm::TileMode			getTileMode() const { return mTileMode; }
		inline AntiAliasingType				getAAType() const { return mAAType; }

		void *								getBaseAddress() const;
		//TODO for size override operator > >= < <=
		//TODO inline SurfaceSet GetSurfaceSet() { return mSet; } // or the name is enough
	protected:
		Handle								mHandle{ kInvalidRenderSurfaceHandle };
		Texture *							mTexture{ nullptr };
		sce::Gnm::TileMode					mTileMode{ sce::Gnm::kTileModeThin_2dThin };
		AntiAliasingType					mAAType{ AA_NONE };
	};
}
