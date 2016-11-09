#include "stdafx.h"

#include "RenderContextChunk.h"
#include "GraphicDevice.h"
#include "Swapchain.h"
#include "OutputDevice.h"
#include "RenderContext.h"
#include "Memory/Allocators.h"
#include "GPUFence.h"
#include "CommandList.h"

using namespace sce;

void Framework::RenderContextChunk::init(RenderContext *owner, U32 id, Allocators *allocators)
{
	mContext = owner;
	mID = id;

	//#ifdef USING_SEPARATE_CUE_HEAP
	U32 kNumRingEntries = 16;
	//#endif

	mDcbChunkSize = UTIL_MB(1);
	mCcbChunkSize = UTIL_MB(1);

	m_acb.m_beginptr = m_acb.m_cmdptr = m_acb.m_endptr = nullptr;
	m_currentAcbSubmissionStart = m_actualAcbEnd = nullptr;

	Gnmx::BaseGfxContext::init(NULL, 0, NULL, 0);

	//#ifdef USING_SEPARATE_CUE_HEAP
	U32 cueHeapSize = Gnmx::ConstantUpdateEngine::computeHeapSize(kNumRingEntries);
	allocators->allocate(&mCueHeapMemory, SCE_KERNEL_WC_GARLIC, cueHeapSize, 4, Gnm::kResourceTypeGenericBuffer, &mCueHandle, "Chunk %d Constant Update Engine", mID);
	m_cue.init(mCueHeapMemory, kNumRingEntries);
	m_cue.bindCommandBuffers(&m_dcb, &m_ccb, nullptr);
	//#else
	// 	void *t_cueHeap = m_Device->GetCueHeap();
	// 	m_cue.init(t_cueHeap, m_Device->GetNumRingEntries());
	// 	m_cue.bindCommandBuffers(&m_dcb, &m_ccb, nullptr);
	//#endif

	mFence = GPUFenceManager::getInstance()->allocFence();
	mState = FREE;

	mNextChunk = nullptr;

	// TODO debug option
	Gnm::setErrorResponseLevel(Gnm::kErrorResponseLevelPrintAndBreak);
}

void Framework::RenderContextChunk::deinit(Allocators *allocators)
{
	waitUntilIdle();
	GPUFenceManager::getInstance()->releaseFence(mFence);

	//#ifdef USING_SEPARATE_CUE_HEAP
	allocators->release(mCueHeapMemory, SCE_KERNEL_WC_GARLIC, &mCueHandle);
	//#endif
}

void Framework::RenderContextChunk::prepareToFill()
{
	SCE_GNM_ASSERT(!isBusy());

	prepareCommandBuffers();

	// reset
	m_submissionCount = 0;
	m_currentDcbSubmissionStart = m_dcb.m_beginptr;
	m_actualDcbEnd = (uint32_t*)m_dcb.m_beginptr + (mDcbChunkSize / 4);

	m_currentCcbSubmissionStart = m_ccb.m_beginptr;
	m_actualCcbEnd = (uint32_t*)m_ccb.m_beginptr + (mCcbChunkSize / 4);

	//#ifndef USING_SEPARATE_CUE_HEAP
	// 	void *t_cueHeap = m_Device->GetCueHeap();
	// 	//m_cue.bindCommandBuffers(&m_dcb, &m_ccb, nullptr);
	// 	m_cue.moveConstantStateAfterAddress(t_cueHeap);

	//TODO to be fix, get flicking if use this way, should be I always use 't_cueHeap' as begin address with out kick constant buffer, cause some overwrite!
	//#endif

	GfxContext::reset();
}

void Framework::RenderContextChunk::waitUntilIdle()
{
	SCE_GNM_ASSERT(mFence != nullptr);
	mFence->waitUntilIdle();
}

bool Framework::RenderContextChunk::isBusy() const
{
	return (mFence->isBusy() || (mState != FREE));
}

Framework::CommandList * Framework::RenderContextChunk::internalRecord(bool appendFlipRequest /*= false*/, bool forceRecord /*= false*/)
{
	bool _needRecode = false;
	if (forceRecord || (!isEmpty()))
	{
		// Set this context fence to pending, will also push it to attachedFences.
		mFence->setPending(this);
		shipFences();
		_needRecode = true;

	}

	// Only increase the fence value when we preparing the immediate context, 
	// Otherwise(if you also update the value for deferred). you will get the fence value order issue as the deferred context won't be kicked immediately.
	if (!mContext->mIsDeferred)
	{
		GPUFenceManager::getInstance()->preset();
	}

	if (appendFlipRequest)
	{
		// Prepares the system to flip command buffers.
		// This function is intended to be used in conjunction with Gnm::submitAndFlipCommandBuffers().
		// This function MUST BE the last command to be inserted in the command buffer before a call to Gnm::submitAndFlipCommandBuffers().
		// We do this here, so that we can also record it into end of pending command list.
		m_dcb.prepareFlip();
		_needRecode = true;
	}

	CommandList* cmdList = nullptr;
	if (_needRecode)
	{
		// Check whether if we got a valid command list recode.
		auto t_size = (m_dcb.m_cmdptr - mDcbCurrentBeginPtr) * 4;
		SCE_GNM_ASSERT(t_size >= 0);		// allow empty here
		SCE_GNM_ASSERT(t_size <= sce::Gnm::kIndirectBufferMaximumSizeInBytes);  // We cannot submit in one call a command buffer bigger than this
		t_size = (m_ccb.m_cmdptr - mCcbCurrentBeginPtr) * 4;
		SCE_GNM_ASSERT(t_size >= 0);		// allow empty here
		SCE_GNM_ASSERT(t_size <= sce::Gnm::kIndirectBufferMaximumSizeInBytes);  // We cannot submit in one call a command buffer bigger than this

		cmdList = new CommandList;
		cmdList->mBeginDcbPtr = mDcbCurrentBeginPtr;
		cmdList->mEndDcbPtr = m_dcb.m_cmdptr;
		cmdList->mBeginCcbPtr = mCcbCurrentBeginPtr;
		cmdList->mEndCcbPtr = m_ccb.m_cmdptr;
		cmdList->mSourceChunk = this;

		mDcbCurrentBeginPtr = m_dcb.m_cmdptr;
		mCcbCurrentBeginPtr = m_ccb.m_cmdptr;

		mContext->updateCmdPtr(m_dcb.m_cmdptr, m_ccb.m_cmdptr);
	}

	return cmdList;
}

void Framework::RenderContextChunk::shipFences()
{
	// Add the fences here - need only one value, set it to all fences
	U64 _value = GPUFenceManager::getInstance()->getExpectedLabel();
	for (auto itor = mAttachedFences.begin(); itor != mAttachedFences.end(); itor++)
	{
		(*itor)->setValue(_value);
	}
	GPUFenceManager::getInstance()->appendLabelToGPU(mContext, _value);
	mAttachedFences.clear();
}

void Framework::RenderContextChunk::prepareCommandBuffers()
{
	U32 *_dcbCmdPtr = mContext->getCurrentDcbCmdPtr(mDcbChunkSize);
	SCE_GNM_ASSERT(_dcbCmdPtr != nullptr);

	U32 *_ccbCmdPtr = mContext->getCurrentCcbCmdPtr(mCcbChunkSize);
	SCE_GNM_ASSERT(_ccbCmdPtr != nullptr);

	m_dcb.init(_dcbCmdPtr, mDcbChunkSize, RenderContextChunk::staticCmdBufferFullCallback, this);
	m_dcb.resetBuffer();
	mDcbCurrentBeginPtr = m_dcb.m_beginptr;
	m_ccb.init(_ccbCmdPtr, mCcbChunkSize, RenderContextChunk::staticCmdBufferFullCallback, this);
	m_ccb.resetBuffer();
	mCcbCurrentBeginPtr = m_ccb.m_beginptr;
}

bool Framework::RenderContextChunk::cmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes)
{
	/** @brief Function to call when the command buffer is out of space.

	@param[in,out] cb	Pointer to the CommandBuffer object that is out of space.
	@param[in] sizeInDwords Size of the unfulfilled CommandBuffer request, in <c>DWORD</c>s.
	@return <c>true</c> if the requested space is available in cb when the function returns, or <c>false</c> if not.
	*/

	SCE_GNM_ASSERT_MSG(false, "Command Buffer is full - we need to use a bigger command buffer! extra %d(dwords) is requested", reserveSizeInBytes);
	// TODO grow command buffer
	return false;
}

bool Framework::RenderContextChunk::staticCmdBufferFullCallback(sce::Gnm::CommandBuffer* cb, U32 reserveSizeInBytes, void* userData)
{
	RenderContextChunk* _self = static_cast<RenderContextChunk *>(userData);
	SCE_GNM_ASSERT(_self != nullptr);
	return _self->cmdBufferFullCallback(cb, reserveSizeInBytes);
}

Framework::CommandList * Framework::ImmediateRenderContextChunk::replay(CommandList* cmdList)
{
	// TODO : auto lock
	CommandList* ownCmdList = internalRecord();
	if (ownCmdList != nullptr)
	{
		mState = PENDING;
		stashPendingCommandList(ownCmdList);
	}
	stashPendingCommandList(cmdList);

	return ownCmdList;
}

bool Framework::ImmediateRenderContextChunk::kickCommandBuffer(const Bitset &actionFlag /*= Bitset(KICK_ONLY)*/)
{
	bool _requestFlip = actionFlag.get(REQUEST_FLIP);
	bool _postSync = actionFlag.get(POST_SYNC);

	// Nothing to kick 
	if ((!_requestFlip) && isEmpty() && mContext->mPendingCommandLists.empty())
	{
		return false;
	}

	CommandList *ownCmdList = internalRecord(_requestFlip, _postSync);

	bool ret = false;
	if (ownCmdList != nullptr)
	{
		stashPendingCommandList(ownCmdList);
		mState = PENDING;
		ret = true;
	}

	// submit all of pending command list
	batchKick(_requestFlip);

	if (_postSync)
	{
		waitUntilIdle();
	}

	return ret;
}

void Framework::ImmediateRenderContextChunk::stashPendingCommandList(CommandList* cmdList)
{
	// discard empty cmdList
	if (!cmdList->empty())
	{
		mContext->mPendingCommandLists.push_back(cmdList);
	}
}

void Framework::ImmediateRenderContextChunk::batchKick(bool flip)
{
	std::vector<CommandList *> &_pendingCmdLists = mContext->mPendingCommandLists;
	SCE_GNM_ASSERT(!_pendingCmdLists.empty()); // Shouldn't be empty at here, even for flip, at least we will have a fence CMD.
	const auto _size = _pendingCmdLists.size();

	void **_dcbGpuAddrs = (void **)malloc(_size * sizeof(void *));
	U32 *_dcbSizesInBytes = (U32 *)malloc(_size * sizeof(U32));
	void **_ccbGpuAddrs = (void **)malloc(_size * sizeof(void *));
	U32 *_ccbSizesInBytes = (U32 *)malloc(_size * sizeof(U32));

	for (auto i = 0; i < _size; i++)
	{
		CommandList *_cmdList = _pendingCmdLists[i];
		SCE_GNM_ASSERT(!_cmdList->empty());
		SCE_GNM_ASSERT(_cmdList->mSourceChunk->getState() == PENDING);
#ifndef POP_OPTIMIZED
		// 		ms_SubmitBeginPtr[ms_CurrentSubmitArrayIndex].Add(cmdList->m_BeginDcbPtr);
		// 		ms_SubmitCmdPtr[ms_CurrentSubmitArrayIndex].Add(cmdList->m_EndDcbPtr);
		// 
		// 		if (g_DumpFullFrameCommandBuffer)
		// 		{
		// 			FILE* myFile = fopen("/app0/OrbisCmdBufferDump.txt", "a");
		// 			ubiPtrSize cbNumBytes = (ubiPtrSize)cmdList->m_EndDcbPtr - (ubiPtrSize)cmdList->m_BeginDcbPtr;
		// 			Gnm::Pm4Dump::dumpPm4PacketStream(myFile, (ubiU32*)cmdList->m_BeginDcbPtr, cbNumBytes / 4);
		// 			fclose(myFile);
		// 		}
#endif
		_dcbGpuAddrs[i] = _cmdList->mBeginDcbPtr;
		_dcbSizesInBytes[i] = (_cmdList->mEndDcbPtr - _cmdList->mBeginDcbPtr) * 4;
		_ccbGpuAddrs[i] = _cmdList->mBeginCcbPtr;
		_ccbSizesInBytes[i] = (_cmdList->mEndCcbPtr - _cmdList->mBeginCcbPtr) * 4;

		SCE_GNM_ASSERT(_dcbSizesInBytes[i] > 0);
		SCE_GNM_ASSERT(_dcbSizesInBytes[i] <= sce::Gnm::kIndirectBufferMaximumSizeInBytes);  // We cannot submit in one call a command buffer bigger than this
	}


	Result ret = SCE_GNM_OK;
	if (flip)
	{
		GraphicDevice *_device = mContext->mDevice;
		OutputDevice::DeviceHandle _handle = _device->getOutput()->getHandle();
		SceVideoOutFlipMode _flipMode = _device->getSwapChain()->getDescription().mFilpMode;
		U32 _displayBufferIndex = _device->getSwapChain()->getCurrentBufferIndex();
		// A user - provided argument with no internal meaning.The <c><i>flipArg< / i>< / c> associated with the most recently completed flip is
		// included in the <c>SceVideoOutFlipStatus< / c> object retrieved by <c>sceVideoOutGetFlipStatus() < / c > ; it could therefore
		// be used to uniquely identify each flip.
		U64 _flipArg = _device->getSwapChain()->getFrameCount();

		ret = Gnm::submitAndFlipCommandBuffers(_size, _dcbGpuAddrs, _dcbSizesInBytes, _ccbGpuAddrs, _ccbSizesInBytes, _handle, _displayBufferIndex, _flipMode, _flipArg);
	}
	else
	{
		ret = Gnm::submitCommandBuffers(_size, _dcbGpuAddrs, _dcbSizesInBytes, _ccbGpuAddrs, _ccbSizesInBytes);
	}

	// 	if (flip)
	// 	{
	// #ifndef POP_OPTIMIZED
	// 		ms_CurrentSubmitArrayIndex = (ms_CurrentSubmitArrayIndex + 1) % SubmitArraySize;
	// 		ms_SubmitBeginPtr[ms_CurrentSubmitArrayIndex].Clear();
	// 		ms_SubmitCmdPtr[ms_CurrentSubmitArrayIndex].Clear();
	// #endif
	// 	}

	validateResult(ret);

	for (auto itor = _pendingCmdLists.begin(); itor != _pendingCmdLists.end(); itor++)
	{
		(*itor)->mSourceChunk->setState(FREE);
		SAFE_DELETE(*itor);
	}
	_pendingCmdLists.clear();
}

void Framework::ImmediateRenderContextChunk::validateResult(Result ret)
{
	SCE_GNM_ASSERT_MSG(ret == sce::Gnm::kSubmissionSuccess, "Command buffer validation error.");

	// TODO parse and report the validation error
	// 	switch (ret)
	// 	{
	// 	case 0:										/* no error */																										break;
	// 	case Gnm::kValidationDiagnosticErrorShaderResource: SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorShaderResource");      break;
	// 	case Gnm::kValidationDiagnosticErrorSyncFence:		SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorSyncFence");			break;
	// 	case Gnm::kValidationDiagnosticErrorIndexBuffer:    SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorIndexBuffer");			break;
	// 	case Gnm::kValidationDiagnosticErrorTesselation:	SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorTesselation");			break;
	// 	case Gnm::kValidationDiagnosticErrorDraw:			SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorDraw");				break;
	// 	case Gnm::kValidationDiagnosticErrorGeometry:       SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorGeometry");            break;
	// 	case Gnm::kValidationDiagnosticErrorShader:			SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticErrorShader");				break;
	// 	case Gnm::kValidationDiagnosticWarningRenderTarget: popWarningWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticWarningRenderTarget");     break;
	// 	case Gnm::kValidationDiagnosticWarningDepthTarget:	popWarningWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with kValidationDiagnosticWarningDepthTarget");		break;
	// 	case SCE_GNM_ERROR_VALIDATION_WARNING:				popWarningWithMsg(0, "GnmContext::KickCommandBuffer: validation failed with SCE_GNM_ERROR_VALIDATION_WARNING");     break;
	// 	default:											SCE_GNM_ASSERT(0, "GnmContext::KickCommandBuffer: validation failed, error unknown");                             break;
	// 	}
}

void Framework::DeferredRenderContextChunk::beginRecord()
{
	// Make sure our context is empty as expected
	SCE_GNM_ASSERT(mAttachedFences.empty());
	SCE_GNM_ASSERT(!isBusy());
	SCE_GNM_ASSERT(isEmpty(true));
	mState = INUSE_DEFERRED;
}

Framework::CommandList * Framework::DeferredRenderContextChunk::endRecord()
{
	SCE_GNM_ASSERT(mState == INUSE_DEFERRED);

	CommandList* cmdList = internalRecord();
	if (cmdList == nullptr)
	{
		mState = FREE;
	}
	else
	{
		mState = PENDING;
	}

	return cmdList;
}
