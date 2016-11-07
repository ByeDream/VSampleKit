#include "stdafx.h"

#include "GPUResourceManager.h"
#include "RenderSurface.h"
#include "Shader.h"
#include "Texture.h"

using namespace sce;

void Framework::GPUResourceManager::init(Allocators *allocators, U32 poolSize)
{
	mPoolSize = poolSize;
	mPool = new ToBeDestoryPool[mPoolSize];
	mPoolIndex = 0;
}

void Framework::GPUResourceManager::deinit(Allocators *allocators)
{
	for (auto itor = mResourceTable.begin(); itor != mResourceTable.end(); itor++)
	{
		if (itor->second)
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
		}
	}
	mResourceTable.clear();

	for (mPoolIndex = 0; mPoolIndex < mPoolSize; mPoolIndex++)
	{
		clearPool(mPoolIndex);
	}
	SAFE_DELETE_ARRAY(mPool);
}

void Framework::GPUResourceManager::flip()
{
	mPoolIndex = (mPoolIndex + 1) % mPoolSize;
	clearPool(mPoolIndex);
}

Framework::GPUResourceHandle Framework::GPUResourceManager::createResource(GPUResourceType type, BaseGPUResource **out_resource, const BaseGPUResource::Description *desc)
{
	GPUResourceHandle _handle = RESOURCE_HANDLE_INVALID;
	if (out_resource != nullptr && desc != nullptr)
	{
		_handle = genResourceHandle(type);
		SCE_GNM_ASSERT(mAllocators != nullptr);
		SCE_GNM_ASSERT(getResource(_handle) == nullptr);

		switch (type)
		{
		case Framework::RESOURCE_TYPE_SURFACE:
			// TODO, mark the status for Surface, first search the one can be reused when create a surface
			*out_resource = new RenderSurface;
			break;
		case Framework::RESOURCE_TYPE_SHADER:
			*out_resource = new Shader;
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}

		(*out_resource)->init(desc, mAllocators);
		(*out_resource)->setType(type);
		(*out_resource)->setHandle(_handle);
		mResourceTable[_handle] = *out_resource;
	}
	return _handle;
}

Framework::GPUResourceHandle Framework::GPUResourceManager::createResourceFromFile(GPUResourceType type, BaseGPUResource **out_resource, const char *filePath, BaseGPUResource::Description *desc /*= nullptr*/)
{
	FileIO _file(filePath);
	_file.load();

	GPUResourceHandle _handle = RESOURCE_HANDLE_INVALID;
	switch (type)
	{
	case Framework::RESOURCE_TYPE_SURFACE:
		{
			RenderSurface::Description _defaultDesc;
			_defaultDesc.mType = GpuAddress::kSurfaceTypeTextureFlat;
			_defaultDesc.mAAType = AA_NONE; // TODO AA for RenderSurface

			RenderSurface::Description *_desc = (desc != nullptr) ? typeCast<BaseGPUResource::Description, RenderSurface::Description>(desc) : &_defaultDesc;
			_desc->mSrcData = new TextureSourcePixelData;
			parseSurface(_file.getBuffer(), _desc);
			SCE_GNM_ASSERT(_desc->mSrcData->mDataPtr != nullptr);
			SCE_GNM_ASSERT(_desc->mSrcData->mOffsetSolver != nullptr);
			_handle = createResource(type, out_resource, _desc);
			SAFE_DELETE(_desc->mSrcData);
		}
		break;
	case Framework::RESOURCE_TYPE_SHADER:
		{
			SCE_GNM_ASSERT_MSG(desc != nullptr, "The description must be specified for shader type");
			Shader::Description *_desc = typeCast<BaseGPUResource::Description, Shader::Description>(desc);
			_desc->mDataPtr = _file.getBuffer();
			_desc->mName = _file.getName();
			_handle = createResource(type, out_resource, _desc);
		}
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}

	return _handle;
}

void Framework::GPUResourceManager::saveResourceToFile(const char *filePath, GPUResourceHandle handle)
{
	SCE_GNM_ASSERT(filePath != nullptr);
	BaseGPUResource *_resource = getResource(handle);
	SCE_GNM_ASSERT(_resource != nullptr);
	// TODO
}

void Framework::GPUResourceManager::releaseResource(GPUResourceHandle handle)
{
	if (handle != RESOURCE_HANDLE_INVALID)
	{
		auto itor = mResourceTable.find(handle);
		if (itor != mResourceTable.end())
		{
			// TODO
		}
	}
}

void Framework::GPUResourceManager::destoryResource(GPUResourceHandle handle)
{
	if (handle != RESOURCE_HANDLE_INVALID)
	{
		auto itor = mResourceTable.find(handle);
		if (itor != mResourceTable.end())
		{
			BaseGPUResource *_resource = itor->second;
			if (_resource->isBusy())
			{
				mPool[mPoolIndex].push_back(_resource);
			}
			else
			{
				_resource->deinit(mAllocators);
				SAFE_DELETE(_resource);
			}
			mResourceTable.erase(itor);
		}
	}

}

Framework::BaseGPUResource * Framework::GPUResourceManager::getResource(GPUResourceHandle handle) const
{
	auto itor = mResourceTable.find(handle);
	return (itor != mResourceTable.end()) ? itor->second : nullptr;
}

Framework::GPUResourceManager::GPUResourceManager()
{

}

Framework::GPUResourceManager::~GPUResourceManager()
{
	SCE_GNM_ASSERT_MSG(mPool == nullptr, "deinit me before destroy me");
}

Framework::GPUResourceHandle Framework::GPUResourceManager::genResourceHandle(GPUResourceType type)
{
	static GPUResourceHandle _sSurfaceHandle = SURFACE_HANDLE_START;
	static GPUResourceHandle _sShaderHandle = SHADER_HANDLE_START;
	GPUResourceHandle ret = RESOURCE_HANDLE_INVALID;
	switch (type)
	{
	case Framework::RESOURCE_TYPE_SURFACE:
		ret = _sSurfaceHandle++;
		break;
	case Framework::RESOURCE_TYPE_SHADER:
		ret = _sShaderHandle++;
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}
	return ret;
}

void Framework::GPUResourceManager::clearPool(U32 index)
{
	for (auto itor = mPool[index].begin(); itor != mPool[index].end(); itor++)
	{
		SAFE_DELETE(*itor);
	}
	mPool[index].clear();
}

// Texture Source Data Offset solvers --------------------------------------------
namespace
{
	Framework::U32 TGAOffsetSolver(const Framework::Texture::Description &desc, Framework::U32 mipLevel, Framework::U32 arraySlice)
	{
		SCE_GNM_ASSERT_MSG(mipLevel == 0, "TGA is a quite simple format doesn't sopport it");
		SCE_GNM_ASSERT_MSG(arraySlice == 0, "TGA is a quite simple format doesn't sopport it");
		return 0;
	}
}
//  -------------------------------------------------------------------------------

void Framework::GPUResourceManager::parseSurface(const U8 *fileBuffer, BaseGPUResource::Description *out_desc)
{
	RenderSurface::Description *_out_desc = typeCast<BaseGPUResource::Description, RenderSurface::Description>(out_desc);
	SCE_GNM_ASSERT(fileBuffer != nullptr && _out_desc != nullptr && _out_desc->mSrcData != nullptr);

	// TODO support other format, only support TGA at the moment.
	{
		// Simple TGA file
		_out_desc->mWidth = (U32)tgaGetWidth(fileBuffer);
		_out_desc->mHeight = (U32)tgaGetHeight(fileBuffer);
		_out_desc->mDepth = 1;
		_out_desc->mMipLevels = 1;
		_out_desc->mFormat = Gnm::kDataFormatR8G8B8A8Unorm;

		_out_desc->mSrcData->mDataPtr = (const U8 *)tgaRead(fileBuffer, TGA_READER_ABGR);
		_out_desc->mSrcData->mOffsetSolver = TGAOffsetSolver;
	}
}
