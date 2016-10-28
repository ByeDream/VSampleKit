#include "stdafx.h"
#include "StackAllocator.h"

using namespace sce;

Framework::StackAllocator::StackAllocator()
	: mIsInitialized(false)
{

}

Framework::StackAllocator::~StackAllocator()
{
	if (mIsInitialized)
	{
		deinit();
	}
}

void Framework::StackAllocator::init(SceKernelMemoryType type, U32 size)
{
	SCE_GNM_ASSERT(false == mIsInitialized);

	const size_t HW_MEM_PAGE_SIZE = 2 * 1024 * 1024;

	mAllocations = 0;
	memset(mAllocation, 0, sizeof(mAllocation));
	mTop = 0;
	mType = type;
	mSize = size;
	mAlignment = HW_MEM_PAGE_SIZE;
	mBase = 0;

	// Direct memory assigned to application programs exists in consecutive areas in a physical address space. In addition, the beginning of these areas is guaranteed to be aligned to a 2 MiB boundary.
	// Direct memory comprises a single consecutive area beginning from physical address 0. The size is defined with the SCE_KERNEL_MAIN_DMEM_SIZE macro.
	Result ret = sceKernelAllocateDirectMemory(0,
		SCE_KERNEL_MAIN_DMEM_SIZE,
		mSize,
		mAlignment, // alignment
		mType,
		&mOffset);

	SCE_GNM_ASSERT(SCE_OK == ret);

	ret = sceKernelMapDirectMemory(&reinterpret_cast<void*&>(mBase),
		mSize,
		SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL,
		0,						//flags
		mOffset,
		mAlignment);

	SCE_GNM_ASSERT(SCE_OK == ret);

	mIsInitialized = true;

}

void Framework::StackAllocator::deinit()
{
	SCE_GNM_ASSERT(true == mIsInitialized);
	Result ret = sceKernelReleaseDirectMemory(mOffset, mSize);
	SCE_GNM_ASSERT(SCE_OK == ret);
	mIsInitialized = false;
}

void * Framework::StackAllocator::allocate(sce::Gnm::SizeAlign sizeAlign)
{
	SCE_GNM_ASSERT(true == mIsInitialized);
	SCE_GNM_ASSERT(mAllocations < kMaximumAllocations); // out of max allocations

	uint32_t size = sizeAlign.m_size;
	uint32_t alignment = sizeAlign.m_align;

	// alignment
	const uint32_t mask = alignment - 1;
	mTop = (mTop + mask) & ~mask;

	void* result = mAllocation[mAllocations].startPtr = mBase + mTop;
	mTop += size;
	SCE_GNM_ASSERT(mTop <= static_cast<off_t>(mSize)); // out of the memory pool
	mAllocation[mAllocations].endPtr = mBase + mTop;
	mAllocations++;

	return result;
}

void Framework::StackAllocator::release(void *pointer)
{
	SCE_GNM_ASSERT(true == mIsInitialized);
	SCE_GNM_ASSERT(mAllocations > 0);
	mAllocations--;
	SCE_GNM_ASSERT(pointer == mAllocation[mAllocations].startPtr);

	uint8_t* lastPointer = (mAllocations == 0) ? mBase : mAllocation[mAllocations - 1].endPtr;
	mTop = lastPointer - mBase;
}

void * Framework::StackAllocator::staticAllocate(void *instance, sce::Gnm::SizeAlign sizeAlign)
{
	return static_cast<Framework::StackAllocator*>(instance)->allocate(sizeAlign);
}

void Framework::StackAllocator::staticRelease(void *instance, void *pointer)
{
	static_cast<Framework::StackAllocator*>(instance)->release(pointer);
}

Framework::IAllocator Framework::GetInterface(StackAllocator *stackAllocator)
{
	IAllocator allocator;
	allocator.mInstance = stackAllocator;
	allocator.mAllocate = StackAllocator::staticAllocate;
	allocator.mRelease = StackAllocator::staticRelease;
	return allocator;
}
