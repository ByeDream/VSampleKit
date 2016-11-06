#include "stdafx.h"

#include "OutputDevice.h"
#include "GraphicHelpers.h"
#include "GPUResource/GPUResourceViews.h"

using namespace sce;

Framework::OutputDevice::OutputDevice(const Description &desc)
{
	
}

Framework::OutputDevice::~OutputDevice()
{
	
}

Framework::OutputDevice::DeviceHandle Framework::OutputDevice::startup()
{
	SCE_GNM_ASSERT(mHandle == INVAILD_DEV_HANDLE);
	// open video out
	mHandle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
	SCE_GNM_ASSERT_MSG(mHandle >= 0, "sceVideoOutOpen() returned error code %d.", mHandle);

	// check resolution
	SceVideoOutResolutionStatus status;
	Result ret = sceVideoOutGetResolutionStatus(mHandle, &status);
	SCE_GNM_ASSERT(ret == SCE_OK);
	mWindowRect.left = 0;
	mWindowRect.right = status.fullWidth;
	mWindowRect.top = 0;
	mWindowRect.bottom = status.fullWidth;

	mIs4K = status.fullWidth > 1080;

	return mHandle;
}

void Framework::OutputDevice::shutdown()
{
	SCE_GNM_ASSERT(mHandle != INVAILD_DEV_HANDLE);
	sceVideoOutClose(mHandle);
	mHandle = INVAILD_DEV_HANDLE;

	memset(&mWindowRect, 0, sizeof(Rect));
}

void Framework::OutputDevice::registerBufferChain(void * const *addresses, U32 bufferNum, RenderTargetView *view)
{
	SCE_GNM_ASSERT(mHandle != INVAILD_DEV_HANDLE);

	Gnm::DataFormat _format = view->getInternalObj()->getDataFormat();
	Gnm::TileMode _tileMode = view->getInternalObj()->getTileMode();
	U32 _width = view->getInternalObj()->getWidth();
	U32 _height = view->getInternalObj()->getHeight();
	U32 _pitchInPixel = view->getInternalObj()->getPitch();

	SceVideoOutBufferAttribute _attribute;
	sceVideoOutSetBufferAttribute(&_attribute,
		getVideoOutFormat(_format),
		getVideoOutTileMode(_tileMode),
		SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
		_width,
		_height,
		_pitchInPixel);
	mRegistrationIndex = sceVideoOutRegisterBuffers(mHandle, 0, addresses, bufferNum, &_attribute);
	SCE_GNM_ASSERT_MSG(mRegistrationIndex >= 0, "sceVideoOutRegisterBuffers() returned error code %d.", mRegistrationIndex);
}

void Framework::OutputDevice::unregisterBufferChain()
{
	Result ret = sceVideoOutUnregisterBuffers(mHandle, mRegistrationIndex);
	SCE_GNM_ASSERT_MSG(ret != SCE_OK, "sceVideoOutUnregisterBuffers() returned error code %d.", ret);
}
