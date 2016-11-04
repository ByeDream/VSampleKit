#pragma once

#include <vector>

namespace Framework
{
	class Allocators;
	class RenderContext;
	class EopEventQueue;
	class RenderContextChunk;

	class GPUFence
	{
		friend class GPUFenceManager;

	public:
		enum State
		{
			IDLE = 0,
			PENDING = 1,

			LABEL_VALUE_START,
		};

		void							setPending(RenderContext *context);
		void							setPending(RenderContextChunk *contextChunk);  //TODO too expensive, to be refactoring
		inline bool						isBusy();
		void							waitUntilIdle();
		inline void						setValue(U64 value) { mValue = value; }
		inline U64						getValue() const { return mValue; }
		
	private:
		GPUFence();
		~GPUFence();
		GPUFence &						operator=(const GPUFence &);		// not allowed

	private:
		U32								mValue{ IDLE };
		EopEventQueue *					mEopEventQueue{ nullptr };

	};

	class GPUFenceManager : public Singleton<GPUFenceManager>
	{
		friend class Singleton<GPUFenceManager>;

	public:
		void							init(Allocators *allocators, U32 poolSize);
		void							deinit(Allocators *allocators);

		inline void						preset() { mExpectedLabel = Framework::max(mExpectedLabel + 1, (U64)GPUFence::LABEL_VALUE_START); }
		inline U64						getExpectedLabel() const { return mExpectedLabel; }
		void							appendLabelToGPU(RenderContext *context, U64 value);

		inline const volatile U64 *		getLabel() const { return mLabel; }

		inline GPUFence *				allocFence() { return new GPUFence; }
		void							releaseFence(GPUFence *fence);

		void							flip();

	private:
		GPUFenceManager();
		~GPUFenceManager();

		void							clearPool(U32 index);
	private:
		typedef std::vector<GPUFence *> FencePool;

		FencePool *						mPool{ nullptr };
		U32								mPoolSize{ 0 };
		U32								mPoolIndex{ 0 };

		volatile U64 *					mLabel{ nullptr };
		U64								mExpectedLabel{ 0 };
		sce::Gnm::ResourceHandle		mLabelHandle{ sce::Gnm::kInvalidResourceHandle };
	};
}

bool Framework::GPUFence::isBusy()
{
	const volatile U64 *_label = GPUFenceManager::getInstance()->getLabel();
	bool ret = (mValue == PENDING) || (((*_label - mValue) >> 63) != 0);
	if (!ret)
		mValue = IDLE;
	return ret;
}
