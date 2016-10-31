#pragma once

namespace Framework
{
	class Allocators;
	class TextureView;
	class BaseTargetView;

	class Texture
	{
	public:

		struct Description
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mDepth{ 1 };
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

		virtual void				init(const Description& desc, Allocators *allocators, const U8 *pData);
		virtual void				deinit(Allocators *allocators);

		virtual TextureView *		getShaderResourceView() const { return mShaderResourceView; }
		virtual BaseTargetView *	getTargetView() const { return nullptr; }

		inline const Description &	getDescription() const { return mDesc; }

		static Texture *			createTexture(const Description& desc, Allocators *allocators, const U8 *pData = nullptr);
		static Texture *			createTextureFromFile(const char *filePath, Allocators *allocators);

		static bool					saveTextureToFile(const char *filePath, Texture *texture);
	protected:
		void						createShaderResourceView();
		void						allocMemory(Allocators *allocators);
		void						transferData(const U8 *pData);

		static void					parseTexture(const U8 *fileBuffer, Description *out_desc, U8 **out_pixelData);

	protected:
		Description					mDesc;
		TextureView *				mShaderResourceView{ nullptr };
		sce::Gnm::ResourceHandle	mHandle{ sce::Gnm::kInvalidResourceHandle };
		SceKernelMemoryType			mGpuMemType{ SCE_KERNEL_WC_GARLIC };
		void *						mGpuBaseAddr{ nullptr };
	};
}