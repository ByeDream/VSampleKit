#pragma once

namespace Framework
{
	class Allocators;
	class TextureView;

	class Texture
	{
	public:

		struct Desciption
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mDepth{ 1 };
			U32 mMipLevels{ 1 };
			sce::Gnm::DataFormat mFormat{ sce::Gnm::kDataFormatInvalid };
			sce::Gnm::TextureType mTexType{ sce::Gnm::kTextureType2d };
			bool mIsDynamic{ false };
			AntiAliasingType mAAType{ AA_NONE };
			//RESOURCE_BIND_FLAG mBindFlag{ BIND_SHADER_RESOURCE };

			const char *mName{ nullptr };
		};

		Texture();
		virtual ~Texture();

		void						init(const Desciption& desc, Allocators *allocators, const U8 *pData);
		void						deinit(Allocators *allocators);

		static Texture *			createTexture(const Desciption& desc, Allocators *allocators, const U8 *pData = nullptr);
		static Texture *			createTextureFromFile(const char *filePath, Allocators *allocators);

		static bool					saveTextureToFile(const char *filePath, Texture *texture);
	protected:
		void						createShaderResourceView();
		void						allocMemory(Allocators *allocators);
		void						transferData(const U8 *pData);

		static void					parseTexture(const U8 *fileBuffer, Desciption *out_desc, U8 **out_pixelData);

	protected:
		Desciption					mDesc;
		TextureView *				mShaderResourceView{ nullptr };
		sce::Gnm::ResourceHandle	mHandle{ sce::Gnm::kInvalidResourceHandle };
		SceKernelMemoryType			mGpuMemType{ SCE_KERNEL_WC_GARLIC };
		void *						mGpuBaseAddr{ nullptr };
	};
}