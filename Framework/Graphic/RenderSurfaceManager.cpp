#include "stdafx.h"

#include "RenderSurfaceManager.h"

using namespace sce;

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::createSurface(RenderSurface **out_surface, const RenderSurface::Description &desc, const U8 *pData /*= nullptr*/)
{
	SCE_GNM_ASSERT(mAllocators != nullptr);
	RenderSurface *_surface = new RenderSurface;
	_surface->init(desc, mAllocators, pData);
	RenderSurface::Handle _handle = genSurfaceHandle();
	_surface->setHandle(_handle);
	SCE_GNM_ASSERT(getSurface(_handle) == nullptr);
	mSurfaceTable[_handle] = _surface;
	*out_surface = _surface;
	return _handle;
}

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::createSurface(RenderSurface **out_surface, U32 width, U32 height, U32 depth, U32 mipLevels, bool enableCMask, bool enableFMask, bool enableHTile, bool enableStencil, sce::Gnm::DataFormat format, AntiAliasingType AAType, sce::GpuAddress::SurfaceType type, const char *name, const U8 *pData /*= nullptr*/)
{
	RenderSurface::Description _desc;
	_desc.mWidth			= width;
	_desc.mHeight			= height;
	_desc.mDepth			= depth;
	_desc.mMipLevels		= mipLevels;
	_desc.mEnableCMask		= enableCMask;
	_desc.mEnableFMask		= enableFMask;
	_desc.mEnableHTile		= enableHTile;
	_desc.mEnableStencil	= enableStencil;
	_desc.mFormat			= format;
	_desc.mAAType			= AAType;
	_desc.mType				= type;
	_desc.mName				= name;
	return createSurface(out_surface, _desc, pData);
}

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::createSurfaceFromFile(RenderSurface **out_surface, const char *filePath, RenderSurface::Description *desc /*= nullptr*/)
{
	FileIO _file(filePath);
	_file.load();

	RenderSurface::Description _defaultDesc;
	_defaultDesc.mType		= GpuAddress::kSurfaceTypeTextureFlat;
	_defaultDesc.mAAType	= AA_NONE; // TODO

	RenderSurface::Description *_desc = (desc != nullptr) ? desc : &_defaultDesc;
	U8 *_pixelData = nullptr;

	parseSurface(_file.getBuffer(), _desc, &_pixelData);
	SCE_GNM_ASSERT(_pixelData != nullptr);
	_desc->mName = _file.getName();
	return createSurface(out_surface, *_desc, _pixelData);
}

void Framework::RenderSurfaceManager::destorySurface(RenderSurface::Handle handle)
{
	auto itor = mSurfaceTable.find(handle);
	if (itor != mSurfaceTable.end())
	{
		itor->second->deinit(mAllocators);
		SAFE_DELETE(itor->second);
		mSurfaceTable.erase(itor);
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

void Framework::RenderSurfaceManager::parseSurface(const U8 *fileBuffer, RenderSurface::Description *out_desc, U8 **out_pixelData)
{
	SCE_GNM_ASSERT(fileBuffer != nullptr && out_desc != nullptr && out_pixelData != nullptr);

	// TODO support other format, only support TGA at the moment.
	out_desc->mWidth			= (U32)tgaGetWidth(fileBuffer);
	out_desc->mHeight			= (U32)tgaGetHeight(fileBuffer);
	*out_pixelData				= (U8 *)tgaRead(fileBuffer, TGA_READER_ABGR);
}

Framework::RenderSurface::Handle Framework::RenderSurfaceManager::genSurfaceHandle()
{
	static RenderSurface::Handle _handle = RenderSurface::kInvalidRenderSurfaceHandle;
	_handle++;
	return _handle;
}

