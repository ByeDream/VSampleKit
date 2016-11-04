#pragma once

namespace Framework
{
	class TextureView;
	class RenderStateManager
	{
	public:
		RenderStateManager();
		~RenderStateManager();

		inline void								setVertexShader(VertexShader *shader);
		inline void								setPixelShader(PixelShader *shader);

		void									setTexture(ShaderType shaderType, U32 slot, TextureView *texture);

		void									setSamplerState(ShaderType shaderType, U32 slot, SampleStateType state, SampleStateValue Value);
		void									setSamplerStateForAllSamplers(SampleStateType state, SampleStateValue Value);
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
	};
}
