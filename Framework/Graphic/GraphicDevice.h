#pragma once

#include "RenderSurface.h"

namespace Framework
{
	class Application;
	class Allocators;
	class StackAllocator;
	class OutputDevice;
	class Swapchain;
	class RenderSet;

	class GraphicDevice
	{
	public:
		GraphicDevice(Application *app);
		~GraphicDevice();

		void								init();
		void								deinit();

		void								createRenderSet(RenderSet *out_renderSet,
															const RenderSurface::Description *depth,
															const RenderSurface::Description *color0,
															const RenderSurface::Description *color1 = nullptr,
															const RenderSurface::Description *color2 = nullptr,
															const RenderSurface::Description *color3 = nullptr);

		inline Application *				getApplication() const { return mApp; }

		inline const OutputDevice *			getOutput() const { return mOutput; }
		inline OutputDevice *				getOutput() { return mOutput; }
	private:
		void								initMem();
		void								deinitMem();

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