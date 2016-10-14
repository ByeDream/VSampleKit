#pragma once

namespace Framework
{
	struct IAllocator
	{
		void *mInstance;
		void *(*mAllocate)(void *instance, sce::Gnm::SizeAlign sizeAlign);
		void (*mRelease)(void *instance, void *pointer);

		void *allocate(sce::Gnm::SizeAlign sizeAlign)
		{
			return mAllocate(mInstance, sizeAlign);
		}

		void release(void *pointer)
		{
			if (pointer != nullptr)
				mRelease(mInstance, pointer);
		}
	};
}
