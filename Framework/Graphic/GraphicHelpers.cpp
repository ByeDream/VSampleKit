#include "stdafx.h"

#include "GraphicHelpers.h"

using namespace sce;

Framework::EopEventQueue::EopEventQueue(const char *name)
{
	mName = name;

	Result ret = sceKernelCreateEqueue(&mEqueue, mName);
	SCE_GNM_ASSERT_MSG(ret >= 0, "sceKernelCreateEqueue returned error code %d.", ret);
	ret = Gnm::addEqEvent(mEqueue, sce::Gnm::kEqEventGfxEop, NULL);
	SCE_GNM_ASSERT_MSG(ret == 0, "sce::Gnm::addEqEvent returned error code %d.", ret);
}

Framework::EopEventQueue::~EopEventQueue()
{
	sceKernelDeleteEqueue(mEqueue);
}

void Framework::EopEventQueue::waitForEvent()
{
	SceKernelEvent ev;
	int num;
	Result ret = sceKernelWaitEqueue(mEqueue, &ev, 1, &num, nullptr);
	SCE_GNM_ASSERT(ret == SCE_OK);
}
