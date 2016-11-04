#pragma once

#include <vector>

namespace Framework
{
	struct CommandList;
	class GPUFence;
	class Allocators;
	class RenderContext;

	class RenderContextChunk : public sce::Gnmx::GfxContext
	{
		friend class RenderContext;

	public:
		enum State
		{
			FREE = 0,
			PENDING,
			INUSE_DEFERRED,
		};

		void								init(RenderContext *owner, U32 id, Allocators *allocators);
		void								deinit(Allocators *allocators);
		void								prepareToFill();

		void								waitUntilIdle();
		bool								isBusy() const;
		inline void							attachFence(GPUFence* fence) { mAttachedFences.push_back(fence); }

		inline State						getState() const { return mState; }
		inline void							setState(State state) { mState = state; }

		// TODO
		//void								flushGPUCaches();
		//static void						dumpBlockingCommandbuffer();

	protected:
		virtual ~RenderContextChunk() {}

		CommandList *						internalRecord(bool appendFlipRequest = false, bool forceRecord = false);
		void								shipFences();
		void								prepareCommandBuffers();
		bool								cmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes);
		static bool							staticCmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes, void* userData);

		// if strictChecking == false, the context is still empty, but not fresh, may includes some submitted data.
		inline bool							isEmpty(bool strictChecking = false) const
		{
			bool ret = false;
			// Every thing has been submitted
			ret = (mDcbCurrentBeginPtr == m_dcb.m_cmdptr) && (mCcbCurrentBeginPtr == m_ccb.m_cmdptr);
			if (strictChecking)
			{
				// Already reset
				ret = ret && (m_dcb.m_beginptr == m_dcb.m_cmdptr) && (m_ccb.m_beginptr == m_ccb.m_cmdptr);
			}
			return ret;
		}

	protected:
		RenderContextChunk *				mNextChunk{ nullptr };
		RenderContext *						mContext{ nullptr };
		U32									mID{ 0 };		// A unique handle corresponding to each context

		U32									mDcbChunkSize{ 0 };
		U32									mCcbChunkSize{ 0 };

//#ifdef USING_SEPARATE_CUE_HEAP
		void *								mCueHeapMemory{ nullptr };
		sce::Gnm::ResourceHandle			mCueHandle{ sce::Gnm::kInvalidResourceHandle };
//#endif

		// Internal pointer that we move ourselves to kick sub-part of the command buffer
		U32 *								mDcbCurrentBeginPtr{ nullptr };
		U32 *								mCcbCurrentBeginPtr{ nullptr };

		std::vector<GPUFence *>				mAttachedFences;
		GPUFence *							mFence{ nullptr };
		State								mState{ FREE };
	
// #ifndef POP_OPTIMIZED
		// for dumpBlockingCommandbuffer
// 		static Array<void*>					ms_SubmitBeginPtr[NumSwapchainBuffers];
// 		static Array<void*>					ms_SubmitCmdPtr[NumSwapchainBuffers];
// 		static U32							ms_CurrentSubmitArrayIndex;
// #endif
	};

	class ImmediateRenderContextChunk : public RenderContextChunk
	{
	public:
		enum KickingAction
		{
			// as bit flag
			KICK_ONLY = 0,
			REQUEST_FLIP = 1,
			POST_SYNC = 2,
		};

		CommandList *						replay(CommandList* cmdList);
		bool								kickCommandBuffer(const Bitset &actionFlag = Bitset(KICK_ONLY));

	protected:
		void								stashPendingCommandList(CommandList* cmdList);
		void								batchKick(bool flip);
		void								validateResult(Result ret);

	};

	class DeferredRenderContextChunk : public RenderContextChunk
	{
	public:
		void								beginRecord();
		CommandList *						endRecord();
	};
}
