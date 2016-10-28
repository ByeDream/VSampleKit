#pragma once

#define INTERNAL_OBJ(_typename) \
public: inline _typename *getInternalObj() { return &mObject; } \
inline const _typename *getInternalObj() const { return &mObject; } \
private: _typename mObject

#define INTERNAL_OBJ_PTR(_typename) _typename *mObject{ nullptr }; \
inline _typename *getInternalObj() const { SCE_GNM_ASSERT(mObject != nullptr); return mObject; } \
//inline _typename** GetGNMObjectInputPtr() { return &m_GNMResource; }

namespace Framework
{
	// base class
	class GPUResourceView
	{
	public:
		virtual ~GPUResourceView() {}
	};

	class TextureView : public GPUResourceView
	{
		INTERNAL_OBJ(sce::Gnm::Texture);

	public:
		struct Desciption
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mDepth{ 1 };
			U32 mMipLevels{ 1 };
			U32 mPitch{ 0 };
			U32 mNumSlices{ 1 };
			sce::Gnm::DataFormat mFormat{ sce::Gnm::kDataFormatInvalid };
			sce::Gnm::TextureType mTexType{ sce::Gnm::kTextureType2d };
			bool mIsDynamic{ false };
			AntiAliasingType mAAType{ AA_NONE };
		};

		TextureView(const Desciption &desc);
		void assignAddress(void *baseAddr);
	};

	class RenderTargetView : public GPUResourceView
	{
		INTERNAL_OBJ(sce::Gnm::RenderTarget);

	public:
		struct Desciption
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mPitch{ 0 };
			U32 mNumSlices{ 1 };
			sce::Gnm::DataFormat mFormat{ sce::Gnm::kDataFormatInvalid };
			bool mIsDynamic{ false };
			bool mIsDisplayable{ false };
			AntiAliasingType mAAType{ AA_NONE };
		};

		RenderTargetView(const Desciption &desc);
		void assignAddress(void *colorAddr, void *cMaskAddr = nullptr, void *fMaskAddr = nullptr);

		inline bool				IsUsingCMask() const { return mUseCMask; }
		inline bool				IsUsingFMask() const { return mUseFMask; }

		inline sce::Gnm::SizeAlign GetColorSizeAlign() const { return mObject.getColorSizeAlign(); }
		inline sce::Gnm::SizeAlign GetCMaskSizeAlign() const { return mObject.getCmaskSizeAlign(); }
		inline sce::Gnm::SizeAlign GetFMaskSizeAlign() const { return mObject.getFmaskSizeAlign(); }

	private:
		bool					mUseCMask{ false };
		bool					mUseFMask{ false };
	};

	class DepthStencilView : public GPUResourceView
	{
		INTERNAL_OBJ(sce::Gnm::DepthRenderTarget);

	public:
		struct Desciption
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mPitch{ 0 };
			U32 mNumSlices{ 1 };
			sce::Gnm::ZFormat mZFormat{sce::Gnm::kZFormatInvalid};
			sce::Gnm::StencilFormat mSFormat{ sce::Gnm::kStencilInvalid };
			bool mUseHTile{ true };
			AntiAliasingType mAAType{ AA_NONE };
		};

		DepthStencilView(const Desciption &desc);
		void assignAddress(void *depthAddr, void *hTileAddr, void *stencilAddr = nullptr);

		inline bool				IsUsingStencil() const { return mUseStencil; }
		inline bool				IsUsingeHTile() const { return mUseHTile; }

		inline sce::Gnm::SizeAlign GetDepthSizeAlign() const { return mObject.getZSizeAlign(); }
		inline sce::Gnm::SizeAlign GetStencilSizeAlign() const { return mObject.getStencilSizeAlign(); }
		inline sce::Gnm::SizeAlign GetHTileSizeAlign() const { return mObject.getHtileSizeAlign(); }

	private:
		bool					mUseStencil{ false };
		bool					mUseHTile{ false };
	};
}
