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
		AntiAliasingType			mColorAAType{ AA_NONE };
		AntiAliasingType			mDepthAAType{ AA_NONE };

		RenderSet *					mSwappedBuffers[MAX_NUMBER_OF_SWAPPED_BUFFERS];
		U32							mNumSwappedBuffers{ 0 };
		U32							mBackBufferIndex{ 0 };
		RenderSet *					mBackBuffer{ nullptr };
		RenderSet *					mFrontBuffer{ nullptr };

		// For frame synchronization
		volatile U64 *				mLabelPool{ nullptr };
		volatile U64 *				mLabelForPreparePool{ nullptr };
		sce::Gnm::ResourceHandle	mLabelHandle{ sce::Gnm::kInvalidResourceHandle };
		sce::Gnm::ResourceHandle	mLabelForPrepareFlipHandle{ sce::Gnm::kInvalidResourceHandle };
	};
}
