#pragma once

#include <vector>

namespace Framework
{
	struct CommandList;
	class GPUFence;
	class RenderContextChunk : public sce::Gnmx::GfxContext
	{
		friend class RenderContext;

	public:
		enum State
		{
			Free = 0,
			InUseDeferred,
			Pending,
		};

		void								init(RenderContext *context, U32 id);
		void								deinit();

		void								waitUntilIdle();
		bool								isBusy() const;
		void								attachFence(GPUFence* fence);
		// if strictChecking == false, the context is still empty, but not fresh, may includes some submitted data.
		bool								isEmpty(bool strictChecking = false) const;

		void								submit(bool asynchronous);
		void								submitAndFlip(bool asynchronous);

		void								beginRecord();
		CommandList *						endRecord();
		CommandList *						replay(CommandList* cmdList);

		void								prepareToFill();
		CommandList *						prepareToSubmit(bool flip = false, bool forceRecord = false);



		//void								flushGPUCaches();

		//bool								insertFence();
		//bool								isFencePending(U64 fence);

#ifndef POP_OPTIMIZED
		//static void						dumpBlockingCommandbuffer();
		//void								validateResult(int result);
#endif


	private:
		void								updateFenceInCmdBuffer();
		CommandList *						recodeComandList();
		void								stashPendingCommandList(CommandList* cmdList);

		void								allocCBChunk();

		void								batchKick(std::vector<CommandList>& cmdListArray, bool flip);

		bool								cmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes);
		static bool							staticCmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes, void* userData);

	private:
		RenderContextChunk *				mNextChunk{ nullptr };
		RenderContext *						mContext{ nullptr };
		U32									mID{ 0 };		// A unique handle corresponding to each context

		U32									mDcbChunkSize{ 0 };
		U32									mCcbChunkSize{ 0 };

//#ifdef USING_SEPARATE_CUE_HEAP
		void *								mCueHeapMemory{ nullptr };
//#endif

		// Internal pointer that we move ourselves to kick sub-part of the command buffer
		U32 *								mDcbCurrentBeginPtr{ nullptr };
		U32 *								mCcbCurrentBeginPtr{ nullptr };

		std::vector<GPUFence *>				mAttachedFences;
		GPUFence *							mFence;
		State								mState;


// #ifndef POP_OPTIMIZED
// 		enum { SubmitArraySize = 4 };
// 		static Array<void*> ms_SubmitBeginPtr[SubmitArraySize];
// 		static Array<void*> ms_SubmitCmdPtr[SubmitArraySize];
// 		static ubiU32 ms_CurrentSubmitArrayIndex;
// #endif
	};
}


/*

struct GnmCommandList
{
	GnmCommandList() : m_BeginDcbPtr(NULL), m_EndDcbPtr(NULL), m_BeginCcbPtr(NULL), m_EndCcbPtr(NULL) {}
	uint32_t* m_BeginDcbPtr;
	uint32_t* m_EndDcbPtr;
	uint32_t* m_BeginCcbPtr;
	uint32_t* m_EndCcbPtr;

	GnmContext* m_Context;

	bool isEmpty() { return (m_EndDcbPtr == m_BeginDcbPtr) && (m_EndCcbPtr == m_BeginCcbPtr); }
};

class GnmContext : public sce::Gnmx::GfxContext
{
public:
	enum SubmitFlag
	{
		SUBMIT_FLAG_NONE = 0,
		SUBMIT_AND_FLIP = 1,
		SUBMIT_AND_WAIT = 2,
	};


	

private:
	
};

*/

