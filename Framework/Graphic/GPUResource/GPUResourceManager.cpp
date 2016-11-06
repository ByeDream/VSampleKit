#include "stdafx.h"

#include "GPUResourceManager.h"
#include "RenderSurface.h"
#include "Shader.h"

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

Framework::GPUResourceHandle Framework::GPUResourceManager::createResourceFromFile(GPUResourceType type, BaseGPUResource **out_resource, const char *filePath, BaseGPUResource::Description *desc)
{
	FileIO _file(filePath);
	_file.load();

	switch (type)
	{
	case Framework::RESOURCE_TYPE_SURFACE:
		break;
	case Framework::RESOURCE_TYPE_SHADER:
		{
		Shader::Description *_desc = typeCast<BaseGPUResource::Description, Shader::Description>(desc);
		_desc->mDataPtr = _file.getBuffer();
		_desc->mName = _file.getName();
		}
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}

	return createResource(type, out_resource, desc);
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
	// TODO get fence and delay delete
	if (handle != RESOURCE_HANDLE_INVALID)
	{
		auto itor = mResourceTable.find(handle);
		if (itor != mResourceTable.end())
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
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
	for (auto itor = mResourceTable.begin(); itor != mResourceTable.end(); itor++)
	{
		if (itor->second)
		{
			itor->second->deinit(mAllocators);
			SAFE_DELETE(itor->second);
		}
	}
	mResourceTable.clear();
}

Framework::GPUResourceHandle Framework::GPUResourceManager::genResourceHandle(GPUResourceType type)
{
	static GPUResourceHandle _sSurfaceHandle = SURFACE_HANDLE_START;
	static GPUResourceHandle _sShaderHandle = SHADER_HANDLE_START;
	GPUResourceHandle ret;
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
