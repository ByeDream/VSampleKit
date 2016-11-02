#include "stdafx.h"

#include "GraphicDevice.h"
#include "Application.h"
#include "Memory/Allocators.h"
#include "Memory/StackAllocator.h"
#include "OutputDevice.h"
#include "Swapchain.h"
#include "RenderSurfaceManager.h"
#include "RenderSet.h"
#include "RenderContext.h"

using namespace sce;

Framework::GraphicDevice::GraphicDevice(Application *app)
	: mApp(app)
{
	
}

Framework::GraphicDevice::~GraphicDevice()
{

}

void Framework::GraphicDevice::init()
{
	initMem();

	RenderSurfaceManager::getInstance()->setAllocator(mAllocators);

	OutputDevice::Description _desc;
	_desc.mWidth = mApp->getConfig()->mTargetWidth;
	_desc.mHeight = mApp->getConfig()->mTargetHeight;
	mOutput = new OutputDevice(_desc);
	mOutput->startup();

	mSwapChain = new SwapChain(this);
	mSwapChain->init(mAllocators);

	initContexts();
}

void Framework::GraphicDevice::deinit()
{
	deinitContexts();

	mSwapChain->deinit(mAllocators);
	SAFE_DELETE(mSwapChain);

	mOutput->shutdown();
	SAFE_DELETE(mOutput);

	RenderSurfaceManager::destory();

	deinitMem();
}

void Framework::GraphicDevice::createRenderSet(RenderSet *out_renderSet, const RenderSurface::Description *depth, const RenderSurface::Description *color0, const RenderSurface::Description *color1 /*= nullptr*/, const RenderSurface::Description *color2 /*= nullptr*/, const RenderSurface::Description *color3 /*= nullptr*/)
{
	RenderSurface *_depth	= nullptr;
	RenderSurface *_color0	= nullptr;
	RenderSurface *_color1	= nullptr;
	RenderSurface *_color2	= nullptr;
	RenderSurface *_color3	= nullptr;

	RenderSurfaceManager *_mgr = RenderSurfaceManager::getInstance();
	_mgr->createSurface(&_depth, depth);
	_mgr->createSurface(&_color0, color0);
	_mgr->createSurface(&_color1, color1);
	_mgr->createSurface(&_color2, color2);
	_mgr->createSurface(&_color3, color3);

	out_renderSet->init(_depth, _color0, _color1, _color2, _color3);
}

void Framework::GraphicDevice::rollImmediateContext()
{
	mContexts[0]->roll();
	mContexts[0]->reset();
}

void Framework::GraphicDevice::rollDeferreContext()
{
	for (auto itor = mContexts.begin() + 1; itor != mContexts.end(); itor++)
	{
		(*itor)->roll();
		(*itor)->reset();
	}
}

void Framework::GraphicDevice::initMem()
{
	// register graphic resource owner as this application
	Gnm::OwnerHandle ownerHandle;
	Result res = Gnm::registerOwner(&ownerHandle, mApp->getTitleName());
	SCE_GNM_ASSERT(res == SCE_OK);

	// init allocators
	mOnionAllocator = new StackAllocator();
	mGarlicAllocator = new StackAllocator();

	mAllocators = new Allocators(GetInterface(mOnionAllocator), GetInterface(mGarlicAllocator), ownerHandle);

	mOnionAllocator->init(SCE_KERNEL_WB_ONION, mApp->getConfig()->mOnionMemoryInBytes);
	mGarlicAllocator->init(SCE_KERNEL_WC_GARLIC, mApp->getConfig()->mGarlicMemoryInBytes);
}

void Framework::GraphicDevice::deinitMem()
{
	Result res = Gnm::unregisterOwnerAndResources(mAllocators->mOwner);
	SCE_GNM_ASSERT(res == SCE_OK);

	mOnionAllocator->deinit();
	mGarlicAllocator->deinit();

	SAFE_DELETE(mAllocators);
	SAFE_DELETE(mOnionAllocator);
	SAFE_DELETE(mGarlicAllocator);
}

void Framework::GraphicDevice::initContexts()
{
	// TODO mContexts
	
}

void Framework::GraphicDevice::deinitContexts()
{
	// TODO
}
