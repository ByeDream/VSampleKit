#pragma once

#include "GPUResource.h"
#include <map>

namespace Framework
{
	class GPUResourceManager : public Singleton<GPUResourceManager>
	{
		friend class Singleton<GPUResourceManager>;

	public:
		inline void						setAllocator(Allocators *allocators) { mAllocators = allocators; }

		GPUResourceHandle				createResource(GPUResourceType type, BaseGPUResource **out_resource, const BaseGPUResource::Description *desc);
		GPUResourceHandle				createResourceFromFile(GPUResourceType type, BaseGPUResource **out_resource, const char *filePath, BaseGPUResource::Description *desc);
		void							saveResourceToFile(const char *filePath, GPUResourceHandle handle);

		void							releaseResource(GPUResourceHandle handle);
		void							destoryResource(GPUResourceHandle handle);

		BaseGPUResource *				getResource(GPUResourceHandle handle) const;

	private:
		GPUResourceManager();
		virtual ~GPUResourceManager();

		GPUResourceHandle				genResourceHandle(GPUResourceType type);

	private:
		std::map<GPUResourceHandle, BaseGPUResource *> mResourceTable;
		Allocators *					mAllocators{ nullptr };
	};
}