#include "stdafx.h"

#include "RenderContextChunk.h"

using namespace sce;

void Framework::RenderContextChunk::init(RenderContext *context, U32 id)
{
	mContext = context;
	mID = id;

//#ifdef USING_SEPARATE_CUE_HEAP
	U32 kNumRingEntries = 16;
//#endif

	// TODO MB(1)
	mDcbChunkSize = 1 * 1024 * 1024;
	mCcbChunkSize = 1 * 1024 * 1024;

	m_acb.m_beginptr = m_acb.m_cmdptr = m_acb.m_endptr = nullptr;
	m_currentAcbSubmissionStart = m_actualAcbEnd = nullptr;

	Gnmx::BaseGfxContext::init(NULL, 0, NULL, 0);

//#ifdef USING_SEPARATE_CUE_HEAP
	U32 cueHeapSize = Gnmx::ConstantUpdateEngine::computeHeapSize(kNumRingEntries);
	mCueHeapMemory = g_PhysMemAllocator->Alloc(cueHeapSize, 0x400);
	m_cue.init(m_CueHeapMemory, kNumRingEntries);
	m_cue.bindCommandBuffers(&m_dcb, &m_ccb, nullptr);
//#else
// 	void *t_cueHeap = m_Device->GetCueHeap();
// 	m_cue.init(t_cueHeap, m_Device->GetNumRingEntries());
// 	m_cue.bindCommandBuffers(&m_dcb, &m_ccb, nullptr);
//#endif

	m_Fence = OrbisGPUFenceManager::GetInstance().AllocFence();
	m_State = GnmContextState::kFree;

	m_pNextContext = nullptr;

#ifndef POP_OPTIMIZED
	popSTATIC_ASSERT(GnmContext::SubmitArraySize == (GNMDevice::NumSwapchainBuffers + 1));
	Gnm::setErrorResponseLevel(Gnm::kErrorResponseLevelPrintAndBreak);
#endif
}

void Framework::RenderContextChunk::deinit()
{

}

void Framework::RenderContextChunk::waitUntilIdle()
{

}

bool Framework::RenderContextChunk::isBusy() const
{

}

void Framework::RenderContextChunk::attachFence(GPUFence* fence)
{

}

bool Framework::RenderContextChunk::isEmpty(bool strictChecking /*= false*/) const
{

}

void Framework::RenderContextChunk::submit(bool asynchronous)
{

}

void Framework::RenderContextChunk::submitAndFlip(bool asynchronous)
{

}

void Framework::RenderContextChunk::beginRecord()
{

}

Framework::CommandList * Framework::RenderContextChunk::endRecord()
{

}

ForwardDeclare::Framework::CommandList * Framework::RenderContextChunk::replay(CommandList* cmdList)
{

}

void Framework::RenderContextChunk::prepareToFill()
{

}

ForwardDeclare::Framework::CommandList * Framework::RenderContextChunk::prepareToSubmit(bool flip /*= false*/, bool forceRecord /*= false*/)
{

}

void Framework::RenderContextChunk::updateFenceInCmdBuffer()
{

}

ForwardDeclare::Framework::CommandList * Framework::RenderContextChunk::recodeComandList()
{

}

void Framework::RenderContextChunk::stashPendingCommandList(CommandList* cmdList)
{

}

void Framework::RenderContextChunk::allocCBChunk()
{

}

void Framework::RenderContextChunk::batchKick(std::vector<CommandList>& cmdListArray, bool flip)
{

}

bool Framework::RenderContextChunk::cmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes)
{

}

bool Framework::RenderContextChunk::staticCmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes, void* userData)
{

}

#ifdef POP_PLATFORM_GNM


#include "graphic/gnm/gnmcontext.h"

#include "graphic/gnm/gnmwrapper.h"
#include "system/memory/physmemalloc.h" //@@LRF TODO memory //@@kfh the memory is simply fix, need check
#include <pm4_dump.h>
#include <video_out.h>

popBEGIN_NAMESPACE

using namespace sce;

popDeclareProfile(GNMKickCommandBuffer);
popDeclareProfile(GNMUpdateFenceInCmdBuffer);
popDeclareProfile(GNMPrepareFlip);
popDeclareProfile(GNMRecodeCommandList);
popDeclareProfile(GNMBatchKick);
popDeclareProfile(GNMBlockUntilIdle);

#ifndef POP_OPTIMIZED
ubiBool g_DumpFullFrameCommandBuffer = false;
#endif

Gear::AdaptiveLock          GnmContext::m_Lock;
#ifndef POP_OPTIMIZED
Array<void*>				GnmContext::ms_SubmitBeginPtr[GnmContext::SubmitArraySize];
Array<void*>				GnmContext::ms_SubmitCmdPtr[GnmContext::SubmitArraySize];
ubiU32						GnmContext::ms_CurrentSubmitArrayIndex = 0;
#endif

// Note about our gnmcontext
// We are using it a little bit differently than sony implementation
// We are updating our own beginptr to be able to kick sub-part of the command buffer
// For this reason, we do not use the gnmx::GfxContext submit versions that will internally split large buffers (only needed for submit)
// Or course, if we do have a kick that has a command buffer bigger that 4MB - 4 bytes, we will crash (assert is there to trap that case)
// Some of the sony code to handle large command buffer is thus useless - we should at one point take the whole code and kill useless parts
//   but for now, it's a easy way to do what we want to achieve without having to care about the constant udpate engine
// --------------------------------------------------------------------------
void GnmContext::Initialize(GNMDevice *a_device)
{
	
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::Shutdown()
{
	BlockUntilIdle();
	OrbisGPUFenceManager::GetInstance().ReleaseFence(m_Fence);

#ifdef USING_SEPARATE_CUE_HEAP
	g_PhysMemAllocator->Free(m_CueHeapMemory);
#endif
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::BlockUntilIdle()
{
	popProfile(GNMBlockUntilIdle);
	m_Fence->WaitReadWrite();
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::StartCommandList()
{
	// Make sure our context is empty as expected
	popAssert(m_AttachedFences.Size() == 0);
	popAssert(!IsBusy());
	popAssert(IsEmpty(true));

	m_State = GnmContextState::kInUseDeferred;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
GnmCommandList* GnmContext::EndCommandList()
{
	popAssert(m_State == GnmContextState::kInUseDeferred);

	GnmCommandList* cmdList = PrepareToSubmit();
	if (cmdList == nullptr)
	{
		m_State = GnmContextState::kFree;
	}
	else
	{
		m_State = GnmContextState::kPending;
	}

	return cmdList;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
GnmCommandList* GnmContext::ExecuteCommandList(GnmCommandList* cmdList)
{

	GnmCommandList* ownCmdList = PrepareToSubmit();
	if (ownCmdList != nullptr)
	{
		m_State = GnmContextState::kPending;
		StashPendingCommandList(ownCmdList);
	}
	StashPendingCommandList(cmdList);

	return ownCmdList;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
#ifdef ENABLE_DUMP_CMDBUFFER_USAGE
// for dump commander buffer chunk usage
ubiU32 g_totalDcbChunkSize_main = 0;
ubiU32 g_maxDcbChunkUsage_main = 0;
ubiU32 g_totalCcbChunkSize_main = 0;
ubiU32 g_maxCcbChunkUsage_main = 0;
ubiU32 g_totalDcbChunkSize_deferred = 0;
ubiU32 g_maxDcbChunkUsage_deferred = 0;
ubiU32 g_totalCcbChunkSize_deferred = 0;
ubiU32 g_maxCcbChunkUsage_deferred = 0;
#endif

GnmCommandList* GnmContext::RecodeComandList()
{
	popProfile(GNMRecodeCommandList);

#ifndef POP_OPTIMIZED
	//@@LRF check whether if we got a vailed command list recode.
	uint32_t t_size = (m_dcb.m_cmdptr - m_DcbCurrentBeginPtr) * 4;
	popAssert(t_size >= 0);		// allow empty here
	popAssert(t_size <= sce::Gnm::kIndirectBufferMaximumSizeInBytes);  // We cannot submit in one call a command buffer bigger than this
	t_size = (m_ccb.m_cmdptr - m_CcbCurrentBeginPtr) * 4;
	popAssert(t_size >= 0);		// allow empty here
	popAssert(t_size <= sce::Gnm::kIndirectBufferMaximumSizeInBytes);  // We cannot submit in one call a command buffer bigger than this
#ifdef ENABLE_DUMP_CMDBUFFER_USAGE
	if (m_Device->IsDeferred())
	{
		g_totalDcbChunkSize_deferred = m_DcbChunkSize;
		g_totalCcbChunkSize_deferred = m_CcbChunkSize;
		t_size = (m_dcb.m_cmdptr - m_dcb.m_beginptr) * 4;
		g_maxDcbChunkUsage_deferred = (t_size > g_maxDcbChunkUsage_deferred ? t_size : g_maxDcbChunkUsage_deferred);
		t_size = (m_ccb.m_cmdptr - m_ccb.m_beginptr) * 4;
		g_maxCcbChunkUsage_deferred = (t_size > g_maxCcbChunkUsage_deferred ? t_size : g_maxCcbChunkUsage_deferred);
	}
	else
	{
		g_totalDcbChunkSize_main = m_DcbChunkSize;
		g_totalCcbChunkSize_main = m_CcbChunkSize;
		t_size = (m_dcb.m_cmdptr - m_dcb.m_beginptr) * 4;
		g_maxDcbChunkUsage_main = (t_size > g_maxDcbChunkUsage_main ? t_size : g_maxDcbChunkUsage_main);
		t_size = (m_ccb.m_cmdptr - m_ccb.m_beginptr) * 4;
		g_maxCcbChunkUsage_main = (t_size > g_maxCcbChunkUsage_main ? t_size : g_maxCcbChunkUsage_main);
	}
#endif
#endif

	GnmCommandList* cmdList = popNew(GnmCommandList, "GfxCmdList", NULL);

	cmdList->m_BeginDcbPtr = m_DcbCurrentBeginPtr;
	cmdList->m_EndDcbPtr = m_dcb.m_cmdptr;
	cmdList->m_BeginCcbPtr = m_CcbCurrentBeginPtr;
	cmdList->m_EndCcbPtr = m_ccb.m_cmdptr;
	cmdList->m_Context = this;

	m_DcbCurrentBeginPtr = m_dcb.m_cmdptr;
	m_CcbCurrentBeginPtr = m_ccb.m_cmdptr;

	m_Device->UpdateCmdPtr(m_dcb.m_cmdptr, m_ccb.m_cmdptr);

	return cmdList;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::StashPendingCommandList(GnmCommandList* cmdList)
{
	// discard empty cmdList
	if (!cmdList->isEmpty())
	{
		m_Device->PushPendingCommandList(cmdList);
	}
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::UpdateFenceInCmdBuffer()
{
	popAutoLock(m_DeferredLock);

	popProfile(GNMUpdateFenceInCmdBuffer);

	// Add the fences here - need only one value, set it to all fences
	ubiU64 fenceValue = OrbisGPUFence::GetCurrentGlobalCPUValue();
	for (ubiU32 i = 0; i < m_AttachedFences.Size(); ++i)
	{
		m_AttachedFences[i]->SetFromPreset(fenceValue);
	}

	OrbisGPUFence::SendPreset(this, fenceValue);

	m_AttachedFences.Clear();
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
bool GnmContext::KickCommandBuffer(ubiU32 a_submitMode, ubiU32 videoHandle, ubiU32 flipMode, ubiU32 currentBackbufferIndex, ubiU32 flipArg)
{
	// Can only kick one command buffer at a time
#ifdef POP_MULTITHREAD_RENDERER
	popAutoLock(m_Lock);
#endif
	// Split in begin end so we do not profile empyt kick...
	popProfile(GNMKickCommandBuffer);

	// compute the different Flags
	ubiBool t_flip = (a_submitMode & SUBMIT_AND_FLIP) == SUBMIT_AND_FLIP;
	ubiBool t_wait = (a_submitMode & SUBMIT_AND_WAIT) == SUBMIT_AND_WAIT;

	// Nothing to kick 
	if ((!t_flip) && (m_dcb.m_cmdptr == m_DcbCurrentBeginPtr && m_ccb.m_cmdptr == m_CcbCurrentBeginPtr) && m_Device->GetPendingCommandLists().Size() == 0)
	{
		return false;
	}

	GnmCommandList *ownCmdList = PrepareToSubmit(t_flip, t_wait);

	ubiBool t_return = false;
	if (ownCmdList != nullptr)
	{
		StashPendingCommandList(ownCmdList);
		m_State = GnmContextState::kPending;
		t_return = true;
	}

	// submit all of pending command list
	BatchKick(m_Device->GetPendingCommandLists(), t_flip, videoHandle, flipMode, currentBackbufferIndex, flipArg);

	if (t_wait)
	{
		BlockUntilIdle();
	}

	return t_return;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
#ifndef POP_OPTIMIZED
void GnmContext::ValidateResult(int result)
{
#ifdef ENABLE_DUMP_GPU_SUBMISSION
	switch (result)
	{
	case Gnm::kValidationErrorVsharp:
	case Gnm::kValidationErrorTsharp:
	case Gnm::kValidationErrorResource:
	case Gnm::kValidationErrorTableMemory:
	case Gnm::kValidationErrorWriteEventOp:
	case Gnm::kValidationErrorIndexBuffer:
	case Gnm::kValidationErrorTessFactorBuffer:
	case Gnm::kValidationErrorScratchRing:
	case Gnm::kValidationErrorPrimitiveType:
	case Gnm::kValidationErrorIndexSize:
	case Gnm::kValidationErrorInlineDrawSize:
	case Gnm::kValidationErrorNumInputPatches:
	case Gnm::kValidationErrorGsMode:
	case Gnm::kValidationErrorShaderAddress:
		GNMStateManager::DumpSubmissionLog();
		break;
	}
#endif // ENABLE_DUMP_GPU_SUBMISSION


	switch (result)
	{
	case 0:										/* no error */																										break;
	case Gnm::kValidationDiagnosticErrorShaderResource: popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorShaderResource");      break;
	case Gnm::kValidationDiagnosticErrorSyncFence:		popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorSyncFence");			break;
	case Gnm::kValidationDiagnosticErrorIndexBuffer:    popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorIndexBuffer");			break;
	case Gnm::kValidationDiagnosticErrorTesselation:	popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorTesselation");			break;
	case Gnm::kValidationDiagnosticErrorDraw:			popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorDraw");				break;
	case Gnm::kValidationDiagnosticErrorGeometry:       popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorGeometry");            break;
	case Gnm::kValidationDiagnosticErrorShader:			popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorShader");				break;
	case Gnm::kValidationDiagnosticWarningRenderTarget: popWarningWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticWarningRenderTarget");     break;
	case Gnm::kValidationDiagnosticWarningDepthTarget:	popWarningWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticWarningDepthTarget");		break;
	case SCE_GNM_ERROR_VALIDATION_WARNING:		popWarningWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with SCE_GNM_ERROR_VALIDATION_WARNING");     break;
	default:                                    popAssertWithMsg(0, "GnmContext::KickCommandBuffer: validation failed, error unknown");                             break;
	}
}
#endif
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
ubiBool GnmContext::StaticCmdBufferFullCallback(Gnm::CommandBuffer* cb, ubiU32 reserveSizeInBytes, void* userData)
{
	GnmContext* self = static_cast<GnmContext*>(userData);
	return self->CmdBufferFullCallback(cb, reserveSizeInBytes);
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
ubiBool GnmContext::CmdBufferFullCallback(Gnm::CommandBuffer* cb, ubiU32 reserveSizeInBytes)
{
	popWarningWithMsg(0, "Command Buffer is full - we need to use a bigger command buffer!");
	popAssert0();

	return false;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
int GnmContext::Submit(GnmCommandList** cmdList, ubiU32 cmdListCount, ubiBool flip, ubiU32 videoHandle, ubiU32 flipMode, ubiU32 currentBackbufferIndex, ubiU32 flipArg)
{
	Array<void*> dcbPtr;
	Array<ubiU32> dcbSize;
	Array<void*> ccbPtr;
	Array<ubiU32> ccbSize;

	for (ubiU32 i = 0; i < cmdListCount; ++i)
	{
		dcbPtr.Add(cmdList[i]->m_BeginDcbPtr);
		dcbSize.Add((cmdList[i]->m_EndDcbPtr - cmdList[i]->m_BeginDcbPtr) * 4);
		ccbPtr.Add(cmdList[i]->m_BeginCcbPtr);
		ccbSize.Add((cmdList[i]->m_EndCcbPtr - cmdList[i]->m_BeginCcbPtr) * 4);

		popAssert(dcbSize[i] > 0);
		popAssert(dcbSize[i] <= sce::Gnm::kIndirectBufferMaximumSizeInBytes);  // We cannot submit in one call a command buffer bigger than this
	}
	int result = 0;
	if (flip)
	{
		result = Gnm::submitAndFlipCommandBuffers(cmdListCount, &dcbPtr[0], &dcbSize[0], &ccbPtr[0], &ccbSize[0], videoHandle, currentBackbufferIndex, (SceVideoOutFlipMode)flipMode, flipArg);
	}
	else
	{
		result = Gnm::submitCommandBuffers(cmdListCount, &dcbPtr[0], &dcbSize[0], &ccbPtr[0], &ccbSize[0]);
	}

	if (flip)
	{
#ifndef POP_OPTIMIZED
		ms_CurrentSubmitArrayIndex = (ms_CurrentSubmitArrayIndex + 1) % SubmitArraySize;
		ms_SubmitBeginPtr[ms_CurrentSubmitArrayIndex].Clear();
		ms_SubmitCmdPtr[ms_CurrentSubmitArrayIndex].Clear();
#endif
	}

	return result;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::AttachFence(OrbisGPUFence* fence)
{
	m_AttachedFences.Add(fence);
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::BatchKick(PtrArray<GnmCommandList>& cmdListArray, ubiBool flip, ubiU32 videoHandle, ubiU32 flipMode, ubiU32 currentBackbufferIndex, ubiU32 flipArg)
{
	popProfile(GNMBatchKick);

	popAssert(cmdListArray.Size() > 0); //@@LRF shouldn't be empty at here, even for flip, at least we will have a fence CMD.

	for (ubiU32 i = 0; i < cmdListArray.Size(); ++i)
	{
		GnmCommandList* cmdList = cmdListArray[i];

		// Cmd List should never be empty
		popAssert(!cmdList->isEmpty());
		popAssert(cmdList->m_Context->m_State == GnmContextState::kPending);

#ifndef POP_OPTIMIZED
		ms_SubmitBeginPtr[ms_CurrentSubmitArrayIndex].Add(cmdList->m_BeginDcbPtr);
		ms_SubmitCmdPtr[ms_CurrentSubmitArrayIndex].Add(cmdList->m_EndDcbPtr);

		if (g_DumpFullFrameCommandBuffer)
		{
			FILE* myFile = fopen("/app0/OrbisCmdBufferDump.txt", "a");
			ubiPtrSize cbNumBytes = (ubiPtrSize)cmdList->m_EndDcbPtr - (ubiPtrSize)cmdList->m_BeginDcbPtr;
			Gnm::Pm4Dump::dumpPm4PacketStream(myFile, (ubiU32*)cmdList->m_BeginDcbPtr, cbNumBytes / 4);
			fclose(myFile);
		}
#endif
	}

	int	result = Submit(&(cmdListArray[0]), cmdListArray.Size(), flip, videoHandle, flipMode, currentBackbufferIndex, flipArg);
#ifndef POP_OPTIMIZED
	ValidateResult(result);
#endif

	for (ubiU32 i = 0; i < cmdListArray.Size(); ++i)
	{
		GnmCommandList* cmdList = cmdListArray[i];
		cmdList->m_Context->m_State = GnmContextState::kFree;
		popSafeDelete(cmdList);
	}
	cmdListArray.Clear();
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
#ifndef POP_OPTIMIZED
void GnmContext::DumpBlockingCommandbuffer()
{
	ubiU64 blockingCmdBufferIdx = OrbisGPUFence::GetCurrentGlobalCPUValue() - (*(OrbisGPUFence::GetGPUAddress()));

	if (blockingCmdBufferIdx > 0)
	{
		ubiU32 submitArrayIdx = ms_CurrentSubmitArrayIndex;
		Array<void*>* beginPtr = &ms_SubmitBeginPtr[submitArrayIdx];
		Array<void*>* cmdPtr = &ms_SubmitCmdPtr[submitArrayIdx];

		ubiS32 submitIdx = beginPtr->Size() - blockingCmdBufferIdx;
		while (submitIdx < 0)
		{
			// blocking cmdbuffer is in a previous frame...
			if (submitArrayIdx > 0)
				submitArrayIdx--;
			else
				submitArrayIdx = SubmitArraySize - 1;

			beginPtr = &ms_SubmitBeginPtr[submitArrayIdx];
			cmdPtr = &ms_SubmitCmdPtr[submitArrayIdx];
			submitIdx += beginPtr->Size();
		}

		FILE* myFile = fopen("/app0/OrbisCmdBufferDump_Block.txt", "w");
		ubiPtrSize cbNumBytes = (ubiPtrSize)((*cmdPtr)[submitIdx]) - (ubiPtrSize)((*beginPtr)[submitIdx]);
		Gnm::Pm4Dump::dumpPm4PacketStream(myFile, (ubiU32*)((*beginPtr)[submitIdx]), cbNumBytes / 4);
		fclose(myFile);
	}
}
#endif // POP_OPTIMIZED
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::FlushGPUCaches()
{
	volatile uint64_t* label = (volatile uint64_t*)allocateFromCommandBuffer(sizeof(uint64_t), Gnm::kEmbeddedDataAlignment8); // allocate memory from the command buffer
	*label = 0x0; // set the memory to have the val 0
	writeAtEndOfShader(Gnm::kEosPsDone, const_cast<uint64_t*>(label), 0x1); // tell the CP to write a 1 into the memory only w&hen all shaders have finished
	waitOnAddress(const_cast<uint64_t*>(label), 0xffffffff, Gnm::kWaitCompareFuncEqual, 0x1); // tell the CP to wait until the memory has the val 1


	Gnm::DrawCommandBuffer* dcb = &(m_dcb);
	dcb->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackAndInvalidateL1andL2,
		Gnm::kExtendedCacheActionFlushAndInvalidateCbCache |
		Gnm::kExtendedCacheActionFlushAndInvalidateDbCache |
		Gnm::kExtendedCacheActionInvalidateKCache |
		Gnm::kExtendedCacheActionInvalidateICache,
		Gnm::kStallCommandBufferParserEnable);
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
ubiU64 GnmContext::InsertFence()
{
	ubiU32 cacheAction = sce::Gnm::kCacheActionNone;
	cacheAction |= sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2;
	volatile ubiU32* label = (volatile ubiU32*)allocateFromCommandBuffer(sizeof(ubiU32), Gnm::kEmbeddedDataAlignment8);
	*label = 0x0;
	writeAtEndOfShader(Gnm::kEosPsDone, const_cast<ubiU32*>(label), 0x1);

	Gnm::DrawCommandBuffer* dcb = &(m_dcb);
	dcb->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackAndInvalidateL1andL2,
		Gnm::kExtendedCacheActionFlushAndInvalidateCbCache |
		Gnm::kExtendedCacheActionFlushAndInvalidateDbCache |
		Gnm::kExtendedCacheActionInvalidateKCache |
		Gnm::kExtendedCacheActionInvalidateICache,
		Gnm::kStallCommandBufferParserEnable);
	return (ubiU64)label;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
ubiBool GnmContext::IsFencePending(ubiU64 fence)
{
	if (fence != 0)
	{
		volatile ubiU32* label = (volatile ubiU32*)fence;
		return (*label == 0);
	}
	return false;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
ubiBool GnmContext::IsBusy() const
{
	return m_Fence->IsPendingReadWrite() || (m_State != GnmContextState::kFree);
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
ubiBool GnmContext::IsEmpty(ubiBool strictChecking /*= false*/) const
{
	ubiBool t_res = false;
	t_res = (m_DcbCurrentBeginPtr == m_dcb.m_cmdptr) && (m_CcbCurrentBeginPtr == m_ccb.m_cmdptr);
	if (strictChecking)
	{
		t_res = t_res && (m_dcb.m_beginptr == m_dcb.m_cmdptr) && (m_ccb.m_beginptr == m_ccb.m_cmdptr);
	}

	return t_res;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
GnmCommandList * GnmContext::PrepareToSubmit(ubiBool a_flip /*= false*/, ubiBool a_forceRecord /*= false*/)
{
	ubiBool t_needRecode = false;
	if (a_forceRecord || (!IsEmpty()))
	{
		// set this context fence to pending, will also push it to attachedFences.
		m_Fence->Set(this, true);
		UpdateFenceInCmdBuffer();
		t_needRecode = true;

	}

	//@@LRF Only increase the fence value when we preparing the main context, 
	//@@LRF otherwise(if you also update the value for deferred). you will get the fence value order issue as the deferred context won't be kicked immediately.
	if (!m_Device->IsDeferred())
	{
		OrbisGPUFence::Preset();
	}

	if (a_flip)
	{
		// Prepares the system to flip command buffers.
		// This function is intended to be used in conjunction with Gnm::submitAndFlipCommandBuffers().
		// This function MUST BE the last command to be inserted in the command buffer before a call to Gnm::submitAndFlipCommandBuffers().
		//@@LRF we do this here, so that we can also record it into end of pending command list.
		m_dcb.prepareFlip();
		t_needRecode = true;
	}

	GnmCommandList* cmdList = nullptr;
	if (t_needRecode)
	{
		cmdList = RecodeComandList();
	}

	return cmdList;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::PrepareToFill()
{
	popAssert(IsBusy() == false);

	AllocCBChunk();

	// reset
	m_submissionCount = 0;
	m_currentDcbSubmissionStart = m_dcb.m_beginptr;
	m_actualDcbEnd = (uint32_t*)m_dcb.m_beginptr + (m_DcbChunkSize / 4);

	m_currentCcbSubmissionStart = m_ccb.m_beginptr;
	m_actualCcbEnd = (uint32_t*)m_ccb.m_beginptr + (m_CcbChunkSize / 4);

#ifndef USING_SEPARATE_CUE_HEAP
	void *t_cueHeap = m_Device->GetCueHeap();
	//m_cue.bindCommandBuffers(&m_dcb, &m_ccb, nullptr);
	m_cue.moveConstantStateAfterAddress(t_cueHeap);

	//@@LRF to be fix, get flicking if use this way, should be I always use 't_cueHeap' as begin address with out kick constant buffer, cause some overwrite!
#endif

	GfxContext::reset();
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void GnmContext::AllocCBChunk()
{
	uint32_t *t_DcbCmdPtr = m_Device->GetCurrentDcbCmdPtr(m_DcbChunkSize);
	popAssert(t_DcbCmdPtr != NULL);

	uint32_t *t_CcbCmdPtr = m_Device->GetCurrentCcbCmdPtr(m_CcbChunkSize);
	popAssert(t_CcbCmdPtr != NULL);

	m_dcb.init(t_DcbCmdPtr, m_DcbChunkSize, GnmContext::StaticCmdBufferFullCallback, this);
	m_dcb.resetBuffer();
	m_DcbCurrentBeginPtr = m_dcb.m_beginptr;
	m_ccb.init(t_CcbCmdPtr, m_CcbChunkSize, GnmContext::StaticCmdBufferFullCallback, this);
	m_ccb.resetBuffer();
	m_CcbCurrentBeginPtr = m_ccb.m_beginptr;
}
// --------------------------------------------------------------------------
popEND_NAMESPACE

#endif // POP_PLATFORM_GNM

