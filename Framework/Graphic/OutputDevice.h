#pragma once

namespace Framework
{
	class RenderTargetView;
	class OutputDevice
	{
	public:
		struct Description
		{
			U32 mWidth;
			U32 mHeight;
		};

		typedef S32 DeviceHandle;
		enum 
		{
			INVAILD_DEV_HANDLE = -1,
		};
		

		OutputDevice(const Description &desc);
		~OutputDevice();

		DeviceHandle			startup();
		void					shutdown();

		void					registerBufferChain(void * const *addresses, U32 bufferNum, RenderTargetView *view);
		void					unregisterBufferChain();

		inline const Rect &		getWindowRect() const { return mWindowRect; }
		inline bool				is4K() const { return mIs4K; }
		inline DeviceHandle		getHandle() const { SCE_GNM_ASSERT(mHandle != INVAILD_DEV_HANDLE); return mHandle; }

	private:
		Rect					mWindowRect;
		bool					mIs4K{ false };
		DeviceHandle			mHandle{ INVAILD_DEV_HANDLE };
		S32						mRegistrationIndex{ 0 };
	};
}