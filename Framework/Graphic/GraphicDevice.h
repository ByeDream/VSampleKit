#pragma once

namespace Framework
{
	class Application;
	class Allocators;
	class StackAllocator;
	class OutputDevice;
	class Swapchain;

	class GraphicDevice
	{
	public:
		GraphicDevice(Application *app);
		~GraphicDevice();

		void init();
		void deinit();

		inline const OutputDevice *getOutput() const { return mOutput; }
	private:
		void initMem();
		void deinitMem();

	private:
		Application *						mApp{ nullptr };

		// graphic memory allocators interface
		Allocators *						mAllocators{ nullptr };
		// use stack allocator at the moment
		StackAllocator *					mGarlicAllocator{ nullptr };
		StackAllocator *					mOnionAllocator{ nullptr };

		OutputDevice *						mOutput{ nullptr };
		Swapchain *							mSwapchain{ nullptr };
	};
}