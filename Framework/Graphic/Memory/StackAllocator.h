#pragma once

#include "IAllocator.h"

namespace Framework
{
	class StackAllocator
	{
		friend IAllocator GetInterface(StackAllocator *stackAllocator);
	public:
		StackAllocator();
		~StackAllocator();
		void init(SceKernelMemoryType type, U32 size);
		void deinit();
	private:
		struct StackAllocatorAllocation
		{
			U8 *startPtr;
			U8 *endPtr;
		};

		enum { kMaximumAllocations = 8192 };
		StackAllocatorAllocation mAllocation[kMaximumAllocations];
		U8 *mBase;
		off_t mOffset;
		size_t mSize;
		off_t mTop;
		U64 mAllocations;
		U64 mAlignment;
		SceKernelMemoryType mType;
		bool mIsInitialized;
		U8 mReserved[3];
		void *allocate(sce::Gnm::SizeAlign sizeAlign);
		void release(void *pointer);

		static void *staticAllocate(void *instance, sce::Gnm::SizeAlign sizeAlign);
		static void staticRelease(void *instance, void *pointer);
	};

	IAllocator GetInterface(StackAllocator *stackAllocator);
}
