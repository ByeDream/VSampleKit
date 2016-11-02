#pragma once

namespace Framework
{
	class GraphicDevice;
	class RenderSet;
	class Allocators;

	class Swapchain
	{
	public:
		enum
		{
			MAX_NUMBER_OF_SWAPPED_BUFFERS = 8,
		};

		Swapchain(GraphicDevice *device);
		~Swapchain();

		void						init(Allocators *allocators);
		void						deinit(Allocators *allocators);

		void						flip();

	private:
		void						initSwappedBuffers();

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

		void *						mSwappedBuffers[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		RenderSet *					mSwapchainRenderSets[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		U32							mNumSwappedBuffers{ 0 };
		U32							mBackBufferIndex{ 0 };
		U32							mFrontBufferIndex{ 0 };
		RenderSet *					mBackBuffer{ nullptr };

		// For frame synchronization
		volatile U64 *				mFrameLabelPool{ nullptr };		// Labels for frame sync with GPU
		volatile U64 *				mFlippingLabelPool{ nullptr };	// Labels for frame flipping sync with GPU (only needed when GPU handles the flips )
		sce::Gnm::ResourceHandle	mLabelHandle{ sce::Gnm::kInvalidResourceHandle };
		sce::Gnm::ResourceHandle	mLabelForPrepareFlipHandle{ sce::Gnm::kInvalidResourceHandle };
		U64							mExpectedLabel[MAX_NUMBER_OF_SWAPPED_BUFFERS];
	};
}
