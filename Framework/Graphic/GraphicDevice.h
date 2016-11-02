#pragma once

#include "RenderSurface.h"
#include <vector>

namespace Framework
{
	class Application;
	class Allocators;
	class StackAllocator;
	class OutputDevice;
	class SwapChain;
	class RenderSet;
	class RenderContext;

	class GraphicDevice
	{
	public:
		GraphicDevice(Application *app);
		~GraphicDevice();

		void								init();
		void								deinit();

		void								allocRenderSet(	RenderSet *renderSet,
															const RenderSurface::Description *depth,
															const RenderSurface::Description *color0,
															const RenderSurface::Description *color1 = nullptr,
															const RenderSurface::Description *color2 = nullptr,
															const RenderSurface::Description *color3 = nullptr);
		void								releaseRenderSet(RenderSet *renderSet);

		inline Application *				getApplication() const { return mApp; }

		inline const OutputDevice *			getOutput() const { return mOutput; }
		inline OutputDevice *				getOutput() { return mOutput; }
		inline const SwapChain *			getSwapChain() const { return mSwapChain; }
		inline SwapChain *					getSwapChain() { return mSwapChain; }

		inline const RenderContext *		getImmediateContext() const { return mContexts[0]; }
		inline RenderContext *				getImmediateContext() { return mContexts[0]; }
		inline const RenderContext *		getDeferredContext(U32 index) const
		{
			SCE_GNM_ASSERT(index > 0 && index < mContexts.size());
			return mContexts[index];
		}
		inline RenderContext *				getDeferredContext(U32 index)
		{
			SCE_GNM_ASSERT(index > 0 && index < mContexts.size());
			return mContexts[index];
		}

		void								rollImmediateContext();
		void								rollDeferreContext();
	private:
		void								initMem();
		void								deinitMem();
		void								initOutputAndSwapChain();
		void								deinitOutputAndSwapChain();
		void								initContexts();
		void								deinitContexts();

	private:
		Application *						mApp{ nullptr };

		// graphic memory allocators interface
		Allocators *						mAllocators{ nullptr };
		// use stack allocator at the moment
		StackAllocator *					mGarlicAllocator{ nullptr };
		StackAllocator *					mOnionAllocator{ nullptr };

		OutputDevice *						mOutput{ nullptr };
		SwapChain *							mSwapChain{ nullptr };

		std::vector<RenderContext *>		mContexts;
	};
}