#pragma once

#include "RenderStates.h"

namespace Framework
{
	class RenderContext;
	class TextureView;
	class RenderStateUpdateEngine
	{
	public:
		RenderStateUpdateEngine(RenderContext *owner);
		~RenderStateUpdateEngine();

		inline void								setVertexShader(const VertexShader *shader);
		inline void								setPixelShader(const PixelShader *shader);
		inline void								setComputeShader(const ComputeShaderView *shader);

		void									setTexture(ShaderType shaderType, U32 slot, TextureView *texture);

		void									setSamplerState(ShaderType shaderType, U32 slot, SampleStateType state, SampleStateValue Value);
		void									setSamplerStateForAllSamplers(SampleStateType state, SampleStateValue Value);
	
		void									setRenderTarget(U32 slot, RenderTargetView *renderTarget);
		void									SetDepthStencilTarget(DepthStencilView *a_surface);

		void SetViewport(const PlatformGfxViewport& vp);
		void SetScissorRect(const GFX_RECT& rect);

		void SetVertexDeclaration(PlatformGfxInputLayout* pDecl);
		void SetVertexBuffer(ubiUInt index, const PlatformGfxBaseBuffer* pBuffer, ubiUInt offset, ubiUInt stride);
		void SetIndexBuffer(const PlatformGfxBaseBuffer* pBuffer, GFXFORMAT fmt, ubiUInt offset);

		void SetRenderState(GFX_RENDER_STATE_TYPE state, ubiU32 value);

		// Texture filter default value overwrite
		void SetDefaultLODBias(ubiU32 lodBias);
		void SetAnisotropicFilteringOverride(ubiBool enabled, ubiU8 anisotropyDegree);

		void CompileStates();
		void CommitStates(GnmContext* context);
		void PushRenderStates();
		void PopRenderStates();
		void ResetRenderStates();

		inline ubiBool DrawCallGuard() const
		{
			ubiBool pass = true;
			// attempting to draw Color primitives without a PixelShader will cause untrackable GPU exceptions, so don't do it
			if (((m_RenderTargets[0] != NULL) && ((m_CompressedRenderStates.m_WriteColorMask & 0xf) != 0) && (m_PixelShader == NULL)))
			{
				pass = false;
			}
			//@@LRF Todo more

			return pass;
		}

		inline ubiU32 GetCurrentIndexSizeInBytes() const
		{
			return m_IndexSizeInBytes;
		}

		// initialize all dirty flags (sure not to forget any!)
		void SetAllDirtyFlags()
		{
			// reset the DirtyFlags
			m_dirtyFlags = DIRTY_ALL;
			m_psSamplerDirtyFlags = ((0x01 << popGetArraySize(m_PSSamplerStates)) - 1);
			m_vsSamplerDirtyFlags = ((0x01 << popGetArraySize(m_VSSamplerStates)) - 1);
			m_renderTargetDirtyFlags = ((0x01 << popGetArraySize(m_RenderTargets)) - 1);
			m_psBufferDirtyFlags = ((0x01 << popGetArraySize(m_PSBuffers)) - 1);
		}

		//@@LRF for render states dirty flags part only!!
		void ResetRenderStatesDirtyFlags();

		void FullResetStates();
		void ResetDefaultStates();
	private:

		


		enum DirtyFlagBit
		{
			DIRTY_VIEWPORT						= 1 << 0,
			DIRTY_SCISSORRECT					= 1 << 1,
			DIRTY_PIXELSHADER					= 1 << 2,
			DIRTY_VERTEXSHADER					= 1 << 3,
			DIRTY_VERTEX_BUFFER					= 1 << 4,
			DIRTY_INDEX_BUFFER					= 1 << 5,
			DIRTY_DEPTH_STENCIL					= 1 << 6,
			DIRTY_RENDER_TARGETS				= 1 << 7,
			DIRTY_VS_SAMPLER					= 1 << 8,
			DIRTY_VS_TEXTURE					= 1 << 9,
			DIRTY_PS_SAMPLER					= 1 << 10,
			DIRTY_PS_TEXTURE					= 1 << 11,
			DIRTY_PS_BUFFER						= 1 << 12,

			DIRTY_VS_CONTANT_BUFFER				= 1 << 30,
			DIRTY_PS_CONTANT_BUFFER				= 1 << 31,


			DIRTY_BLEND_CONTROL					= 1 << 40,
			DIRTY_DEPTH_STENCIL_CONTROL			= 1 << 41,
			DIRTY_STENCIL_CONTROL				= 1 << 42,
			DIRTY_STENCIL_OP_CONTROL			= 1 << 43,
			DIRTY_CLIP_CONTROL					= 1 << 44,
			DIRTY_WRITE_COLOR_MASK				= 1 << 45,
			DIRTY_ALPHA_TEST					= 1 << 46,
			DIRTY_PRIMITIVE_SETUP				= 1 << 47,
			DIRTY_DO_RENDER_CONTROL				= 1 << 48,

			DIRTY_ALL							= MAX_VALUE_64,
		};

		void									resetSamplerStates();

		static void InitRSDescTable();
	};
}
