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

		struct Description
		{
			U32								mNumSwappedBuffers{ 3 };
			bool							mUseDepth{ false };
			bool							mUseStencil{ false };
			sce::Gnm::DataFormat			mColorFormat{ sce::Gnm::kDataFormatB8G8R8A8UnormSrgb };
			sce::Gnm::ZFormat				mDepthFormat{ sce::Gnm::kZFormat32Float };
			AntiAliasingType				mColorAAType{ AA_NONE };
			AntiAliasingType				mDepthAAType{ AA_NONE };
			SceVideoOutFlipMode				mFilpMode{ SCE_VIDEO_OUT_FLIP_MODE_VSYNC };
			bool							mIsDynamic{ false };
			bool							mAsynchronous{ true };
		};

		SwapChain(GraphicDevice *device);
		~SwapChain();

		void								init(const Description& desc, Allocators *allocators);
		void								deinit(Allocators *allocators);
		void								flip();

		inline const RenderSet *			getSwapChianRenderSet() const { return mCurrentBuffer; }
		inline RenderSet *					getSwapChianRenderSet() { return mCurrentBuffer; }

		inline U32							getCurrentBufferIndex() const { return mCurrentBufferIndex; }
		inline U32							getNextBufferIndex() const { return mNextBufferIndex; }
		inline U64							getFrameCount() const { return mFrameCounter; }

		inline const Description &			getDescription() const { return mDesc; }

	private:
		void								initSwappedBuffers();
		void								deinitSwappedBuffers();
		void								submitFrame();
		void								synchronizeFrameToGPU();
		void								prepareFrame();
		void								advance();
		void								stallUntilGPUIsNotUsingBuffer(EopEventQueue *eopEventQueue, U32 bufferIndex);

	private:
		GraphicDevice *						mDevice{ nullptr };
		Description							mDesc;
		U32									mTargetWidth{ 0 };
		U32									mTargetHeight{ 0 };

		void *								mSwappedBuffers[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		RenderSet *							mSwapChainRenderSets[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		U32									mCurrentBufferIndex{ 0 };	// Corresponding to back buffer in the double buffering, the current of buffer CPU is writing / drawing
		U32									mNextBufferIndex{ 0 };		// Corresponding to front buffer in the double buffering, the last of buffer GPU wrote
		RenderSet *							mCurrentBuffer{ nullptr };

		// For frame synchronization
		volatile U64 *						mFrameLabelPool{ nullptr };		// Labels for frame sync with GPU
		volatile U64 *						mFlippingLabelPool{ nullptr };	// Labels for frame flipping sync with GPU (only needed when GPU handles the flips )
		sce::Gnm::ResourceHandle			mFrameLabellHandle{ sce::Gnm::kInvalidResourceHandle };
		sce::Gnm::ResourceHandle			mFlippingLabelHandle{ sce::Gnm::kInvalidResourceHandle };
		U64									mExpectedLabel[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		EopEventQueue *						mEopEventQueue;

		U64									mFrameCounter{ 0 };
	};
}
