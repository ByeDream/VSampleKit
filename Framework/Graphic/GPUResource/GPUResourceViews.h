#pragma once

#define INTERNAL_OBJ(_typename) \
public: inline _typename *getInternalObj() { return &mObject; } \
inline const _typename *getInternalObj() const { return &mObject; } \
private: _typename mObject

#define INTERNAL_OBJ_PTR(_typename) \
public: inline _typename *getInternalObj() { SCE_GNM_ASSERT(mObject != nullptr); return mObject; } \
inline const _typename *getInternalObj() const { SCE_GNM_ASSERT(mObject != nullptr); return mObject; } \
private: _typename *mObject{ nullptr }

namespace Framework
{
	// Base class
	class GPUFence;
	class GPUResourceView
	{
	public:
		virtual ~GPUResourceView() {}

		inline void assignFence(GPUFence *fence) { mFence = fence; }
		inline GPUFence *getFence() const { return mFence; }
	protected:
		GPUFence *mFence;
	};

	// Shaders
	class BaseShaderView : public GPUResourceView
	{
	public:
		BaseShaderView(const U8 *pData);
		virtual void assignAddress(void *headerAddr, void *binaryAddr) = 0;

		virtual sce::Gnm::SizeAlign getHeaderSizeAlign() const = 0;
		virtual sce::Gnm::SizeAlign getBinarySizeAlign() const { return sce::Gnm::SizeAlign(mShaderInfo.m_gpuShaderCodeSize, sce::Gnm::kAlignmentOfShaderInBytes); }
		virtual const void *getHeaderPtr() const { return mHeaderPtr; }
		virtual const void *getBinaryPtr() const { return mBinaryPtr; }
		virtual const sce::Gnmx::InputOffsetsCache *getInputCache() const { return &mInputCache; }
	
	protected:
		sce::Gnmx::ShaderInfo mShaderInfo;
		const void *mHeaderPtr{ nullptr };
		const void *mBinaryPtr{ nullptr };
		sce::Gnmx::InputOffsetsCache mInputCache;
	};

	class VertexShaderView : public BaseShaderView
	{
		INTERNAL_OBJ_PTR(sce::Gnmx::VsShader);

	public:
		VertexShaderView(const U8 *pData) : BaseShaderView(pData) {}
		virtual void assignAddress(void *headerAddr, void *binaryAddr);
		virtual void assignAddress(void *headerAddr, void *binaryAddr, void *fetchAddr);

		virtual sce::Gnm::SizeAlign getHeaderSizeAlign() const;
		virtual sce::Gnm::SizeAlign getFetchShaderSizeAlign() const;

		inline U32 getModifier() const { return mModifier; }
		inline void *getFetchAddr() const { return mFetchAddr; }
	protected:
		U32 mModifier{ 0 };
		void *mFetchAddr{ nullptr };
	};

	class PixelShaderView : public BaseShaderView
	{
		INTERNAL_OBJ_PTR(sce::Gnmx::PsShader);

	public:
		PixelShaderView(const U8 *pData) : BaseShaderView(pData) {}
		virtual void assignAddress(void *headerAddr, void *binaryAddr);

		virtual sce::Gnm::SizeAlign getHeaderSizeAlign() const;
	};

	class ComputeShaderView : public BaseShaderView
	{
		INTERNAL_OBJ_PTR(sce::Gnmx::CsShader);

	public:
		ComputeShaderView(const U8 *pData) : BaseShaderView(pData) {}
		virtual void assignAddress(void *headerAddr, void *binaryAddr);

		virtual sce::Gnm::SizeAlign getHeaderSizeAlign() const;
	};
	// TODO more shader type

	// Texture
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
			sce::Gnm::TileMode mTileMode{ sce::Gnm::kTileModeThin_2dThin };
		};

		TextureView(const Description &desc);
		void assignAddress(void *baseAddr);

		inline sce::Gnm::SizeAlign getSizeAlign() const { return mObject.getSizeAlign(); }
	};

	// Targets
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
			sce::Gnm::TileMode			mTileMode{ sce::Gnm::kTileModeThin_2dThin };
			bool						mIsDisplayable{ false };
			bool						mUseCMask{ false };
			bool						mUseFMask{ false };
			bool						mUseHTile{ false };
			bool						mUseStencil{ false };
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

	class BufferView : public GPUResourceView
	{
		INTERNAL_OBJ(sce::Gnm::Buffer);
	};

	class VertexBufferView : public BufferView
	{

	};

	class DataBufferView : public BufferView
	{

	};

	class ConstantBufferView : public BufferView
	{

	};
}
