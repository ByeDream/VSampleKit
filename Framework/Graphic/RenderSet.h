#pragma once

namespace Framework
{
	class RenderSurface;
	class RenderContext;

	class RenderSet
	{
	public:
		enum
		{
			MAX_NUM_COLOR_SURFACE = 4,
		};

		RenderSet();
		virtual ~RenderSet();
		
		virtual void				init(RenderSurface *depth, RenderSurface *color0, RenderSurface *color1 = nullptr, RenderSurface *color2 = nullptr, RenderSurface *color3 = nullptr);
		virtual void				setColorSurface(U32 slotID, RenderSurface *surface);
		virtual void				setDepthSurface(RenderSurface *surface);

		virtual void				bindToPipeline(RenderContext *context) const;

		inline const RenderSurface *getColorSurface(U32 slotID) const { SCE_GNM_ASSERT(slotID >= 0 && slotID < MAX_NUM_COLOR_SURFACE); return mColorSurfaces[slotID]; }
		inline const RenderSurface *getDepthSurface() const { return mDepthSurface; }
		inline RenderSurface *		getColorSurface(U32 slotID) { SCE_GNM_ASSERT(slotID >= 0 && slotID < MAX_NUM_COLOR_SURFACE); return mColorSurfaces[slotID]; }
		inline RenderSurface *		getDepthSurface() { return mDepthSurface; }
		inline U32					getNumColorSurfaces() const { return mNumColorSurfaces; }

	protected:
		RenderSurface *				mDepthSurface{ nullptr };
		RenderSurface *				mColorSurfaces[MAX_NUM_COLOR_SURFACE]{ nullptr, nullptr, nullptr, nullptr };
		U32							mNumColorSurfaces{ 0 };
	};
}