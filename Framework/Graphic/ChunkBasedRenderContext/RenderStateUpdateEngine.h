#pragma once

#include "RenderStates.h"

namespace Framework
{
	class RenderContext;
	class BaseShaderView;
	class TextureView;
	class BaseTargetView;

	class RenderStateUpdateEngine
	{
	public:
		RenderStateUpdateEngine(RenderContext *owner);
		~RenderStateUpdateEngine();

		void									setRenderState(RenderStateType state, U32 value);
		inline void								setRenderStateFloat(RenderStateType state, Float32 value) { setRenderState(state, rawCast<U32>(value)); }
		U32										getRenderState(RenderStateType state) const;
		inline Float32							getRenderStateFloat(RenderStateType state) const { return rawCast<Float32>(getRenderState(state)); }

		inline void								setShader(sce::Gnm::ShaderStage shaderStage, BaseShaderView *shader) { getCurrentResourceBinding().mShaders[shaderStage] = shader; mDirtyFlag.set(DIRTY_SHADERS); }
		inline BaseShaderView *					getShader(sce::Gnm::ShaderStage shaderStage) const { return getCurrentResourceBinding().mShaders[shaderStage]; }
		inline void								setTexture(sce::Gnm::ShaderStage shaderStage, U32 slot, TextureView *texture) { SCE_GNM_ASSERT(slot < MAX_NUM_SAMPLERS); getCurrentResourceBinding().mTextures[shaderStage][slot] = texture; mDirtyFlag.set(DIRTY_TEXTURES); mDelegateTextureDirtyFlag[shaderStage].set(1 << slot); }
		inline TextureView *					getTexture(sce::Gnm::ShaderStage shaderStage, U32 slot) const { SCE_GNM_ASSERT(slot < MAX_NUM_SAMPLERS); return getCurrentResourceBinding().mTextures[shaderStage][slot]; }
		inline void								setSamplerState(sce::Gnm::ShaderStage shaderStage, U32 slot, SamplerStateType state, U32 value) { SCE_GNM_ASSERT(slot < MAX_NUM_SAMPLERS); internalSetSamplerState(getCurrentResourceBinding().mSamplerStates[shaderStage][slot], state, value); mDirtyFlag.set(DIRTY_SAMPLERS); mDelegateSamplerDirtyFlag[shaderStage].set(1 << slot); }
		inline U32								getSamplerState(sce::Gnm::ShaderStage shaderStage, U32 slot, SamplerStateType state) const { SCE_GNM_ASSERT(slot < MAX_NUM_SAMPLERS); return internalGetSamplerState(getCurrentResourceBinding().mSamplerStates[shaderStage][slot], state); }
		void									setSamplerStateForAllSamplers(SamplerStateType state, U32 value);
		inline void								setRenderTarget(U32 slot, BaseTargetView *target) { SCE_GNM_ASSERT(slot < MAX_NUM_RENDER_TARGETS); getCurrentResourceBinding().mRenderTargets[slot] = target; mDirtyFlag.set(DIRTY_RENDER_TARGETS); }
		inline BaseTargetView *					getRenderTarget(U32 slot) const { SCE_GNM_ASSERT(slot < MAX_NUM_RENDER_TARGETS); return getCurrentResourceBinding().mRenderTargets[slot]; }
		inline void								setDepthStencilTarget(BaseTargetView *target) { getCurrentResourceBinding().mDepthStencilTarget = target; mDirtyFlag.set(DIRTY_DEPTH_STENCIL_TARGET); }
		inline BaseTargetView *					getDepthStencilTarget() const { return getCurrentResourceBinding().mDepthStencilTarget; }

// 		void SetViewport(const PlatformGfxViewport& vp);
// 		void SetScissorRect(const GFX_RECT& rect);
// 
// 		void SetVertexDeclaration(PlatformGfxInputLayout* pDecl);
// 		void SetVertexBuffer(ubiUInt index, const PlatformGfxBaseBuffer* pBuffer, ubiUInt offset, ubiUInt stride);
// 		void SetIndexBuffer(const PlatformGfxBaseBuffer* pBuffer, GFXFORMAT fmt, ubiUInt offset);
// 
// 		
// 
// 		// Texture filter default value overwrite
// 		void SetDefaultLODBias(ubiU32 lodBias);
// 		void SetAnisotropicFilteringOverride(ubiBool enabled, ubiU8 anisotropyDegree);
// 
 		void									compile();
 		void									commit();
		inline void								reset()
		{
			mStackLevel = 0;
			memcpy(&mStatesStack[mStackLevel], &mDefaultStates, sizeof(CompleteRenderStates));
			fullsetDirtyFlags();
		}

		inline void								push()
		{
			U32 _nextLevel = mStackLevel + 1;
			if (_nextLevel < STACK_SIZE)
			{
				memcpy(&mStatesStack[_nextLevel], &mStatesStack[mStackLevel], sizeof(CompleteRenderStates));
				mStackLevel = _nextLevel;
			}
			else
			{
				SCE_GNM_ASSERT_MSG(false, "Out of Render states stack range");
			}
		}

		inline void								pop()
		{
			U32 _prevLevel = mStackLevel - 1;
			if (_prevLevel > 0)
			{
				mStackLevel = _prevLevel;
				fullsetDirtyFlags();
			}
			else
			{
				SCE_GNM_ASSERT_MSG(false, "Out of Render states stack range");
			}
		}
// 		void ResetRenderStates();
// 
// 		inline ubiBool DrawCallGuard() const
// 		{
// 			ubiBool pass = true;
// 			// attempting to draw Color primitives without a PixelShader will cause untrackable GPU exceptions, so don't do it
// 			if (((m_RenderTargets[0] != NULL) && ((m_CompressedRenderStates.m_WriteColorMask & 0xf) != 0) && (m_PixelShader == NULL)))
// 			{
// 				pass = false;
// 			}
// 			//@@LRF Todo more
// 
// 			return pass;
// 		}
// 
// 		inline ubiU32 GetCurrentIndexSizeInBytes() const
// 		{
// 			return m_IndexSizeInBytes;
// 		}
// 
// 		// initialize all dirty flags (sure not to forget any!)
// 		void SetAllDirtyFlags()
// 		{
// 			// reset the DirtyFlags
// 			m_dirtyFlags = DIRTY_ALL;
// 			m_psSamplerDirtyFlags = ((0x01 << popGetArraySize(m_PSSamplerStates)) - 1);
// 			m_vsSamplerDirtyFlags = ((0x01 << popGetArraySize(m_VSSamplerStates)) - 1);
// 			m_renderTargetDirtyFlags = ((0x01 << popGetArraySize(m_RenderTargets)) - 1);
// 			m_psBufferDirtyFlags = ((0x01 << popGetArraySize(m_PSBuffers)) - 1);
// 		}

		//@@LRF for render states dirty flags part only!!
		void ResetRenderStatesDirtyFlags();

		void FullResetStates();
		void ResetDefaultStates();
	private:
		


		enum DirtyFlagBit
		{
			// render states
			DIRTY_PRIMITIVE_SETUP				= 1 << 0,
			DIRTY_CLIP_CONTROL					= 1 << 1,
			DIRTY_COLOR_WRITE					= 1 << 2,
			DIRTY_DEPTH_STENCIL_CONTROL			= 1 << 3,
			DIRTY_DB_RENDER_CONTROL				= 1 << 4,
			DIRTY_STENCIL_CONTROL				= 1 << 5,
			DIRTY_STENCIL_OP_CONTROL			= 1 << 6,
			DIRTY_ALPHA_TEST					= 1 << 7,
			DIRTY_BLEND_CONTROL					= 1 << 8,

			// resource binding
			DIRTY_SHADERS						= 1 << 20,
			DIRTY_TEXTURES						= 1 << 21,
			DIRTY_SAMPLERS						= 1 << 22,
			DIRTY_RENDER_TARGETS				= 1 << 23,
			DIRTY_DEPTH_STENCIL_TARGET			= 1 << 24,


// 			DIRTY_VIEWPORT						= 1 << 0,
// 			DIRTY_SCISSORRECT					= 1 << 1,
// 			DIRTY_PIXELSHADER					= 1 << 2,
// 			
// 			DIRTY_VERTEX_BUFFER					= 1 << 4,
// 			DIRTY_INDEX_BUFFER					= 1 << 5,
// 			DIRTY_DEPTH_STENCIL					= 1 << 6,
// 			
// 			DIRTY_VS_SAMPLER					= 1 << 8,
// 			DIRTY_VS_TEXTURE					= 1 << 9,
// 			DIRTY_PS_SAMPLER					= 1 << 10,
// 			DIRTY_PS_TEXTURE					= 1 << 11,
// 			DIRTY_PS_BUFFER						= 1 << 12,
// 
// 			DIRTY_VS_CONTANT_BUFFER				= 1 << 30,
// 			DIRTY_PS_CONTANT_BUFFER				= 1 << 31,
// 
// 
// 			DIRTY_WRITE_COLOR_MASK				= 1 << 45,

		};

		enum
		{
			STACK_SIZE = 8,
		};

		inline void								fullsetDirtyFlags()
		{
			mDirtyFlag.fullset();
			for (auto i = 0; i < sce::Gnm::kShaderStageCount; i++)
			{
				mDelegateTextureDirtyFlag[i].fullset();
				mDelegateSamplerDirtyFlag[i].fullset();
			}
		}

		inline void								clearDirtyFlags()
		{
			mDirtyFlag.clear();
			for (auto i = 0; i < sce::Gnm::kShaderStageCount; i++)
			{
				mDelegateTextureDirtyFlag[i].clear();
				mDelegateSamplerDirtyFlag[i].clear();
			}
		}

		void									resetSamplerStates();

		static void								InitRSDescTable();

		void									internalSetSamplerState(SamplerStates &sampler, SamplerStateType state, U32 value);
		U32										internalGetSamplerState(const SamplerStates &sampler, SamplerStateType state) const;

		inline ResouceBinding &					getCurrentResourceBinding() { return mStatesStack[mStackLevel].mResourceBinding; }
		inline const ResouceBinding &			getCurrentResourceBinding() const { return mStatesStack[mStackLevel].mResourceBinding; }

	private:
		RenderContext *							mContext{ nullptr };

		CompleteRenderStates					mStatesStack[STACK_SIZE];
		U32										mStackLevel{ 0 };
		const CompleteRenderStates				mDefaultStates;

		Bitset									mDirtyFlag;
		Bitset									mDelegateTextureDirtyFlag[sce::Gnm::kShaderStageCount];
		Bitset									mDelegateSamplerDirtyFlag[sce::Gnm::kShaderStageCount];	// As the sampler objects need to be compiled and could be time-consuming

		// render states controls
		sce::Gnm::PrimitiveSetup				mPrimitiveSetup;
		sce::Gnm::ClipControl					mClipControl;
		sce::Gnm::DepthStencilControl			mDepthStencilControl;
		sce::Gnm::DbRenderControl				mDbRenderControl;
		sce::Gnm::StencilControl				mStencilControl;
		sce::Gnm::StencilOpControl				mStencilOpControl;
		sce::Gnm::BlendControl					mBlendControl;

		// sampler objects
		sce::Gnm::Sampler						mSamplerObjs[sce::Gnm::kShaderStageCount][MAX_NUM_SAMPLERS];

		// render targets mask
		U16										mRenderTargetsMask{ 0 };
	};
}
