#include "stdafx.h"

#include "GPUResource.h"
#include "GPUFence.h"

Framework::BaseGPUResource::BaseGPUResource()
{
	mFence = GPUFenceManager::getInstance()->allocFence();
}

Framework::BaseGPUResource::~BaseGPUResource()
{
	GPUFenceManager::getInstance()->allocFence(mFence);
	mFence = nullptr;
}
