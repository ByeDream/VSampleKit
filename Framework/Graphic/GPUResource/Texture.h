#pragma once

namespace Framework
{
	class Allocators;
	class TextureView;
	class BaseTargetView;
	struct TextureSourcePixelData;

	class Texture
	{
	public:

		struct Description
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mDepth{ 1 };
			U32	mPitch{ 0 };
			U32	mNumSlices{ 1 };
			U32 mMipLevels{ 1 };
			sce::Gnm::DataFormat mFormat{ sce::Gnm::kDataFormatInvalid };
			sce::Gnm::TextureType mTexType{ sce::Gnm::kTextureType2d };
			sce::Gnm::TileMode mTileMode{ sce::Gnm::kTileModeThin_2dThin };
			sce::Gnm::NumFragments mFragments{ sce::Gnm::kNumFragments1 };
			bool mIsDynamic{ false };

			const char *mName{ nullptr };
		};

		Texture();
		virtual ~Texture();

		virtual void				init(const Description& desc, Allocators *allocators, const TextureSourcePixelData *srcData);
		virtual void				deinit(Allocators *allocators);

		virtual TextureView *		getShaderResourceView() const { return mShaderResourceView; }
		virtual BaseTargetView *	getTargetView() const { return nullptr; }

		inline const Description &	getDescription() const { return mDesc; }
		inline U32					getTotalNumSlices() const;

	protected:
		void						createShaderResourceView();
		void						allocMemory(Allocators *allocators);
		void						transferData(const TextureSourcePixelData *srcData);

	protected:
		Description					mDesc;
		TextureView *				mShaderResourceView{ nullptr };
		sce::Gnm::ResourceHandle	mHandle{ sce::Gnm::kInvalidResourceHandle };
		SceKernelMemoryType			mGpuMemType{ SCE_KERNEL_WC_GARLIC };
		void *						mGpuBaseAddr{ nullptr };
	};

	struct TextureSourcePixelData
	{
		typedef U32(*OffsetSolver)(const Texture::Description &desc, U32 mipLevel, U32 arraySlice);

		const U8 *		mDataPtr{ nullptr };
		OffsetSolver	mOffsetSolver{ nullptr };
	};
}