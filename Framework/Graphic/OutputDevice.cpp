#include "stdafx.h"

#include "OutputDevice.h"

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
