#pragma once

namespace Framework
{
	enum RenderStateType
	{
		// --- Geometry
		// IA
		RS_POLYGON_MODE = 0,
		RS_CULL_FACE_MODE,
		RS_FRONT_FACE,
		RS_POLYGON_OFFSET_SCALE,
		RS_POLYGON_OFFSET_OFFSET,

		// Clipping
		RS_CLIP_SPACE,

		// --- Rasterizer
		// Color Write
		RS_COLOR_WRITE_0_MASK,
		RS_COLOR_WRITE_1_MASK,
		RS_COLOR_WRITE_2_MASK,
		RS_COLOR_WRITE_3_MASK,
		
		// Depth Test
		RS_DEPTH_ENABLE,
		RS_DEPTH_WRITE_ENABLE,
		RS_DEPTH_COMPARE_FUNC,
		RS_DEPTH_CLEAR_ENABLE,
		RS_DEPTH_CLEAR_VALUE,

		
		// Stencil Test
		RS_STENCIL_ENABLE,
		RS_STENCIL_COMPARE_FUNC,
		RS_STENCIL_TEST_VALUE,
		RS_STENCIL_MASK,
		RS_STENCIL_WRITE_MASK,
		RS_STENCIL_OP_VALUE,
		RS_STENCIL_FAIL_OP,
		RS_STENCIL_ZPASS_OP,
		RS_STENCIL_ZFAIL_OP,
		RS_STENCIL_CLEAR_ENABLE,
		RS_STENCIL_CLEAR_VALUE,

		// Alpha Test
		RS_ALPHA_ENABLE,
		RS_ALPHA_REF,
		RS_ALPHA_FUNC,

		// Blending
		RS_BLEND_ENABLE,
		RS_BLEND_COLOR,
		RS_BLEND_SRCMUL,
		RS_BLEND_DESTMUL,
		RS_BLEND_FUNC,
		RS_BLEND_SEPARATE_ALPHA_ENABLE,
		RS_BLEND_ALPHA_SRCMUL,
		RS_BLEND_ALPHA_DESTMUL,
		RS_BLEND_ALPHA_FUNC,

		RS_TYPE_COUNT
	};

	struct RenderStates
	{
		//										STATE							DEFAULT VALUE
		// --- Geometry
		// IA
		sce::Gnm::PrimitiveSetupPolygonMode		mPolygonMode					{ sce::Gnm::kPrimitiveSetupPolygonModeFill };
		sce::Gnm::PrimitiveSetupCullFaceMode	mCullFaceMode					{ sce::Gnm::kPrimitiveSetupCullFaceNone };
		sce::Gnm::PrimitiveSetupFrontFace		mFrontFace						{ sce::Gnm::kPrimitiveSetupFrontFaceCw };
		Float32									mPolygonOffsetScale				{ 0.0f };
		Float32									mPolygonOffsetOffset			{ 0.0f };
		// Clipping
		sce::Gnm::ClipControlClipSpace			mClipSpace						{ sce::Gnm::kClipControlClipSpaceDX };
		// --- Rasterizer
		// Color Write
		U16										mWriteColorMask					{ MAX_VALUE_16 };               // 4bits per RT
		// Depth Test
		bool									mDepthEnable					{ true };
		bool									mDepthWriteEnable				{ true };
		sce::Gnm::CompareFunc					mDepthCompareFunc				{ sce::Gnm::kCompareFuncLessEqual };
		bool									mDepthClearEnable				{ false };
		Float32									mDepthClearValue				{ 1.0f };
		// Stencil Test
		bool									mStencilEnable					{ false };
		sce::Gnm::CompareFunc					mStencilCompareFunc				{ sce::Gnm::kCompareFuncAlways };
		U8										mStencilTestValue				{ 0 };
		U8										mStencilMask					{ MAX_VALUE_8 };
		U8										mStencilWriteMask				{ MAX_VALUE_8 };
		U8										mStencilOpValue					{ 1 };
		sce::Gnm::StencilOp						mStencilFailOp					{ sce::Gnm::kStencilOpKeep };
		sce::Gnm::StencilOp						mStencilZPassOp					{ sce::Gnm::kStencilOpKeep };
		sce::Gnm::StencilOp						mStencilZFailOp					{ sce::Gnm::kStencilOpKeep };
		bool									mStencilClearEnable				{ false };
		U8										mStencilClearValue				{ 0 };
		// Alpha Test
		bool									mAlphaTestEnable				{ false };
		U8										mAlphaRef						{ 0 };
		sce::Gnm::CompareFunc					mAlphaFunc						{ sce::Gnm::kCompareFuncAlways };
		// Blending
		bool									mBlendEnable					{ false };
		U32										mBlendColor						{ 0 };
		Float32									mBlendColorR					{ 0.0f };
		Float32									mBlendColorG					{ 0.0f };
		Float32									mBlendColorB					{ 0.0f };
		Float32									mBlendColorA					{ 0.0f };
		sce::Gnm::BlendMultiplier				mBlendSrcMultiplier				{ sce::Gnm::kBlendMultiplierOne };
		sce::Gnm::BlendMultiplier				mBlendDestMultiplier			{ sce::Gnm::kBlendMultiplierZero };
		sce::Gnm::BlendFunc						mBlendFunc						{ sce::Gnm::kBlendFuncAdd };
		bool									mBlendSeperateAlphaEnable		{ false };
		sce::Gnm::BlendMultiplier				mBlendAlphaSrcMultiplier		{ sce::Gnm::kBlendMultiplierOne };
		sce::Gnm::BlendMultiplier				mBlendAlphaDestMultiplier		{ sce::Gnm::kBlendMultiplierZero };
		sce::Gnm::BlendFunc						mBlendAlphaFunc					{ sce::Gnm::kBlendFuncAdd };
	};

	enum SamplerStateType
	{
		// per texture settings
		SS_ADDRESS_U = 0,
		SS_ADDRESS_V,
		SS_ADDRESS_W,
		SS_BORDER_COLOR,
		SS_MAG_FILTER,
		SS_MIN_FILTER,
		SS_MIP_FILTER,
		SS_Z_FILTER,
		SS_ANISO_RATIO,
//		SAMP_STATE_SRGBTEXTURE,		// TODO

		// global settings
// 		SAMP_STATE_ANISOFILTER,
// 		SAMP_STATE_MAXANISOTROPY,
// 		SAMP_STATE_MIPMAPLODBIAS,

		SS_TYPE_COUNT
	};

	//											STATE							DEFAULT VALUE
	struct SamplerStates
	{
		sce::Gnm::WrapMode						mAddrModeU						{ sce::Gnm::kWrapModeWrap };
		sce::Gnm::WrapMode						mAddrModeV						{ sce::Gnm::kWrapModeWrap };
		sce::Gnm::WrapMode						mAddrModeW						{ sce::Gnm::kWrapModeWrap };
		sce::Gnm::BorderColor					mBorderColor					{ sce::Gnm::kBorderColorTransBlack };
		sce::Gnm::FilterMode					mMagFilter						{ sce::Gnm::kFilterModeBilinear };
		sce::Gnm::FilterMode					mMinFilter						{ sce::Gnm::kFilterModeBilinear };
		sce::Gnm::MipFilterMode					mMipFilter						{ sce::Gnm::kMipFilterModeLinear };
		sce::Gnm::ZFilterMode					mZFilter						{ sce::Gnm::kZFilterModeLinear };
		sce::Gnm::AnisotropyRatio				mAnisotropyRatio				{ sce::Gnm::kAnisotropyRatio1 };
	};

	class BaseShaderView;
	class TextureView;
	class BaseTargetView;
	struct ResouceBinding
	{
		// shaders
		BaseShaderView *						mShaders[sce::Gnm::kShaderStageCount];

		// samplers
		TextureView *							mTextures[sce::Gnm::kShaderStageCount][MAX_NUM_SAMPLERS];
		SamplerStates							mSamplerStates[sce::Gnm::kShaderStageCount][MAX_NUM_SAMPLERS];

		// targets
		BaseTargetView *						mRenderTargets[MAX_NUM_RENDER_TARGETS];
		BaseTargetView *						mDepthStencilTarget{ nullptr };

		ResouceBinding()
		{
			for (auto i = 0; i < sce::Gnm::kShaderStageCount; i++)
			{
				mShaders[i] = nullptr;
				for (auto j = 0; j < MAX_NUM_SAMPLERS; j++)
				{
					mTextures[i][j] = nullptr;
				}
			}

			for (auto i = 0; i < MAX_NUM_RENDER_TARGETS; i++)
			{
				mRenderTargets[i] = nullptr;
			}
		}
	};

	struct CompleteRenderStates
	{
		RenderStates							mRenderStates;
		ResouceBinding							mResourceBinding;
	};
}