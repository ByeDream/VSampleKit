#pragma once

namespace Framework
{
	class GraphicDevice;
	class RenderSet;
	class Allocators;
	class EopEventQueue;

	class SwapChain
	{
	public:
		enum
		{
			MAX_NUMBER_OF_SWAPPED_BUFFERS = 8,
		};

		SwapChain(GraphicDevice *device);
		~SwapChain();

		void						init(Allocators *allocators);
		void						deinit(Allocators *allocators);

		inline RenderSet *			getSwapChianRenderSet() const { return mCurrentBuffer; }
		void						flip();

		inline SceVideoOutFlipMode	getFilpMode() const { return mFilpMode; }
		inline U32					getCurrentBufferIndex() const { return mCurrentBufferIndex; }
		inline U32					getNextBufferIndex() const { return mNextBufferIndex; }
		inline U64					getFrameCount() const { return mFrameCounter; }

	private:
		void						initSwappedBuffers();
		void						advance();
		void						stallUntilGPUIsNotUsingBuffer(EopEventQueue *eopEventQueue, U32 bufferIndex);

	private:
		GraphicDevice *				mDevice{ nullptr };
		U32							mTargetWidth{ 0 };
		U32							mTargetHeight{ 0 };
		bool						mUseDepth{ false };
		bool						mUseStencil{ false };
		AntiAliasingType			mColorAAType{ AA_NONE };
		AntiAliasingType			mDepthAAType{ AA_NONE };
		bool						mUseDynamicBuffers{ false };
		sce::Gnm::DataFormat		mColorFormat{ sce::Gnm::kDataFormatInvalid };
		sce::Gnm::ZFormat			mDepthFormat{ sce::Gnm::kZFormatInvalid };
		SceVideoOutFlipMode			mFilpMode{ SCE_VIDEO_OUT_FLIP_MODE_VSYNC };
		bool						mAsynchronous{ true };		// using asynchronous way between CPU & GPU

		void *						mSwappedBuffers[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		RenderSet *					mSwapChainRenderSets[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		U32							mNumSwappedBuffers{ 0 };
		U32							mCurrentBufferIndex{ 0 };	// Corresponding to back buffer in the double buffering, the current of buffer CPU is writing / drawing
		U32							mNextBufferIndex{ 0 };		// Corresponding to front buffer in the double buffering, the last of buffer GPU wrote
		RenderSet *					mCurrentBuffer{ nullptr };

		// For frame synchronization
		volatile U64 *				mFrameLabelPool{ nullptr };		// Labels for frame sync with GPU
		volatile U64 *				mFlippingLabelPool{ nullptr };	// Labels for frame flipping sync with GPU (only needed when GPU handles the flips )
		sce::Gnm::ResourceHandle	mLabelHandle{ sce::Gnm::kInvalidResourceHandle };
		sce::Gnm::ResourceHandle	mLabelForPrepareFlipHandle{ sce::Gnm::kInvalidResourceHandle };
		U64							mExpectedLabel[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		EopEventQueue *				mEopEventQueue;

		U64							mFrameCounter{ 0 };
	};
}
