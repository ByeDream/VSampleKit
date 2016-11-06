#include "stdafx.h"

#include "RenderSurfaceManager.h"
#include "Texture.h"

using namespace sce;

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::createSurface(RenderSurface **out_surface, const RenderSurface::Description *desc, const TextureSourcePixelData *srcData /*= nullptr*/)
{
	// TODO, mark the status for Surface, first search the one can be reused when create a surface
	RenderSurface::Handle _handle = RenderSurface::RENDER_SURFACE_HANDLE_INVALID;
	if (out_surface != nullptr && desc != nullptr)
	{
		_handle = genSurfaceHandle();
		SCE_GNM_ASSERT(mAllocators != nullptr);
		SCE_GNM_ASSERT(getSurface(_handle) == nullptr);

		RenderSurface *_surface = new RenderSurface;
		_surface->init(*desc, mAllocators, srcData);
		_surface->setHandle(_handle);
		mSurfaceTable[_handle] = _surface;
		*out_surface = _surface;
	}
	return _handle;
}

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::createSurface(RenderSurface **out_surface, U32 width, U32 height, U32 depth, U32 mipLevels, bool enableCMask, bool enableFMask, bool enableHTile, bool enableStencil, bool isDynamicDisplayableColorTarget, sce::Gnm::DataFormat format, AntiAliasingType AAType, sce::GpuAddress::SurfaceType type, const char *name, const TextureSourcePixelData *srcData /*= nullptr*/)
{
	RenderSurface::Description _desc;
	_desc.mWidth								= width;
	_desc.mHeight								= height;
	_desc.mDepth								= depth;
	_desc.mMipLevels							= mipLevels;
	_desc.mEnableCMask							= enableCMask;
	_desc.mEnableFMask							= enableFMask;
	_desc.mEnableHTile							= enableHTile;
	_desc.mEnableStencil						= enableStencil;
	_desc.mIsDynamicDisplayableColorTarget		= isDynamicDisplayableColorTarget;
	_desc.mFormat								= format;
	_desc.mAAType								= AAType;
	_desc.mType									= type;
	_desc.mName									= name;
	return createSurface(out_surface, &_desc, srcData);
}

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::createSurfaceFromFile(RenderSurface **out_surface, const char *filePath, RenderSurface::Description *desc /*= nullptr*/)
{
	FileIO _file(filePath);
	_file.load();

	RenderSurface::Description _defaultDesc;
	_defaultDesc.mType		= GpuAddress::kSurfaceTypeTextureFlat;
	_defaultDesc.mAAType	= AA_NONE; // TODO AA for texture

	RenderSurface::Description *_desc = (desc != nullptr) ? desc : &_defaultDesc;

	TextureSourcePixelData _srcData;
	parseSurface(_file.getBuffer(), _desc, &_srcData);
	SCE_GNM_ASSERT(_srcData.mDataPtr != nullptr);
	SCE_GNM_ASSERT(_srcData.mOffsetSolver != nullptr);

	_desc->mName = _file.getName();
	return createSurface(out_surface, _desc, &_srcData);
}

void Framework::RenderSurfaceManager::releaseSurface(RenderSurface::Handle handle)
{
	if (handle != RenderSurface::RENDER_SURFACE_HANDLE_INVALID)
	{
		auto itor = mSurfaceTable.find(handle);
		if (itor != mSurfaceTable.end())
		{
			// TODO, mark the status for Surface as free
			// itor->second;
		}
	}
}

void Framework::RenderSurfaceManager::destorySurface(RenderSurface::Handle handle)
{
	if (handle != RenderSurface::RENDER_SURFACE_HANDLE_INVALID)
	{
		auto itor = mSurfaceTable.find(handle);
		if (itor != mSurfaceTable.end())
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
			mSurfaceTable.erase(itor);
		}
	}
}

Framework::RenderSurface * Framework::RenderSurfaceManager::getSurface(RenderSurface::Handle handle) const
{
	auto itor = mSurfaceTable.find(handle);
	return (itor != mSurfaceTable.end()) ? itor->second : nullptr;
}

void Framework::RenderSurfaceManager::saveSurfaceToFile(const char *filePath, RenderSurface::Handle handle)
{
	SCE_GNM_ASSERT(filePath != nullptr);
	RenderSurface *_surface = getSurface(handle);
	SCE_GNM_ASSERT(_surface != nullptr);
	// TODO
}

Framework::RenderSurfaceManager::RenderSurfaceManager()
{

}

Framework::RenderSurfaceManager::~RenderSurfaceManager()
{
	for (auto itor = mSurfaceTable.begin(); itor != mSurfaceTable.end(); itor++)
	{
		if (itor->second)
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
		}
	}
	mSurfaceTable.clear();
}

namespace
{
	Framework::U32 TGAOffsetSolver(const Framework::Texture::Description &desc, Framework::U32 mipLevel, Framework::U32 arraySlice)
	{
		SCE_GNM_ASSERT_MSG(mipLevel == 0, "TGA is a quite simple format doesn't sopport it");
		SCE_GNM_ASSERT_MSG(arraySlice == 0, "TGA is a quite simple format doesn't sopport it");

		return 0;
	}
}

void Framework::RenderSurfaceManager::parseSurface(const U8 *fileBuffer, RenderSurface::Description *out_desc, TextureSourcePixelData *out_srcData)
{
	SCE_GNM_ASSERT(fileBuffer != nullptr && out_desc != nullptr && out_srcData != nullptr);

	// TODO support other format, only support TGA at the moment.
	{
		// Simple TGA file
		out_desc->mWidth				= (U32)tgaGetWidth(fileBuffer);
		out_desc->mHeight				= (U32)tgaGetHeight(fileBuffer);
		out_desc->mDepth				= 1;
		out_desc->mMipLevels			= 1;
		out_desc->mFormat				= Gnm::kDataFormatR8G8B8A8Unorm;

		out_srcData->mDataPtr			= (const U8 *)tgaRead(fileBuffer, TGA_READER_ABGR);
		out_srcData->mOffsetSolver		= TGAOffsetSolver;
	}
}

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::genSurfaceHandle()
{
	static RenderSurface::Handle _handle = RenderSurface::RENDER_SURFACE_HANDLE_INVALID;
	_handle++;
	return _handle;
}

