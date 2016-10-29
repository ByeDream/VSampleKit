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
		struct Description
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			U32 mDepth{ 1 };
			U32 mMipLevels{ 1 };
			U32 mPitch{ 0 };
			U32 mNumSlices{ 1 };
			sce::Gnm::NumFragments mFragments{ sce::Gnm::kNumFragments1 };
			sce::Gnm::DataFormat mFormat{ sce::Gnm::kDataFormatInvalid };
			sce::Gnm::TextureType mTexType{ sce::Gnm::kTextureType2d };
			bool mIsDynamic{ false };
		};

		TextureView(const Description &desc);
		void assignAddress(void *baseAddr);

		inline sce::Gnm::SizeAlign getSizeAlign() const { return mObject.getSizeAlign(); }
	};

	class BaseTargetView : public GPUResourceView
	{
	public:
		struct Description
		{
			U32							mWidth{ 0 };
			U32							mHeight{ 0 };
			U32							mPitch{ 0 };
			U32							mNumSlices{ 1 };
			sce::Gnm::NumSamples		mSamples{ sce::Gnm::kNumSamples1 };
			sce::Gnm::NumFragments		mFragments{ sce::Gnm::kNumFragments1 };
			sce::Gnm::DataFormat		mColorFormat{ sce::Gnm::kDataFormatInvalid };
			sce::Gnm::ZFormat			mDepthFormat{ sce::Gnm::kZFormatInvalid };
			sce::Gnm::StencilFormat		mStencilFormat{ sce::Gnm::kStencilInvalid };
			bool						mIsDynamic{ false };
			bool						mIsDisplayable{ false };
			bool						mUseCMask{ false };
			bool						mUseFMask{ false };
			bool						mUseHTile{ false };
		};
	};

	class RenderTargetView : public BaseTargetView
	{
		INTERNAL_OBJ(sce::Gnm::RenderTarget);

	public:
		RenderTargetView(const BaseTargetView::Description &desc);
		void assignAddress(void *colorAddr, void *cMaskAddr = nullptr, void *fMaskAddr = nullptr);

		inline bool				isUsingCMask() const { return mUseCMask; }
		inline bool				isUsingFMask() const { return mUseFMask; }

		inline sce::Gnm::SizeAlign getColorSizeAlign() const { return mObject.getColorSizeAlign(); }
		inline sce::Gnm::SizeAlign getCMaskSizeAlign() const { return mObject.getCmaskSizeAlign(); }
		inline sce::Gnm::SizeAlign getFMaskSizeAlign() const { return mObject.getFmaskSizeAlign(); }

	protected:
		bool					mUseCMask{ false };
		bool					mUseFMask{ false };
	};

	class DepthStencilView : public BaseTargetView
	{
		INTERNAL_OBJ(sce::Gnm::DepthRenderTarget);

	public:
		DepthStencilView(const BaseTargetView::Description &desc);
		void assignAddress(void *depthAddr, void *stencilAddr = nullptr, void *hTileAddr = nullptr);

		inline bool				isUsingStencil() const { return mUseStencil; }
		inline bool				isUsingeHTile() const { return mUseHTile; }

		inline sce::Gnm::SizeAlign getDepthSizeAlign() const { return mObject.getZSizeAlign(); }
		inline sce::Gnm::SizeAlign getStencilSizeAlign() const { return mObject.getStencilSizeAlign(); }
		inline sce::Gnm::SizeAlign getHTileSizeAlign() const { return mObject.getHtileSizeAlign(); }

	protected:
		bool					mUseStencil{ false };
		bool					mUseHTile{ false };
	};
}
