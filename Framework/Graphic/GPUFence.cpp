#include "stdafx.h"
#include "GPUFence.h"
#include "Memory/Allocators.h"
#include "ChunkBasedRenderContext/RenderContext.h"
#include "ChunkBasedRenderContext/RenderContextChunk.h"
#include "GraphicHelpers.h"

using namespace sce;

void Framework::GPUFence::setPending(RenderContext *context)
{
	// TODO
}

void Framework::GPUFence::setPending(RenderContextChunk *contextChunk)
{
	mValue = PENDING;
	contextChunk->attachFence(this);
}

void Framework::GPUFence::waitUntilIdle()
{
	while (isBusy())
		mEopEventQueue->waitForEvent();
}

Framework::GPUFence::GPUFence()
{
	mEopEventQueue = new EopEventQueue("Fence queue");
}

Framework::GPUFence::~GPUFence()
{
	SCE_GNM_ASSERT(mValue != PENDING);
	SAFE_DELETE(mEopEventQueue);
}

void Framework::GPUFenceManager::init(Allocators *allocators, U32 poolSize)
{
	mPoolSize = poolSize;
	mPool = new FencePool[mPoolSize];
	mPoolIndex = 0;

	allocators->allocate((void**)&mLabel, SCE_KERNEL_WB_ONION, sizeof(U64), sizeof(U64), Gnm::kResourceTypeLabel, &mLabelHandle, "GPUFenceManager Label");
	*mLabel = MAX_VALUE_64;
}

void Framework::GPUFenceManager::deinit(Allocators *allocators)
{
	allocators->release((void *)mLabel, SCE_KERNEL_WB_ONION, &mLabelHandle);
	mLabel = nullptr;
	
	for (mPoolIndex = 0; mPoolIndex < mPoolSize; mPoolIndex++)
	{
		clearPool(mPoolIndex);
	}
	SAFE_DELETE_ARRAY(mPool);
}

void Framework::GPUFenceManager::appendLabelToGPU(RenderContext *context, U64 value)
{
	context->appendLabelAtEOPWithInterrupt((void *)mLabel, value);
}

void Framework::GPUFenceManager::releaseFence(GPUFence *fence)
{
	if (fence->mValue == GPUFence::PENDING)
	{
		mPool[mPoolIndex].push_back(fence);
	}
	else
	{
		SAFE_DELETE(fence);
	}
}

void Framework::GPUFenceManager::flip()
{
	mPoolIndex = (mPoolIndex + 1) % mPoolSize;
	clearPool(mPoolIndex);
}

Framework::GPUFenceManager::GPUFenceManager()
{

}

Framework::GPUFenceManager::~GPUFenceManager()
{
	SCE_GNM_ASSERT_MSG(mLabel == nullptr && mPool == nullptr, "deinit me before destroy me");
}

void Framework::GPUFenceManager::clearPool(U32 index)
{
	for (auto itor = mPool[index].begin(); itor != mPool[index].end(); itor++)
	{
		SAFE_DELETE(*itor);
	}
	mPool[index].clear();
}
