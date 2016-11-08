#pragma once

#include "GPUResource/RenderSurface.h"

namespace Framework
{
	class RenderContext;

	class RenderSet
	{
	public:
		RenderSet();
		virtual ~RenderSet();
		
		virtual void					init(RenderSurface *depth, RenderSurface *color0, RenderSurface *color1 = nullptr, RenderSurface *color2 = nullptr, RenderSurface *color3 = nullptr);
		virtual void					setColorSurface(U32 slotID, RenderSurface *surface);
		virtual void					setColorSurface(U32 slotID, GPUResourceHandle surfaceHandle);
		virtual void					setDepthSurface(RenderSurface *surface);
		virtual void					setDepthSurface(GPUResourceHandle surfaceHandle);

		virtual void					bindToPipeline(RenderContext *context) const;

		inline const RenderSurface *	getColorSurface(U32 slotID) const { SCE_GNM_ASSERT(slotID >= 0 && slotID < MAX_NUM_RENDER_TARGETS); return mColorSurfaces[slotID]; }
		inline const RenderSurface *	getDepthSurface() const { return mDepthSurface; }
		inline RenderSurface *			getColorSurface(U32 slotID) { SCE_GNM_ASSERT(slotID >= 0 && slotID < MAX_NUM_RENDER_TARGETS); return mColorSurfaces[slotID]; }
		inline RenderSurface *			getDepthSurface() { return mDepthSurface; }
		inline GPUResourceHandle		getColorSurfaceHandle(U32 slotID) const { SCE_GNM_ASSERT(slotID >= 0 && slotID < MAX_NUM_RENDER_TARGETS); return mColorHandle[slotID]; }
		inline GPUResourceHandle		getDepthSurfaceHandle() const { return mDepthHandle; }
		inline U32						getNumColorSurfaces() const { return mNumColorSurfaces; }

	protected:
		RenderSurface *					mDepthSurface{ nullptr };
		RenderSurface *					mColorSurfaces[MAX_NUM_RENDER_TARGETS]{ nullptr, nullptr, nullptr, nullptr };
		GPUResourceHandle				mDepthHandle{ RESOURCE_HANDLE_INVALID };
		GPUResourceHandle				mColorHandle[MAX_NUM_RENDER_TARGETS]{ RESOURCE_HANDLE_INVALID, RESOURCE_HANDLE_INVALID, RESOURCE_HANDLE_INVALID, RESOURCE_HANDLE_INVALID };
		U32								mNumColorSurfaces{ 0 };
	};
}