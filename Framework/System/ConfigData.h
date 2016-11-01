#pragma once

namespace Framework
{
	struct ConfigData
	{
		U32 mOnionMemoryInBytes{ 268435456 };		// 256M
		U32 mGarlicMemoryInBytes{ 536870912 };		// 512M
		U32 mTargetWidth{ 1920 };					// 1080p
		U32 mTargetHeight{ 1080 };					// 1080p

		U32 mNumberOfSwappedBuffers{ 3 };			// Triple-buffer
	};
}