#include "stdafx.h"

#include "RenderStateUpdateEngine.h"
#include "RenderContext.h"
#include "RenderContextChunk.h"
#include "RenderSet.h"

using namespace sce;

Framework::RenderStateUpdateEngine::RenderStateUpdateEngine(RenderContext *owner)
	: mContext(owner)
	, mDefaultStates()
{

}

Framework::RenderStateUpdateEngine::~RenderStateUpdateEngine()
{

}

void Framework::RenderStateUpdateEngine::setRenderState(RenderStateType state, U32 value)
{
	RenderStates &_renderStates = mStatesStack[mStackLevel].mRenderStates;
	switch (state)
	{
		// --- Geometry
		// IA
	case Framework::RS_POLYGON_MODE:
		_renderStates.mPolygonMode = (Gnm::PrimitiveSetupPolygonMode)value;
		_renderStates.mPrimitiveSetup.setPolygonMode(_renderStates.mPolygonMode, _renderStates.mPolygonMode);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_CULL_FACE_MODE:
		_renderStates.mCullFaceMode = (Gnm::PrimitiveSetupCullFaceMode)value;
		_renderStates.mPrimitiveSetup.setCullFace(_renderStates.mCullFaceMode);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_FRONT_FACE:
		_renderStates.mFrontFace = (Gnm::PrimitiveSetupFrontFace)value;
		_renderStates.mPrimitiveSetup.setFrontFace(_renderStates.mFrontFace);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_POLYGON_OFFSET_SCALE:
		_renderStates.mPolygonOffsetScale = rawCast<Float32>(value);
		if (_renderStates.mPolygonOffsetScale == 0.0f && _renderStates.mPolygonOffsetOffset == 0.0f)
			_renderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetDisable, Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			_renderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetEnable, Gnm::kPrimitiveSetupPolygonOffsetEnable);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_POLYGON_OFFSET_OFFSET:
		_renderStates.mPolygonOffsetOffset = rawCast<Float32>(value);
		if (_renderStates.mPolygonOffsetScale == 0.0f && _renderStates.mPolygonOffsetOffset == 0.0f)
			_renderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetDisable, Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			_renderStates.mPrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetEnable, Gnm::kPrimitiveSetupPolygonOffsetEnable);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
		// Clipping
	case Framework::RS_CLIP_SPACE:
		_renderStates.mClipSpace = (Gnm::ClipControlClipSpace)value;
		_renderStates.mClipControl.setClipSpace(_renderStates.mClipSpace);
		mDirtyFlag.set(DIRTY_CLIP_CONTROL);
		break;
		// --- Rasterizer
		// Color Write
	case Framework::RS_COLOR_WRITE_0_MASK:
		_renderStates.mWriteColorMask = ((_renderStates.mWriteColorMask & 0xFFF0) | (value & 0xF));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
	case Framework::RS_COLOR_WRITE_1_MASK:
		_renderStates.mWriteColorMask = ((_renderStates.mWriteColorMask & 0xFF0F) | ((value & 0xF) << 4));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
	case Framework::RS_COLOR_WRITE_2_MASK:
		_renderStates.mWriteColorMask = ((_renderStates.mWriteColorMask & 0xF0FF) | ((value & 0xF) << 8));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
	case Framework::RS_COLOR_WRITE_3_MASK:
		_renderStates.mWriteColorMask = ((_renderStates.mWriteColorMask & 0x0FFF) | ((value & 0xF) << 12));
		mDirtyFlag.set(DIRTY_COLOR_WRITE);
		break;
		// Depth Test
	case Framework::RS_DEPTH_ENABLE:
		_renderStates.mDepthEnable = value;
		_renderStates.mDepthStencilControl.setDepthEnable(_renderStates.mDepthEnable);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_WRITE_ENABLE:
		_renderStates.mDepthWriteEnable = value;
		_renderStates.mDepthStencilControl.setDepthControl(_renderStates.mDepthWriteEnable ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable, _renderStates.mDepthCompareFunc);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_COMPARE_FUNC:
		_renderStates.mDepthCompareFunc = (Gnm::CompareFunc)value;
		_renderStates.mDepthStencilControl.setDepthControl(_renderStates.mDepthWriteEnable ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable, _renderStates.mDepthCompareFunc);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_CLEAR_ENABLE:
		_renderStates.mDepthClearEnable = value;
		_renderStates.mDbRenderControl.setDepthClearEnable(_renderStates.mDepthClearEnable);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
	case Framework::RS_DEPTH_CLEAR_VALUE:
		_renderStates.mDepthClearValue = rawCast<Float32>(value);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
		// Stencil Test
	case Framework::RS_STENCIL_ENABLE:
		_renderStates.mStencilEnable = value;
		_renderStates.mDepthStencilControl.setStencilEnable(_renderStates.mStencilEnable);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_COMPARE_FUNC:
		_renderStates.mStencilCompareFunc = Gnm::CompareFunc(value);
		_renderStates.mDepthStencilControl.setStencilFunction(_renderStates.mStencilCompareFunc);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_TEST_VALUE:
		_renderStates.mStencilTestValue = (U8)value;
		_renderStates.mStencilControl.m_testVal = _renderStates.mStencilTestValue;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_MASK:
		_renderStates.mStencilMask = (U8)value;
		_renderStates.mStencilControl.m_mask = _renderStates.mStencilMask;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_WRITE_MASK:
		_renderStates.mStencilWriteMask = (U8)value;
		_renderStates.mStencilControl.m_writeMask = _renderStates.mStencilWriteMask;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_OP_VALUE:
		_renderStates.mStencilOpValue = (U8)value;
		_renderStates.mStencilControl.m_opVal = _renderStates.mStencilOpValue;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_FAIL_OP:
		_renderStates.mStencilFailOp = (Gnm::StencilOp)value;
		_renderStates.mStencilOpControl.setStencilOps(_renderStates.mStencilFailOp, _renderStates.mStencilZPassOp, _renderStates.mStencilZFailOp);
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_ZPASS_OP:
		_renderStates.mStencilZPassOp = (Gnm::StencilOp)value;
		_renderStates.mStencilOpControl.setStencilOps(_renderStates.mStencilFailOp, _renderStates.mStencilZPassOp, _renderStates.mStencilZFailOp);
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_ZFAIL_OP:
		_renderStates.mStencilZFailOp = (Gnm::StencilOp)value;
		_renderStates.mStencilOpControl.setStencilOps(_renderStates.mStencilFailOp, _renderStates.mStencilZPassOp, _renderStates.mStencilZFailOp);
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_CLEAR_ENABLE:
		_renderStates.mStencilClearEnable = value;
		_renderStates.mDbRenderControl.setStencilClearEnable(_renderStates.mStencilClearEnable);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
	case Framework::RS_STENCIL_CLEAR_VALUE:
		_renderStates.mStencilClearValue = (U8)value;
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
		// Alpha Test
	case Framework::RS_ALPHA_ENABLE:
		_renderStates.mAlphaTestEnable = value;
		mDirtyFlag.set(DIRTY_ALPHA_TEST);
		break;
	case Framework::RS_ALPHA_REF:
		_renderStates.mAlphaRef = (U8)value;
		mDirtyFlag.set(DIRTY_ALPHA_TEST);
		break;
	case Framework::RS_ALPHA_FUNC:
		_renderStates.mAlphaFunc = (Gnm::CompareFunc)value;
		mDirtyFlag.set(DIRTY_ALPHA_TEST);
		break;
		// Blending
	case Framework::RS_BLEND_ENABLE:
		_renderStates.mBlendEnable = value;
		_renderStates.mBlendControl.setBlendEnable(_renderStates.mBlendEnable);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_COLOR:
		_renderStates.mBlendColor = value;
		_renderStates.mBlendColorR = Float32((_renderStates.mBlendColor >> 24) & 0xFF) / 255.f;
		_renderStates.mBlendColorG = Float32((_renderStates.mBlendColor >> 16) & 0xFF) / 255.f;
		_renderStates.mBlendColorB = Float32((_renderStates.mBlendColor >> 8) & 0xFF) / 255.f;
		_renderStates.mBlendColorA = Float32(_renderStates.mBlendColor & 0xFF) / 255.f;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_SRCMUL:
		_renderStates.mBlendSrcMultiplier = (Gnm::BlendMultiplier)value;
		_renderStates.mBlendControl.setColorEquation(_renderStates.mBlendSrcMultiplier, _renderStates.mBlendFunc, _renderStates.mBlendDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_DESTMUL:
		_renderStates.mBlendDestMultiplier = (Gnm::BlendMultiplier)value;
		_renderStates.mBlendControl.setColorEquation(_renderStates.mBlendSrcMultiplier, _renderStates.mBlendFunc, _renderStates.mBlendDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_FUNC:
		_renderStates.mBlendFunc = (Gnm::BlendFunc)value;
		_renderStates.mBlendControl.setColorEquation(_renderStates.mBlendSrcMultiplier, _renderStates.mBlendFunc, _renderStates.mBlendDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_SEPARATE_ALPHA_ENABLE:
		_renderStates.mBlendSeperateAlphaEnable = value;
		_renderStates.mBlendControl.setSeparateAlphaEnable(_renderStates.mBlendSeperateAlphaEnable);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_SRCMUL:
		_renderStates.mBlendAlphaSrcMultiplier = (Gnm::BlendMultiplier)value;
		_renderStates.mBlendControl.setAlphaEquation(_renderStates.mBlendAlphaSrcMultiplier, _renderStates.mBlendAlphaFunc, _renderStates.mBlendAlphaDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_DESTMUL:
		_renderStates.mBlendAlphaDestMultiplier = (Gnm::BlendMultiplier)value;
		_renderStates.mBlendControl.setAlphaEquation(_renderStates.mBlendAlphaSrcMultiplier, _renderStates.mBlendAlphaFunc, _renderStates.mBlendAlphaDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_FUNC:
		_renderStates.mBlendAlphaFunc = (Gnm::BlendFunc)value;
		_renderStates.mBlendControl.setAlphaEquation(_renderStates.mBlendAlphaSrcMultiplier, _renderStates.mBlendAlphaFunc, _renderStates.mBlendAlphaDestMultiplier);
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not Support yet");
		break;
	}
}

Framework::U32 Framework::RenderStateUpdateEngine::getRenderState(RenderStateType state) const
{
	const RenderStates &_renderStates = mStatesStack[mStackLevel].mRenderStates;
	U32 ret = MAX_VALUE_32;
	switch (state)
	{
		// --- Geometry
		// IA
	case Framework::RS_POLYGON_MODE:
		ret = _renderStates.mPolygonMode;
		break;
	case Framework::RS_CULL_FACE_MODE:
		ret = _renderStates.mCullFaceMode;
		break;
	case Framework::RS_FRONT_FACE:
		ret = _renderStates.mFrontFace;
		break;
	case Framework::RS_POLYGON_OFFSET_SCALE:
		ret = rawCast<U32>(_renderStates.mPolygonOffsetScale);
		break;
	case Framework::RS_POLYGON_OFFSET_OFFSET:
		ret = rawCast<U32>(_renderStates.mPolygonOffsetOffset);
		break;
		// Clipping
	case Framework::RS_CLIP_SPACE:
		ret = _renderStates.mClipSpace;
		break;
		// --- Rasterizer
		// Color Write
	case Framework::RS_COLOR_WRITE_0_MASK:
		ret = (_renderStates.mWriteColorMask & 0xF);
		break;
	case Framework::RS_COLOR_WRITE_1_MASK:
		ret = ((_renderStates.mWriteColorMask >> 4) & 0xF);
		break;
	case Framework::RS_COLOR_WRITE_2_MASK:
		ret = ((_renderStates.mWriteColorMask >> 8) & 0xF);
		break;
	case Framework::RS_COLOR_WRITE_3_MASK:
		ret = ((_renderStates.mWriteColorMask >> 12) & 0xF);
		break;
		// Depth Test
	case Framework::RS_DEPTH_ENABLE:
		ret = _renderStates.mDepthEnable;
		break;
	case Framework::RS_DEPTH_WRITE_ENABLE:
		ret = _renderStates.mDepthWriteEnable;
		break;
	case Framework::RS_DEPTH_COMPARE_FUNC:
		ret = _renderStates.mDepthCompareFunc;
		break;
	case Framework::RS_DEPTH_CLEAR_ENABLE:
		ret = _renderStates.mDepthClearEnable;
		break;
	case Framework::RS_DEPTH_CLEAR_VALUE:
		ret = rawCast<U32>(_renderStates.mDepthClearValue);
		break;
		// Stencil Test
	case Framework::RS_STENCIL_ENABLE:
		ret = _renderStates.mStencilEnable;
		break;
	case Framework::RS_STENCIL_COMPARE_FUNC:
		ret = _renderStates.mStencilCompareFunc;
		break;
	case Framework::RS_STENCIL_TEST_VALUE:
		ret = _renderStates.mStencilTestValue;
		break;
	case Framework::RS_STENCIL_MASK:
		ret = _renderStates.mStencilMask;
		break;
	case Framework::RS_STENCIL_WRITE_MASK:
		ret = _renderStates.mStencilWriteMask;
		break;
	case Framework::RS_STENCIL_OP_VALUE:
		ret = _renderStates.mStencilOpValue;
		break;
	case Framework::RS_STENCIL_FAIL_OP:
		ret = _renderStates.mStencilFailOp;
		break;
	case Framework::RS_STENCIL_ZPASS_OP:
		ret = _renderStates.mStencilZPassOp;
		break;
	case Framework::RS_STENCIL_ZFAIL_OP:
		ret = _renderStates.mStencilZFailOp;
		break;
	case Framework::RS_STENCIL_CLEAR_ENABLE:
		ret = _renderStates.mStencilClearEnable;
		break;
	case Framework::RS_STENCIL_CLEAR_VALUE:
		ret = _renderStates.mStencilClearValue;
		break;
		// Alpha Test
	case Framework::RS_ALPHA_ENABLE:
		ret = _renderStates.mAlphaTestEnable;
		break;
	case Framework::RS_ALPHA_REF:
		ret = _renderStates.mAlphaRef;
		break;
	case Framework::RS_ALPHA_FUNC:
		ret = _renderStates.mAlphaFunc;
		break;
		// Blending
	case Framework::RS_BLEND_ENABLE:
		ret = _renderStates.mBlendEnable;
		break;
	case Framework::RS_BLEND_COLOR:
		ret = _renderStates.mBlendColor;
		break;
	case Framework::RS_BLEND_SRCMUL:
		ret = _renderStates.mBlendSrcMultiplier;
		break;
	case Framework::RS_BLEND_DESTMUL:
		ret = _renderStates.mBlendDestMultiplier;
		break;
	case Framework::RS_BLEND_FUNC:
		ret = _renderStates.mBlendFunc;
		break;
	case Framework::RS_BLEND_SEPARATE_ALPHA_ENABLE:
		ret = _renderStates.mBlendSeperateAlphaEnable;
		break;
	case Framework::RS_BLEND_ALPHA_SRCMUL:
		ret = _renderStates.mBlendAlphaSrcMultiplier;
		break;
	case Framework::RS_BLEND_ALPHA_DESTMUL:
		ret = _renderStates.mBlendAlphaDestMultiplier;
		break;
	case Framework::RS_BLEND_ALPHA_FUNC:
		ret = _renderStates.mBlendAlphaFunc;
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not Support yet");
		break;
	}

	return ret;
}

void Framework::RenderStateUpdateEngine::commit()
{
	RenderContextChunk *_chunk = mContext->getCurrentChunk();
	RenderStates &_renderStates = mStatesStack[mStackLevel].mRenderStates;

	// --- render states
	if (mDirtyFlag.get(DIRTY_PRIMITIVE_SETUP))
	{
		_chunk->setPrimitiveSetup(_renderStates.mPrimitiveSetup);

		_chunk->setPolygonOffsetZFormat(Gnm::kZFormat32Float); // TODO, get correct format from current depth target.
		_chunk->setPolygonOffsetFront(_renderStates.mPolygonOffsetScale, _renderStates.mPolygonOffsetOffset);
		_chunk->setPolygonOffsetBack(_renderStates.mPolygonOffsetScale, _renderStates.mPolygonOffsetOffset);
	}

	if (mDirtyFlag.get(DIRTY_CLIP_CONTROL))
	{
		_chunk->setClipControl(_renderStates.mClipControl);
	}

	if (mDirtyFlag.get(DIRTY_COLOR_WRITE))
	{
		U16 mRenderTargetsMask = MAX_VALUE_16; // TODO update it with real Render target setting.
		_chunk->setRenderTargetMask(_renderStates.mWriteColorMask & mRenderTargetsMask);
	}

	if (mDirtyFlag.get(DIRTY_DEPTH_STENCIL_CONTROL))
	{
		_chunk->setDepthStencilControl(_renderStates.mDepthStencilControl);
	}

	if (mDirtyFlag.get(DIRTY_DB_RENDER_CONTROL))
	{
		_chunk->setDepthClearValue(_renderStates.mDepthClearValue);
		_chunk->setStencilClearValue(_renderStates.mStencilClearValue);
		_chunk->setDbRenderControl(_renderStates.mDbRenderControl);
	}

	if (mDirtyFlag.get(DIRTY_STENCIL_CONTROL))
	{
		_chunk->setStencil(_renderStates.mStencilControl);
	}

	if (mDirtyFlag.get(DIRTY_STENCIL_OP_CONTROL))
	{
		_chunk->setStencilOpControl(_renderStates.mStencilOpControl);
	}

	if (mDirtyFlag.get(DIRTY_ALPHA_TEST))
	{
		// Parse alpha test state to constant buffer
		Float32 AlphaTestFun = 0.0f;
		Float32 ReferenceAlphaValue = _renderStates.mAlphaRef / 255.0f; // Convert U8 to Float32.
		if (_renderStates.mAlphaTestEnable) // AlphaTest enabled
		{
			if (_renderStates.mAlphaFunc == Gnm::kCompareFuncLess)
			{
				AlphaTestFun = -1.0f;
			}
			else
			{
				AlphaTestFun = 1.0f;
			}
		}
		Vector4 AlphaTestParam(ReferenceAlphaValue, AlphaTestFun, 0.0f, 0.0f);
		//GetPSConstantsBuffer()->SetVectorsF(ALPHATEST_PARAM_RESTSTER, 1, &AlphaTestParam); // TODO alpha test
		// //m_DirtyFlags.PSConstantBuffersDirty = true;
	}

	if (mDirtyFlag.get(DIRTY_BLEND_CONTROL))
	{
		for (U32 i = 0; i < RenderSet::MAX_NUM_COLOR_SURFACE; ++i)
			_chunk->setBlendControl(i, _renderStates.mBlendControl);
		_chunk->setBlendColor(_renderStates.mBlendColorR, _renderStates.mBlendColorG, _renderStates.mBlendColorB, _renderStates.mBlendColorA);
	}
}
