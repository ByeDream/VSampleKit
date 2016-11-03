#pragma once

namespace Framework
{
	class GraphicDevice;
	class RenderSurface;

	class RenderContext
	{
	public:
		RenderContext(GraphicDevice *device);
		virtual ~RenderContext();

		virtual void roll();
		virtual void reset();
		virtual void submitAndFlip(bool asynchronous);
		virtual void appendLabelAtEOPWithInterrupt(void *dstGpuAddr, U64 value);	// writes value and and triggers an interrupt
		virtual void appendLabelAtEOP(void *dstGpuAddr, U64 value);					// writes value only


		void setTextureSurface(U32 soltID, const RenderSurface *surface);
		void setRenderTargetSurface(U32 soltID, const RenderSurface *surface);
		void setDepthStencilTargetSurface(const RenderSurface *surface);

		void setViewport(U32 x, U32 y, U32 width, U32 height, Float32 minz = 0.0f, Float32 maxz = 1.0f);

	protected:
		GraphicDevice *					mDevice{ nullptr };

	};
}