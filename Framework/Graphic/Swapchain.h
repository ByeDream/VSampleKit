#pragma once

namespace Framework
{
	class GraphicDevice;
	class RenderSurface;

	class Swapchain
	{
	public:
		enum
		{
			MAX_NUMBER_OF_SWAPPED_SURFACES = 8,
		};

		Swapchain(GraphicDevice *device);
		~Swapchain();

		void init();
		void deinit();

		void flip();

	private:
		GraphicDevice *		mDevice;
		RenderSurface *		mSwappedSurfaces[MAX_NUMBER_OF_SWAPPED_SURFACES];
	};
}
