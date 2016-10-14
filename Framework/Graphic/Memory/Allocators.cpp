#include "stdafx.h"

#include "Allocators.h"

using namespace sce;

Framework::IAllocator Framework::Allocators::getNullIAllocator()
{
	IAllocator result = { 0, 0, 0 }; return result;
}

Framework::Allocators::Allocators(IAllocator onion /*= getNullIAllocator()*/, IAllocator garlic /*= getNullIAllocator()*/, sce::Gnm::OwnerHandle owner /*= sce::Gnm::kInvalidOwnerHandle*/)
	: mOnion(onion)
	, mGarlic(garlic)
	, mOwner(owner)
{

}

Framework::S32 Framework::Allocators::allocate(void **out_memory, SceKernelMemoryType sceKernelMemoryType, U32 size, U32 alignment, sce::Gnm::ResourceType resourceType /*= sce::Gnm::kResourceTypeInvalid*/, sce::Gnm::ResourceHandle *out_resourceHandle /*= nullptr*/, const char *name /*= nullptr*/, ...)
{
	switch (sceKernelMemoryType)
	{
	case SCE_KERNEL_WB_ONION:
		*out_memory = mOnion.allocate(Gnm::SizeAlign(size, alignment));
		break;
	case SCE_KERNEL_WC_GARLIC:
		*out_memory = mGarlic.allocate(Gnm::SizeAlign(size, alignment));
		break;
	default:
		return -1;
		break;
	}

	// register resource information to GPU for debugging
	if (Gnm::kInvalidOwnerHandle != mOwner && resourceType != Gnm::kResourceTypeInvalid && out_resourceHandle != nullptr && name != nullptr)
	{
		va_list args;
		va_start(args, name);
		enum { kBufferSize = 64 };
		char buffer[kBufferSize];
		buffer[0] = 0;
#ifdef __ORBIS__
		vsnprintf(buffer, kBufferSize, name, args);
#else
		_vsnprintf_s(buffer, kBufferSize, _TRUNCATE, name, args);
#endif
		//If PA Debug is disabled, this will return <c>SCE_GNM_ERROR_FAILURE< / c>.
		Gnm::registerResource(out_resourceHandle, mOwner, *out_memory, size, buffer, resourceType, 0);
		va_end(args);
	}
	return 0;
}

Framework::S32 Framework::Allocators::allocate(void **out_memory, SceKernelMemoryType sceKernelMemoryType, sce::Gnm::SizeAlign sizeAlign, sce::Gnm::ResourceType resourceType /*= sce::Gnm::kResourceTypeInvalid*/, sce::Gnm::ResourceHandle *out_resourceHandle /*= nullptr*/, const char *name /*= nullptr*/, ...)
{
	switch (sceKernelMemoryType)
	{
	case SCE_KERNEL_WB_ONION:
		*out_memory = mOnion.allocate(sizeAlign);
		break;
	case SCE_KERNEL_WC_GARLIC:
		*out_memory = mGarlic.allocate(sizeAlign);
		break;
	default:
		return -1;
		break;
	}

	// register resource information to GPU for debugging
	if (Gnm::kInvalidOwnerHandle != mOwner && resourceType != Gnm::kResourceTypeInvalid && out_resourceHandle != nullptr && name != nullptr)
	{
		va_list args;
		va_start(args, name);
		enum { kBufferSize = 64 };
		char buffer[kBufferSize];
		buffer[0] = 0;
#ifdef __ORBIS__
		vsnprintf(buffer, kBufferSize, name, args);
#else
		_vsnprintf_s(buffer, kBufferSize, _TRUNCATE, name, args);
#endif
		//If PA Debug is disabled, this will return <c>SCE_GNM_ERROR_FAILURE< / c>.
		Gnm::registerResource(out_resourceHandle, mOwner, *out_memory, sizeAlign.m_size, buffer, resourceType, 0);
		va_end(args);
	}
	return 0;
}


Framework::S32 Framework::Allocators::release(void *memory, SceKernelMemoryType sceKernelMemoryType, sce::Gnm::ResourceHandle *resourceHandle /*= nullptr*/)
{
	switch (sceKernelMemoryType)
	{
	case SCE_KERNEL_WB_ONION:
		mOnion.release(memory);
		break;
	case SCE_KERNEL_WC_GARLIC:
		mGarlic.release(memory);
		break;
	default:
		return -1;
		break;
	}

	if (resourceHandle != nullptr)
	{
		Gnm::unregisterResource(*resourceHandle);
	}

	return 0;
}

