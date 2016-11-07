#pragma once

namespace Framework
{
	class GPUFence;
	enum GPUResourceType
	{
		RESOURCE_TYPE_SURFACE = 0,
		RESOURCE_TYPE_SHADER,

		RESOURCE_TYPE_COUNT


	};

	typedef U32 GPUResourceHandle;
	enum
	{
		SURFACE_HANDLE_START = 0,
		MAX_NUMBER_OF_SURFACE = 3000,

		SHADER_HANDLE_START = SURFACE_HANDLE_START + MAX_NUMBER_OF_SURFACE,
		MAX_NUMBER_OF_SHADER = 3000,

		RESOURCE_HANDLE_INVALID = MAX_VALUE_32,
	};

	class Allocators;
	class BaseGPUResource
	{
	public:
		struct Description {
			virtual ~Description() {}  // make it polymorphic
		};

		BaseGPUResource();
		virtual ~BaseGPUResource();

		virtual void						init(const BaseGPUResource::Description *desc, Allocators *allocators) = 0;
		virtual void						deinit(Allocators *allocators) = 0;

		inline void							setHandle(GPUResourceHandle handle) { mHandle = handle; }
		inline GPUResourceHandle			getHandle() const { return mHandle; }

		inline void							setType(GPUResourceType type) { mType = type; }
		inline GPUResourceType				getType() const { return mType; }

		inline GPUFence *					getFence() const { return mFence; }
		bool								isBusy();
		void								waitUntilIdle();
	protected:
		GPUResourceType						mType{ RESOURCE_TYPE_SURFACE };
		GPUResourceHandle					mHandle{ RESOURCE_HANDLE_INVALID };
		GPUFence *							mFence{ nullptr };
	};
}
