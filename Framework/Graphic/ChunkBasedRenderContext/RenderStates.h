#pragma once

namespace Framework
{
	struct RenderStates
	{
		// gnm state controls
		sce::Gnm::PrimitiveSetup				mPrimitiveSetup;
		sce::Gnm::ClipControl					mClipControl;
		sce::Gnm::DbRenderControl				mDbRenderControl;
		sce::Gnm::DepthStencilControl			mDepthStencilControl;
		sce::Gnm::StencilControl				mStencilControl;
		sce::Gnm::StencilOpControl				mStencilOpControl;
		sce::Gnm::BlendControl					mBlendControl;

		sce::Gnm::PrimitiveSetupPolygonMode		mPolygonFillMode;
		sce::Gnm::PrimitiveSetupCullFaceMode	mCullFace;
		sce::Gnm::PrimitiveSetupFrontFace		mFrontFace;

		bool									mDepthWriteEnabled;
		bool									mDepthTestEnabled;
		Float32									mDepthScale;
		Float32									mDepthOffset;
		sce::Gnm::CompareFunc					mDepthCompareFunc;
		bool									mStencilEnabled;
		sce::Gnm::CompareFunc					mStencilFunc;
		sce::Gnm::StencilOp						mStencilOpFail;
		sce::Gnm::StencilOp						mStencilOpZfail;
		sce::Gnm::StencilOp						mStencilOpPass;

		bool									mBlendEnabled;
		bool									mBlendSeperateAlphaEnabled;
		sce::Gnm::BlendFunc						mBlendFuncColor;
		sce::Gnm::BlendFunc						mBlendFuncAlpha;
		sce::Gnm::BlendMultiplier				mBlendMultiplierSrcColor;
		sce::Gnm::BlendMultiplier				mBlendMultiplierDestColor;
		sce::Gnm::BlendMultiplier				mBlendMultiplierSrcAlpha;
		sce::Gnm::BlendMultiplier				mBlendMultiplierDestAlpha;
		Float32									mBlendFactorR;
		Float32									mBlendFactorG;
		Float32									mBlendFactorB;
		Float32									mBlendFactorA;
		U32										mBlendFactor;
		
		U32										mWriteColorMask;               // 4bits per RT
		bool									mDepthClearEnable;
		Float32									mDepthClearValue;
		bool									mStencilClearEnable;
		U8										mStencilClearValue;

		bool									mAlphaTestEnabled;
		U8										mAlphaRef;
		//GFX_CMP_FUNC							mAlphaFunc;

		GNMRenderStates()
			: m_ZWriteEnabled(true)
			, m_ZTestEnabled(true)
			, m_DepthScale(0.0f)
			, m_DepthOffset(0.0f)
			, m_ZCompareFunc(sce::Gnm::kCompareFuncLessEqual)
			, m_StencilOpFail(sce::Gnm::kStencilOpKeep)
			, m_StencilOpZfail(sce::Gnm::kStencilOpKeep)
			, m_StencilOpPass(sce::Gnm::kStencilOpKeep)
			, m_BlendEnabled(true)
			, m_BlendFuncColor(sce::Gnm::kBlendFuncAdd)
			, m_BlendFuncAlpha(sce::Gnm::kBlendFuncAdd)
			, m_BlendMultiplierSrcColor(sce::Gnm::kBlendMultiplierOne)
			, m_BlendMultiplierSrcAlpha(sce::Gnm::kBlendMultiplierOne)
			, m_BlendMultiplierDestColor(sce::Gnm::kBlendMultiplierZero)
			, m_BlendMultiplierDestAlpha(sce::Gnm::kBlendMultiplierZero)
			, m_BlendFactorR(0.0f)
			, m_BlendFactorG(0.0f)
			, m_BlendFactorB(0.0f)
			, m_BlendFactorA(0.0f)
			, m_BlendFactor(0xFFFFFFFF)
			, m_PolygonFillMode(sce::Gnm::kPrimitiveSetupPolygonModeFill)
			, m_CullFace(sce::Gnm::kPrimitiveSetupCullFaceBack)
			, m_FrontFace(sce::Gnm::kPrimitiveSetupFrontFaceCw)
			, m_WriteColorMask(0xFFFFFFFF)
			, m_StencilEnabled(false)
			, m_StencilFunc(sce::Gnm::kCompareFuncAlways)
			, m_DepthClearEnable(false)
			, m_StencilClearEnable(false)
			, m_DepthClearValue(0)
			, m_StencilClearValue(0)
			, m_AlphaTestEnabled(true)
			, m_AlphaRef(0)
			, m_AlphaFunc(GFX_CMP_ALWAYS)
		{
			m_BlendControl.init();
			m_PrimitiveSetup.init();
			m_DbRenderControl.init();
			m_DepthStencilControl.init();
			m_StencilControl.init();
			m_StencilOpControl.init();
			m_ClipControl.init();

			//m_DbRenderControl.setForceDepthDecompressEnable(true);
		}
	}
};