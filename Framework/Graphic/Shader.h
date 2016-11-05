#pragma once

namespace Framework
{
	class Allocators;
	class BaseShaderView;
	class RenderContext;

	class Shader
	{
	public:
		typedef U32 Handle;
		enum
		{
			SHADER_HANDLE_INVALID = MAX_VALUE_32,
		};

		struct Description
		{
			ShaderType mType{ SHADER_TYPE_COUNT };
			const U8 *mDataPtr{ nullptr };
			const char *mName{ nullptr };
		};

		Shader();
		virtual ~Shader();

		virtual void				init(const Description &desc, Allocators *allocators);
		virtual void				deinit(Allocators *allocators);

		virtual BaseShaderView *	getShaderView() const { return mShaderView; }
		virtual void				bindAsShader(RenderContext *context) const;

		inline const Description &	getDescription() const { return mDesc; }

		inline void					setHandle(Handle handle) { mHandle = handle; }
		inline Handle				getHandle() const { return mHandle; }

	protected:
		void						createShaderView();
		void						allocMemory(Allocators *allocators);

	protected:
		Handle						mHandle{ SHADER_HANDLE_INVALID };
		Description					mDesc;
		BaseShaderView *			mShaderView{ nullptr };
		sce::Gnm::ResourceHandle	mHandle{ sce::Gnm::kInvalidResourceHandle };
		void *						mHeaderAddr{ nullptr };
		void *						mGpuBaseAddr{ nullptr };

		// for vs shader only
		sce::Gnm::ResourceHandle	mFetchShaderHandle{ sce::Gnm::kInvalidResourceHandle };
		void *						mFetchShaderAddr{ nullptr };
	};
}