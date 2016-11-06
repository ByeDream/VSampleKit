#pragma once

#include "GPUResource.h"

namespace Framework
{
	class Allocators;
	class BaseShaderView;
	class RenderContext;

	class Shader : public BaseGPUResource
	{
	public:
		struct Description : public BaseGPUResource::Description
		{
			ShaderType						mType{ SHADER_TYPE_COUNT };
			const U8 *						mDataPtr{ nullptr };
			const char *					mName{ nullptr };
		};

		Shader();
		virtual ~Shader();

		virtual void						init(const BaseGPUResource::Description *desc, Allocators *allocators);
		virtual void						deinit(Allocators *allocators);

		virtual BaseShaderView *			getShaderView() const { return mShaderView; }
		virtual void						bindAsShader(RenderContext *context) const;

		inline const Description &			getDescription() const { return mDesc; }

	protected:
		void								createShaderView();
		void								allocMemory(Allocators *allocators);

	protected:
		Description							mDesc;
		BaseShaderView *					mShaderView{ nullptr };
		sce::Gnm::ResourceHandle			mBinaryHandle{ sce::Gnm::kInvalidResourceHandle };
		void *								mHeaderAddr{ nullptr };
		void *								mBinaryAddr{ nullptr };

		// for vs shader only
		sce::Gnm::ResourceHandle			mFetchShaderHandle{ sce::Gnm::kInvalidResourceHandle };
		void *								mFetchShaderAddr{ nullptr };
	};
}