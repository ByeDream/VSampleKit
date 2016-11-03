#pragma once

#ifndef __GRAPHIC__GNM__CONTEXT__H__
#define __GRAPHIC__GNM__CONTEXT__H__

popBEGIN_NAMESPACE

enum GnmContextState
{
	kFree = 0,
	kInUseDeferred,
	kPending,
};

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


	void								Initialize(GNMDevice *a_device);
	void								Shutdown();

	void								SetHandle(ubiU32 a_handle) { m_Handle = a_handle; }
	ubiU32								GetHandle() const { return m_Handle; }

	void								BlockUntilIdle();
	bool								KickCommandBuffer(ubiU32 a_submitMode = SUBMIT_FLAG_NONE, ubiU32 videoHandle = 0, ubiU32 flipMode = 0, ubiU32 currentBackbufferIndex = 0, ubiU32 flipArg = 0);
	void								FlushGPUCaches();

	// @@guoxx: TODO
	// need to re-consider implementation later
	ubiU64								InsertFence();
	ubiBool								IsFencePending(ubiU64 fence);

	void								AttachFence(OrbisGPUFence* fence);

	void								StartCommandList();
	GnmCommandList*						EndCommandList();
	GnmCommandList*						ExecuteCommandList(GnmCommandList* cmdList);

	static Gear::AdaptiveLock&			GetLock() { return m_Lock; }

	ubiBool								IsBusy() const;
	//@@LRF if strictChecking == false, the context is still empty, but not fresh, may includes some submitted data.
	ubiBool								IsEmpty(ubiBool strictChecking = false) const;

	void								PrepareToFill();
	GnmCommandList *					PrepareToSubmit(ubiBool a_flip = false, ubiBool a_forceRecord = false);

#ifndef POP_OPTIMIZED
	static void							DumpBlockingCommandbuffer();
	void								ValidateResult(int result);
#endif

	GnmContext *						m_pNextContext{ nullptr };

private:
	void								UpdateFenceInCmdBuffer();
	GnmCommandList*						RecodeComandList();
	void								StashPendingCommandList(GnmCommandList* cmdList);

	void								AllocCBChunk();

	void								BatchKick(PtrArray<GnmCommandList>& cmdListArray, ubiBool flip, ubiU32 videoHandle, ubiU32 flipMode, ubiU32 currentBackbufferIndex, ubiU32 flipArg);
	int									Submit(GnmCommandList** cmdList, ubiU32 cmdListCount, ubiBool flip, ubiU32 videoHandle, ubiU32 flipMode, ubiU32 currentBackbufferIndex, ubiU32 flipArg);

	ubiBool								CmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, ubiU32 reserveSizeInBytes);
	static ubiBool						StaticCmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, ubiU32 reserveSizeInBytes, void* userData);

private:
	GNMDevice *							m_Device{ nullptr };
	ubiU32								m_Handle{ 0 };		// A unique handle corresponding to each context

	ubiU32								m_DcbChunkSize;
	ubiU32								m_CcbChunkSize;

#ifdef USING_SEPARATE_CUE_HEAP
	void*								m_CueHeapMemory;
#endif

	// Internal pointer that we move ourselves to kick sub-part of the command buffer
	uint32_t*							m_DcbCurrentBeginPtr;
	uint32_t*							m_CcbCurrentBeginPtr;

	BigArray<OrbisGPUFence*>			m_AttachedFences;
	OrbisGPUFence*						m_Fence;
	GnmContextState						m_State;

	static Gear::AdaptiveLock			m_Lock;
	Gear::AdaptiveLock					m_DeferredLock;

#ifndef POP_OPTIMIZED
	enum { SubmitArraySize = 4 };
	static Array<void*> ms_SubmitBeginPtr[SubmitArraySize];
	static Array<void*> ms_SubmitCmdPtr[SubmitArraySize];
	static ubiU32 ms_CurrentSubmitArrayIndex;
#endif
};

popEND_NAMESPACE

#endif //#ifndef __GRAPHIC__GNM__CONTEXT__H__

