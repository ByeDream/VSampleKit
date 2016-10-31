#pragma once

namespace Framework
{
	class RenderSurface;

	class RenderContext
	{
	public:
		RenderContext();
		virtual ~RenderContext();

		void setTextureSurface(U32 soltID, const RenderSurface *surface);
		void setRenderTargetSurface(U32 soltID, const RenderSurface *surface);
		void setDepthStencilTargetSurface(const RenderSurface *surface);

		void setViewport(U32 x, U32 y, U32 width, U32 height, Float32 minz = 0.0f, Float32 maxz = 1.0f);
	};
}