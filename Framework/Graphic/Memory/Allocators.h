#pragma once

#include "IAllocator.h"

namespace Framework
{
	class Allocators
	{
		static IAllocator getNullIAllocator();
	public:
		Allocators(IAllocator onion = getNullIAllocator(), IAllocator garlic = getNullIAllocator(), sce::Gnm::OwnerHandle owner = sce::Gnm::kInvalidOwnerHandle);

		IAllocator mOnion;
		IAllocator mGarlic;
		sce::Gnm::OwnerHandle mOwner;
		uint8_t mReserved[4];

		S32 allocate(void **out_memory, SceKernelMemoryType sceKernelMemoryType, U32 size, U32 alignment, sce::Gnm::ResourceType resourceType = sce::Gnm::kResourceTypeInvalid, sce::Gnm::ResourceHandle *out_resourceHandle = nullptr, const char *name = nullptr, ...);
		S32 allocate(void **out_memory, SceKernelMemoryType sceKernelMemoryType, sce::Gnm::SizeAlign sizeAlign, sce::Gnm::ResourceType resourceType = sce::Gnm::kResourceTypeInvalid, sce::Gnm::ResourceHandle *out_resourceHandle = nullptr, const char *name = nullptr, ...);
		S32 release(void *memory, SceKernelMemoryType sceKernelMemoryType, sce::Gnm::ResourceHandle *resourceHandle = nullptr);
	};
}
