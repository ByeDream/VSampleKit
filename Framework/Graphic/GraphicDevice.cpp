#include "stdafx.h"

#include "GraphicDevice.h"
#include "Application.h"
#include "Memory/Allocators.h"
#include "Memory/StackAllocator.h"
#include "OutputDevice.h"
#include "Swapchain.h"

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

	OutputDevice::Description _desc;
	_desc.mWidth = mApp->getConfig()->m_targetWidth;
	_desc.mHeight = mApp->getConfig()->m_targetHeight;


	mOutput = new OutputDevice(_desc);
	mOutput->startup();

	mSwapchain = new Swapchain(this);
	mSwapchain->init();
}

void Framework::GraphicDevice::deinit()
{
	mSwapchain->deinit();
	SAFE_DELETE(mSwapchain);

	mOutput->shutdown();
	SAFE_DELETE(mOutput);

	deinitMem();
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

	mOnionAllocator->init(SCE_KERNEL_WB_ONION, mApp->getConfig()->m_onionMemoryInBytes);
	mGarlicAllocator->init(SCE_KERNEL_WC_GARLIC, mApp->getConfig()->m_garlicMemoryInBytes);
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
