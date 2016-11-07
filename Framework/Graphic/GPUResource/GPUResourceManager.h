#pragma once

#include "GPUResource.h"
#include <map>
#include <vector>

namespace Framework
{
	class Allocators;
	class GPUResourceManager : public Singleton<GPUResourceManager>
	{
		friend class Singleton<GPUResourceManager>;

	public:
		void							init(Allocators *allocators, U32 poolSize);
		void							deinit(Allocators *allocators);
		void							flip();

		GPUResourceHandle				createResource(GPUResourceType type, BaseGPUResource **out_resource, const BaseGPUResource::Description *desc);
		GPUResourceHandle				createResourceFromFile(GPUResourceType type, BaseGPUResource **out_resource, const char *filePath, BaseGPUResource::Description *desc = nullptr);
		void							saveResourceToFile(const char *filePath, GPUResourceHandle handle);

		void							releaseResource(GPUResourceHandle handle);
		void							destoryResource(GPUResourceHandle handle);

		BaseGPUResource *				getResource(GPUResourceHandle handle) const;

	private:
		GPUResourceManager();
		virtual ~GPUResourceManager();

		GPUResourceHandle				genResourceHandle(GPUResourceType type);
		void							clearPool(U32 index);

		// For surface only
		void							parseSurface(const U8 *fileBuffer, BaseGPUResource::Description *out_desc);

	private:
		std::map<GPUResourceHandle, BaseGPUResource *> mResourceTable;
		Allocators *					mAllocators{ nullptr };

		typedef std::vector<BaseGPUResource *>	ToBeDestoryPool;
		ToBeDestoryPool *				mPool{ nullptr };
		U32								mPoolSize{ 0 };
		U32								mPoolIndex{ 0 };
	};
}