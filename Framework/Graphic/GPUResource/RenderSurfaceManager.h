#pragma once

#include "RenderSurface.h"
#include <map>

namespace Framework
{
	class Allocators;
	struct TextureSourcePixelData;

	class RenderSurfaceManager : public Singleton<RenderSurfaceManager>
	{
		friend class Singleton<RenderSurfaceManager>;

	public:
		inline void						setAllocator(Allocators *allocators) { mAllocators = allocators; }

		RenderSurface::Handle			createSurface(RenderSurface **out_surface, const RenderSurface::Description *desc, const TextureSourcePixelData *srcData = nullptr);
		RenderSurface::Handle			createSurface(	RenderSurface **out_surface,
														U32 width, U32 height, U32 depth,
														U32 mipLevels,
														bool enableCMask, bool enableFMask, bool enableHTile, bool enableStencil,
														bool isDynamicDisplayableColorTarget,
														sce::Gnm::DataFormat format,
														AntiAliasingType AAType,
														sce::GpuAddress::SurfaceType type,
														const char *name,
														const TextureSourcePixelData *srcData = nullptr);

		RenderSurface::Handle			createSurfaceFromFile(RenderSurface **out_surface, const char *filePath, RenderSurface::Description *desc = nullptr);
		void							saveSurfaceToFile(const char *filePath, RenderSurface::Handle handle);

		void							releaseSurface(RenderSurface::Handle handle);
		void							destorySurface(RenderSurface::Handle handle);

		RenderSurface *					getSurface(RenderSurface::Handle handle) const;

	private:
		RenderSurfaceManager();
		virtual ~RenderSurfaceManager();

		void							parseSurface(const U8 *fileBuffer, RenderSurface::Description *out_desc, TextureSourcePixelData *out_srcData);
		RenderSurface::Handle			genSurfaceHandle();

	private:
		std::map<RenderSurface::Handle, RenderSurface *> mSurfaceTable;
		Allocators *					mAllocators{ nullptr };
	};
}
