#pragma once

namespace Framework
{
	class RenderContextChunk;
	struct CommandList
	{
		U32 *					mBeginDcbPtr{ nullptr };
		U32 *					mEndDcbPtr{ nullptr };
		U32 *					mBeginCcbPtr{ nullptr };
		U32 *					mEndCcbPtr{ nullptr };
		RenderContextChunk *	mSourceChunk{ nullptr }; // for state update after kick

		inline bool empty() const { return (mEndDcbPtr == mBeginDcbPtr) && (mEndCcbPtr == mBeginCcbPtr); }
	};
}