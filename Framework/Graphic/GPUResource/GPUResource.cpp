#include "stdafx.h"

#include "GPUResource.h"
#include "GPUFence.h"

Framework::BaseGPUResource::BaseGPUResource()
{
	mFence = GPUFenceManager::getInstance()->allocFence();
}

Framework::BaseGPUResource::~BaseGPUResource()
{
	GPUFenceManager::getInstance()->releaseFence(mFence);
	mFence = nullptr;
}

bool Framework::BaseGPUResource::isBusy()
{
	return mFence->isBusy();
}

void Framework::BaseGPUResource::waitUntilIdle()
{
	mFence->waitUntilIdle();
}
