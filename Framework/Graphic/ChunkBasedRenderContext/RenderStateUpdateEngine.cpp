#include "stdafx.h"

#include "RenderStateUpdateEngine.h"

using namespace sce;

void Framework::RenderStateUpdateEngine::setRenderState(RenderStateType state, U32 value)
{
	switch (state)
	{
		// --- Geometry
		// IA
	case Framework::RS_POLYGON_MODE:
		mRenderStates.mPolygonMode = (Gnm::PrimitiveSetupPolygonMode)value;
		mRenderStates.mPrimitiveSetup.setPolygonMode(mRenderStates.mPolygonMode, mRenderStates.mPolygonMode);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_CULL_FACE_MODE:
		mRenderStates.mCullFaceMode = (Gnm::PrimitiveSetupCullFaceMode)value;
		mRenderStates.mPrimitiveSetup.setCullFace(mRenderStates.mCullFaceMode);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_FRONT_FACE:
		mRenderStates.mFrontFace = (Gnm::PrimitiveSetupFrontFace)value;
		mRenderStates.mPrimitiveSetup.setFrontFace(mRenderStates.mFrontFace);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_POLYGON_OFFSET_SCALE:
		mRenderStates.mPolygonOffsetScale = rawCast<Float32>(value);
		if (mRenderStates.mPolygonOffsetScale == 0.0f && mRenderStates.mPolygonOffsetOffset == 0.0f)
			mRenderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetDisable, Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			mRenderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetEnable, Gnm::kPrimitiveSetupPolygonOffsetEnable);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_POLYGON_OFFSET_OFFSET:
		mRenderStates.mPolygonOffsetOffset = rawCast<Float32>(value);
		if (mRenderStates.mPolygonOffsetScale == 0.0f && mRenderStates.mPolygonOffsetOffset == 0.0f)
			mRenderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetDisable, Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			mRenderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetEnable, Gnm::kPrimitiveSetupPolygonOffsetEnable);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
		// Clipping
	case Framework::RS_CLIP_SPACE:
		mRenderStates.mClipSpace = (Gnm::ClipControlClipSpace)value;
		mRenderStates.mClipControl.setClipSpace(mRenderStates.mClipSpace);
		mDirtyFlag.set(DIRTY_CLIP_CONTROL);
		break;
		// --- Rasterizer
		// Color Write
	case Framework::RS_COLOR_WRITE_0_MASK:
		mRenderStates.mWriteColorMask = ((mRenderStates.mWriteColorMask & 0xFFF0) | (value & 0xF));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
	case Framework::RS_COLOR_WRITE_1_MASK:
		mRenderStates.mWriteColorMask = ((mRenderStates.mWriteColorMask & 0xFF0F) | ((value & 0xF) << 4));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
	case Framework::RS_COLOR_WRITE_2_MASK:
		mRenderStates.mWriteColorMask = ((mRenderStates.mWriteColorMask & 0xF0FF) | ((value & 0xF) << 8));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
	case Framework::RS_COLOR_WRITE_3_MASK:
		mRenderStates.mWriteColorMask = ((mRenderStates.mWriteColorMask & 0x0FFF) | ((value & 0xF) << 12));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
		// Depth Test
	case Framework::RS_DEPTH_ENABLE:
		mRenderStates.mDepthEnable = value;
		mRenderStates.mDepthStencilControl.setDepthEnable(mRenderStates.mDepthEnable);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_WRITE_ENABLE:
		mRenderStates.mDepthWriteEnable = value;
		mRenderStates.mDepthStencilControl.setDepthControl(mRenderStates.mDepthWriteEnable ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable, mRenderStates.mDepthCompareFunc);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_COMPARE_FUNC:
		mRenderStates.mDepthCompareFunc = (Gnm::CompareFunc)value;
		mRenderStates.mDepthStencilControl.setDepthControl(mRenderStates.mDepthWriteEnable ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable, mRenderStates.mDepthCompareFunc);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_CLEAR_ENABLE:
		mRenderStates.mDepthClearEnable = value;
		mRenderStates.mDbRenderControl.setDepthClearEnable(mRenderStates.mDepthClearEnable);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
	case Framework::RS_DEPTH_CLEAR_VALUE:
		mRenderStates.mDepthClearValue = rawCast<Float32>(value);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
		// Stencil Test
	case Framework::RS_STENCIL_ENABLE:
		mRenderStates.mStencilEnable = value;
		mRenderStates.mDepthStencilControl.setStencilEnable(mRenderStates.mStencilEnable);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_COMPARE_FUNC:
		mRenderStates.mStencilCompareFunc = Gnm::CompareFunc(value);
		mRenderStates.mDepthStencilControl.setStencilFunction(mRenderStates.mStencilCompareFunc);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_TEST_VALUE:
		mRenderStates.mStencilTestValue = (U8)value;
		mRenderStates.mStencilControl.m_testVal = mRenderStates.mStencilTestValue;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_MASK:
		mRenderStates.mStencilMask = (U8)value;
		mRenderStates.mStencilControl.m_mask = mRenderStates.mStencilMask;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_WRITE_MASK:
		mRenderStates.mStencilWriteMask = (U8)value;
		mRenderStates.mStencilControl.m_writeMask = mRenderStates.mStencilWriteMask;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_OP_VALUE:
		mRenderStates.mStencilOpValue = (U8)value;
		mRenderStates.mStencilControl.m_opVal = mRenderStates.mStencilOpValue;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_FAIL_OP:
		mRenderStates.mStencilFailOp = (Gnm::StencilOp)value;
		mRenderStates.mStencilOpControl.setStencilOps(mRenderStates.mStencilFailOp, mRenderStates.mStencilZPassOp, mRenderStates.mStencilZFailOp);
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_ZPASS_OP:
		mRenderStates.mStencilZPassOp = (Gnm::StencilOp)value;
		mRenderStates.mStencilOpControl.setStencilOps(mRenderStates.mStencilFailOp, mRenderStates.mStencilZPassOp, mRenderStates.mStencilZFailOp);
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_ZFAIL_OP:
		mRenderStates.mStencilZFailOp = (Gnm::StencilOp)value;
		mRenderStates.mStencilOpControl.setStencilOps(mRenderStates.mStencilFailOp, mRenderStates.mStencilZPassOp, mRenderStates.mStencilZFailOp);
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_CLEAR_ENABLE:
		mRenderStates.mStencilClearEnable = value;
		mRenderStates.mDbRenderControl.setStencilClearEnable(mRenderStates.mStencilClearEnable);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
	case Framework::RS_STENCIL_CLEAR_VALUE:
		mRenderStates.mStencilClearValue = (U8)value;
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
		// Alpha Test
	case Framework::RS_ALPHA_ENABLE:
		mRenderStates.mAlphaTestEnabled = value;
		mDirtyFlag.set(DIRTY_ALPHA_TEST);
		break;
	case Framework::RS_ALPHA_REF:
		mRenderStates.mAlphaRef = (U8)value;
		mDirtyFlag.set(DIRTY_ALPHA_TEST);
		break;
	case Framework::RS_ALPHA_FUNC:
		mRenderStates.mAlphaFunc = (Gnm::CompareFunc)value;
		mDirtyFlag.set(DIRTY_ALPHA_TEST);
		break;
		// Blending
	case Framework::RS_BLEND_ENABLE:
		mRenderStates.mBlendEnable = value;
		mRenderStates.mBlendControl.setBlendEnable(mRenderStates.mBlendEnable);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_COLOR:
		mRenderStates.mBlendColor = value;
		mRenderStates.mBlendColorR = Float32((mRenderStates.mBlendColor >> 24) & 0xFF) / 255.f;
		mRenderStates.mBlendColorG = Float32((mRenderStates.mBlendColor >> 16) & 0xFF) / 255.f;
		mRenderStates.mBlendColorB = Float32((mRenderStates.mBlendColor >> 8) & 0xFF) / 255.f;
		mRenderStates.mBlendColorA = Float32(mRenderStates.mBlendColor & 0xFF) / 255.f;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_SRCMUL:
		mRenderStates.mBlendSrcMultiplier = (Gnm::BlendMultiplier)value;
		mRenderStates.mBlendControl.setColorEquation(mRenderStates.mBlendSrcMultiplier, mRenderStates.mBlendFunc, mRenderStates.mBlendDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_DESTMUL:
		mRenderStates.mBlendDestMultiplier = (Gnm::BlendMultiplier)value;
		mRenderStates.mBlendControl.setColorEquation(mRenderStates.mBlendSrcMultiplier, mRenderStates.mBlendFunc, mRenderStates.mBlendDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_FUNC:
		mRenderStates.mBlendFunc = (Gnm::BlendFunc)value;
		mRenderStates.mBlendControl.setColorEquation(mRenderStates.mBlendSrcMultiplier, mRenderStates.mBlendFunc, mRenderStates.mBlendDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_SEPARATE_ALPHA_ENABLE:
		mRenderStates.mBlendSeperateAlphaEnable = value;
		mRenderStates.mBlendControl.setSeparateAlphaEnable(mRenderStates.mBlendSeperateAlphaEnable);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_SRCMUL:
		mRenderStates.mBlendAlphaSrcMultiplier = (Gnm::BlendMultiplier)value;
		mRenderStates.mBlendControl.setAlphaEquation(mRenderStates.mBlendAlphaSrcMultiplier, mRenderStates.mBlendAlphaFunc, mRenderStates.mBlendAlphaDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_DESTMUL:
		mRenderStates.mBlendAlphaDestMultiplier = (Gnm::BlendMultiplier)value;
		mRenderStates.mBlendControl.setAlphaEquation(mRenderStates.mBlendAlphaSrcMultiplier, mRenderStates.mBlendAlphaFunc, mRenderStates.mBlendAlphaDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_FUNC:
		mRenderStates.mBlendAlphaFunc = (Gnm::BlendFunc)value;
		mRenderStates.mBlendControl.setAlphaEquation(mRenderStates.mBlendAlphaSrcMultiplier, mRenderStates.mBlendAlphaFunc, mRenderStates.mBlendAlphaDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not Support yet");
		break;
	}
}

Framework::U32 Framework::RenderStateUpdateEngine::getRenderState(RenderStateType state) const
{
	U32 ret = MAX_VALUE_32;
	switch (state)
	{
		// --- Geometry
		// IA
	case Framework::RS_POLYGON_MODE:
		ret = mRenderStates.mPolygonMode;
		break;
	case Framework::RS_CULL_FACE_MODE:
		ret = mRenderStates.mCullFaceMode;
		break;
	case Framework::RS_FRONT_FACE:
		ret = mRenderStates.mFrontFace;
		break;
	case Framework::RS_POLYGON_OFFSET_SCALE:
		ret = rawCast<U32>(mRenderStates.mPolygonOffsetScale);
		break;
	case Framework::RS_POLYGON_OFFSET_OFFSET:
		ret = rawCast<U32>(mRenderStates.mPolygonOffsetOffset);
		break;
		// Clipping
	case Framework::RS_CLIP_SPACE:
		ret = mRenderStates.mClipSpace;
		break;
		// --- Rasterizer
		// Color Write
	case Framework::RS_COLOR_WRITE_0_MASK:
		ret = (mRenderStates.mWriteColorMask & 0xF);
		break;
	case Framework::RS_COLOR_WRITE_1_MASK:
		ret = ((mRenderStates.mWriteColorMask >> 4) & 0xF);
		break;
	case Framework::RS_COLOR_WRITE_2_MASK:
		ret = ((mRenderStates.mWriteColorMask >> 8) & 0xF);
		break;
	case Framework::RS_COLOR_WRITE_3_MASK:
		ret = ((mRenderStates.mWriteColorMask >> 12) & 0xF);
		break;
		// Depth Test
	case Framework::RS_DEPTH_ENABLE:
		ret = mRenderStates.mDepthEnable;
		break;
	case Framework::RS_DEPTH_WRITE_ENABLE:
		ret = mRenderStates.mDepthWriteEnable;
		break;
	case Framework::RS_DEPTH_COMPARE_FUNC:
		ret = mRenderStates.mDepthCompareFunc;
		break;
	case Framework::RS_DEPTH_CLEAR_ENABLE:
		ret = mRenderStates.mDepthClearEnable;
		break;
	case Framework::RS_DEPTH_CLEAR_VALUE:
		ret = rawCast<U32>(mRenderStates.mDepthClearValue);
		break;
		// Stencil Test
	case Framework::RS_STENCIL_ENABLE:
		ret = mRenderStates.mStencilEnable;
		break;
	case Framework::RS_STENCIL_COMPARE_FUNC:
		ret = mRenderStates.mStencilCompareFunc;
		break;
	case Framework::RS_STENCIL_TEST_VALUE:
		ret = mRenderStates.mStencilTestValue;
		break;
	case Framework::RS_STENCIL_MASK:
		ret = mRenderStates.mStencilMask;
		break;
	case Framework::RS_STENCIL_WRITE_MASK:
		ret = mRenderStates.mStencilWriteMask;
		break;
	case Framework::RS_STENCIL_OP_VALUE:
		ret = mRenderStates.mStencilOpValue;
		break;
	case Framework::RS_STENCIL_FAIL_OP:
		ret = mRenderStates.mStencilFailOp;
		break;
	case Framework::RS_STENCIL_ZPASS_OP:
		ret = mRenderStates.mStencilZPassOp;
		break;
	case Framework::RS_STENCIL_ZFAIL_OP:
		ret = mRenderStates.mStencilZFailOp;
		break;
	case Framework::RS_STENCIL_CLEAR_ENABLE:
		ret = mRenderStates.mStencilClearEnable;
		break;
	case Framework::RS_STENCIL_CLEAR_VALUE:
		ret = mRenderStates.mStencilClearValue;
		break;
		// Alpha Test
	case Framework::RS_ALPHA_ENABLE:
		ret = mRenderStates.mAlphaTestEnabled;
		break;
	case Framework::RS_ALPHA_REF:
		ret = mRenderStates.mAlphaRef;
		break;
	case Framework::RS_ALPHA_FUNC:
		ret = mRenderStates.mAlphaFunc;
		break;
		// Blending
	case Framework::RS_BLEND_ENABLE:
		ret = mRenderStates.mBlendEnable;
		break;
	case Framework::RS_BLEND_COLOR:
		ret = mRenderStates.mBlendColor;
		break;
	case Framework::RS_BLEND_SRCMUL:
		ret = mRenderStates.mBlendSrcMultiplier;
		break;
	case Framework::RS_BLEND_DESTMUL:
		ret = mRenderStates.mBlendDestMultiplier;
		break;
	case Framework::RS_BLEND_FUNC:
		ret = mRenderStates.mBlendFunc;
		break;
	case Framework::RS_BLEND_SEPARATE_ALPHA_ENABLE:
		ret = mRenderStates.mBlendSeperateAlphaEnable;
		break;
	case Framework::RS_BLEND_ALPHA_SRCMUL:
		ret = mRenderStates.mBlendAlphaSrcMultiplier;
		break;
	case Framework::RS_BLEND_ALPHA_DESTMUL:
		ret = mRenderStates.mBlendAlphaDestMultiplier;
		break;
	case Framework::RS_BLEND_ALPHA_FUNC:
		ret = mRenderStates.mBlendAlphaFunc;
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not Support yet");
		break;
	}

	return ret;
}
