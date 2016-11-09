#include "stdafx.h"

#include "RenderStateUpdateEngine.h"
#include "RenderContext.h"
#include "RenderContextChunk.h"
#include "RenderSet.h"
#include "GPUResource/GPUResourceViews.h"
#include "GPUFence.h"

using namespace sce;

Framework::RenderStateUpdateEngine::RenderStateUpdateEngine(RenderContext *owner)
	: mContext(owner)
	, mDefaultStates()
{
	mPrimitiveSetup.init();
	mClipControl.init();
	mDepthStencilControl.init();
	mDbRenderControl.init();
	mStencilControl.init();
	mStencilOpControl.init();
	mBlendControl.init();

	for (auto i = 0; i < Gnm::kShaderStageCount; i++)
	{
		for (auto j = 0; j < MAX_NUM_SAMPLERS; j++)
		{
			mSamplerObjs[i][j].init();
		}
	}

	fullsetDirtyFlags();
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
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_CULL_FACE_MODE:
		_renderStates.mCullFaceMode = (Gnm::PrimitiveSetupCullFaceMode)value;
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_FRONT_FACE:
		_renderStates.mFrontFace = (Gnm::PrimitiveSetupFrontFace)value;
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_POLYGON_OFFSET_SCALE:
		_renderStates.mPolygonOffsetScale = rawCast<Float32>(value);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
	case Framework::RS_POLYGON_OFFSET_OFFSET:
		_renderStates.mPolygonOffsetOffset = rawCast<Float32>(value);
		mDirtyFlag.set(DIRTY_PRIMITIVE_SETUP);
		break;
		// Clipping
	case Framework::RS_CLIP_SPACE:
		_renderStates.mClipSpace = (Gnm::ClipControlClipSpace)value;
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
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_WRITE_ENABLE:
		_renderStates.mDepthWriteEnable = value;
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_COMPARE_FUNC:
		_renderStates.mDepthCompareFunc = (Gnm::CompareFunc)value;
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_DEPTH_CLEAR_ENABLE:
		_renderStates.mDepthClearEnable = value;
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
	case Framework::RS_DEPTH_CLEAR_VALUE:
		_renderStates.mDepthClearValue = rawCast<Float32>(value);
		mDirtyFlag.set(DIRTY_DB_RENDER_CONTROL);
		break;
		// Stencil Test
	case Framework::RS_STENCIL_ENABLE:
		_renderStates.mStencilEnable = value;
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_COMPARE_FUNC:
		_renderStates.mStencilCompareFunc = Gnm::CompareFunc(value);
		mDirtyFlag.set(DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_TEST_VALUE:
		_renderStates.mStencilTestValue = (U8)value;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_MASK:
		_renderStates.mStencilMask = (U8)value;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_WRITE_MASK:
		_renderStates.mStencilWriteMask = (U8)value;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_OP_VALUE:
		_renderStates.mStencilOpValue = (U8)value;
		mDirtyFlag.set(DIRTY_STENCIL_CONTROL);
		break;
	case Framework::RS_STENCIL_FAIL_OP:
		_renderStates.mStencilFailOp = (Gnm::StencilOp)value;
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_ZPASS_OP:
		_renderStates.mStencilZPassOp = (Gnm::StencilOp)value;
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_ZFAIL_OP:
		_renderStates.mStencilZFailOp = (Gnm::StencilOp)value;
		mDirtyFlag.set(DIRTY_STENCIL_OP_CONTROL);
		break;
	case Framework::RS_STENCIL_CLEAR_ENABLE:
		_renderStates.mStencilClearEnable = value;
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
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_DESTMUL:
		_renderStates.mBlendDestMultiplier = (Gnm::BlendMultiplier)value;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_FUNC:
		_renderStates.mBlendFunc = (Gnm::BlendFunc)value;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_SEPARATE_ALPHA_ENABLE:
		_renderStates.mBlendSeperateAlphaEnable = value;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_SRCMUL:
		_renderStates.mBlendAlphaSrcMultiplier = (Gnm::BlendMultiplier)value;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_DESTMUL:
		_renderStates.mBlendAlphaDestMultiplier = (Gnm::BlendMultiplier)value;
		mDirtyFlag.set(DIRTY_BLEND_CONTROL);
		break;
	case Framework::RS_BLEND_ALPHA_FUNC:
		_renderStates.mBlendAlphaFunc = (Gnm::BlendFunc)value;
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

void Framework::RenderStateUpdateEngine::setSamplerStateForAllSamplers(SamplerStateType state, U32 value)
{
	for (auto shaderStage = 0; shaderStage < Gnm::kShaderStageCount; shaderStage++)
	{
		for (auto slot = 0; slot < MAX_NUM_SAMPLERS; slot++)
		{
			internalSetSamplerState(getCurrentResourceBinding().mSamplerStates[shaderStage][slot], state, value);
		}
		mDelegateSamplerDirtyFlag[shaderStage].fullset();
	}
	mDirtyFlag.set(DIRTY_SAMPLERS);
}

void Framework::RenderStateUpdateEngine::compile()
{
	// render states
	const RenderStates &_renderStates = mStatesStack[mStackLevel].mRenderStates;

	if (mDirtyFlag.get(DIRTY_PRIMITIVE_SETUP))
	{
		mPrimitiveSetup.setPolygonMode(_renderStates.mPolygonMode, _renderStates.mPolygonMode);
		mPrimitiveSetup.setCullFace(_renderStates.mCullFaceMode);
		mPrimitiveSetup.setFrontFace(_renderStates.mFrontFace);
		if (_renderStates.mPolygonOffsetScale == 0.0f && _renderStates.mPolygonOffsetOffset == 0.0f)
			mPrimitiveSetup.setPolygonOffsetEnable(sce::Gnm::kPrimitiveSetupPolygonOffsetDisable, sce::Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			mPrimitiveSetup.setPolygonOffsetEnable(sce::Gnm::kPrimitiveSetupPolygonOffsetEnable, sce::Gnm::kPrimitiveSetupPolygonOffsetEnable);
	}

	if (mDirtyFlag.get(DIRTY_CLIP_CONTROL))
	{
		mClipControl.setClipSpace(_renderStates.mClipSpace);
	}

	if (mDirtyFlag.get(DIRTY_DEPTH_STENCIL_CONTROL))
	{
		mDepthStencilControl.setDepthEnable(_renderStates.mDepthEnable);
		mDepthStencilControl.setDepthControl(_renderStates.mDepthWriteEnable ? sce::Gnm::kDepthControlZWriteEnable : sce::Gnm::kDepthControlZWriteDisable, _renderStates.mDepthCompareFunc);
		mDepthStencilControl.setStencilEnable(_renderStates.mStencilEnable);
		mDepthStencilControl.setStencilFunction(_renderStates.mStencilCompareFunc);
	}

	if (mDirtyFlag.get(DIRTY_DB_RENDER_CONTROL))
	{
		mDbRenderControl.setDepthClearEnable(_renderStates.mDepthClearEnable);
		mDbRenderControl.setStencilClearEnable(_renderStates.mStencilClearEnable);
		//mDbRenderControl.setForceDepthDecompressEnable(true);
	}

	if (mDirtyFlag.get(DIRTY_STENCIL_CONTROL))
	{
		mStencilControl.m_testVal = _renderStates.mStencilTestValue;
		mStencilControl.m_mask = _renderStates.mStencilMask;
		mStencilControl.m_writeMask = _renderStates.mStencilWriteMask;
		mStencilControl.m_opVal = _renderStates.mStencilOpValue;
	}

	if (mDirtyFlag.get(DIRTY_STENCIL_OP_CONTROL))
	{
		mStencilOpControl.setStencilOps(_renderStates.mStencilFailOp, _renderStates.mStencilZPassOp, _renderStates.mStencilZFailOp);
	}

	if (mDirtyFlag.get(DIRTY_BLEND_CONTROL))
	{
		mBlendControl.setBlendEnable(_renderStates.mBlendEnable);
		mBlendControl.setColorEquation(_renderStates.mBlendSrcMultiplier, _renderStates.mBlendFunc, _renderStates.mBlendDestMultiplier);
		mBlendControl.setSeparateAlphaEnable(_renderStates.mBlendSeperateAlphaEnable);
		mBlendControl.setAlphaEquation(_renderStates.mBlendAlphaSrcMultiplier, _renderStates.mBlendAlphaFunc, _renderStates.mBlendAlphaDestMultiplier);
	}

	// sampler states
	if (mDirtyFlag.get(DIRTY_SAMPLERS))
	{
		for (auto shaderStage = 0; shaderStage < Gnm::kShaderStageCount; shaderStage++)
		{
			for (auto slot = 0; slot < MAX_NUM_SAMPLERS; slot++)
			{
				if (mDelegateSamplerDirtyFlag[shaderStage].get(1 << slot))
				{
					Gnm::Sampler &_samplerObj = mSamplerObjs[shaderStage][slot];
					const SamplerStates &_samplerStates = getCurrentResourceBinding().mSamplerStates[shaderStage][slot];
					_samplerObj.setWrapMode(_samplerStates.mAddrModeU, _samplerStates.mAddrModeV, _samplerStates.mAddrModeW);
					_samplerObj.setBorderColor(_samplerStates.mBorderColor);
					// TODO AnisoFilter
					_samplerObj.setXyFilterMode(_samplerStates.mMagFilter, _samplerStates.mMinFilter);
					_samplerObj.setMipFilterMode(_samplerStates.mMipFilter);
					_samplerObj.setZFilterMode(_samplerStates.mZFilter);
					_samplerObj.setAnisotropyRatio(_samplerStates.mAnisotropyRatio);
				}
			}
		}
	}

	// render targets mask
	for (auto slot = 0; slot < MAX_NUM_RENDER_TARGETS; slot++)
	{
		if (getCurrentResourceBinding().mRenderTargets[slot] != nullptr)
		{
			mRenderTargetsMask |= (0xf << (slot * 4));
		}
		else
		{
			mRenderTargetsMask &= ~(0xf << (slot * 4));
		}
	}
}

void Framework::RenderStateUpdateEngine::commit()
{
	RenderContextChunk *_chunk = mContext->getCurrentChunk();
	const RenderStates &_renderStates = mStatesStack[mStackLevel].mRenderStates;

	// --- render states
	if (mDirtyFlag.get(DIRTY_PRIMITIVE_SETUP))
	{
		_chunk->setPrimitiveSetup(mPrimitiveSetup);

		_chunk->setPolygonOffsetZFormat(Gnm::kZFormat32Float); // TODO, get correct format from current depth target.
		_chunk->setPolygonOffsetFront(_renderStates.mPolygonOffsetScale, _renderStates.mPolygonOffsetOffset);
		_chunk->setPolygonOffsetBack(_renderStates.mPolygonOffsetScale, _renderStates.mPolygonOffsetOffset);
	}

	if (mDirtyFlag.get(DIRTY_CLIP_CONTROL))
	{
		_chunk->setClipControl(mClipControl);
	}

	if (mDirtyFlag.get(DIRTY_COLOR_WRITE))
	{
		_chunk->setRenderTargetMask(_renderStates.mWriteColorMask & mRenderTargetsMask);
	}

	if (mDirtyFlag.get(DIRTY_DEPTH_STENCIL_CONTROL))
	{
		_chunk->setDepthStencilControl(mDepthStencilControl);
	}

	if (mDirtyFlag.get(DIRTY_DB_RENDER_CONTROL))
	{
		_chunk->setDepthClearValue(_renderStates.mDepthClearValue);
		_chunk->setStencilClearValue(_renderStates.mStencilClearValue);
		_chunk->setDbRenderControl(mDbRenderControl);
	}

	if (mDirtyFlag.get(DIRTY_STENCIL_CONTROL))
	{
		_chunk->setStencil(mStencilControl);
	}

	if (mDirtyFlag.get(DIRTY_STENCIL_OP_CONTROL))
	{
		_chunk->setStencilOpControl(mStencilOpControl);
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
		for (U32 i = 0; i < MAX_NUM_RENDER_TARGETS; ++i)
			_chunk->setBlendControl(i, mBlendControl);
		_chunk->setBlendColor(_renderStates.mBlendColorR, _renderStates.mBlendColorG, _renderStates.mBlendColorB, _renderStates.mBlendColorA);
	}

	// --- resource binding
	ResouceBinding &_binding = getCurrentResourceBinding();
	if (mDirtyFlag.get(DIRTY_SHADERS))
	{
		_chunk->setActiveShaderStages(Gnm::kActiveShaderStagesVsPs);  // TODO fix me with real usage

		if (_binding.mShaders[Gnm::kShaderStageVs] != nullptr)
		{
			const VertexShaderView *_vs = typeCast<VertexShaderView>(_binding.mShaders[Gnm::kShaderStageVs]);
			_chunk->setVsShader(_vs->getInternalObj(), _vs->getModifier(), _vs->getFetchAddr(), _vs->getInputCache());
			_vs->getFence()->setPending(mContext->getCurrentChunk());
		}
		else
		{
			_chunk->setVsShader(nullptr, 0, nullptr, nullptr);
		}

		if (_binding.mShaders[Gnm::kShaderStagePs] != nullptr)
		{
			const PixelShaderView *_ps = typeCast<PixelShaderView>(_binding.mShaders[Gnm::kShaderStagePs]);
			_chunk->setPsShader(_ps->getInternalObj(), _ps->getInputCache());
			_ps->getFence()->setPending(mContext->getCurrentChunk());
		}
		else
		{
			_chunk->setPsShader(nullptr, nullptr);
		}

		if (_binding.mShaders[Gnm::kShaderStageCs] != nullptr)
		{
			const ComputeShaderView *_cs = typeCast<ComputeShaderView>(_binding.mShaders[Gnm::kShaderStageCs]);
			_chunk->setCsShader(_cs->getInternalObj(), _cs->getInputCache());
			_cs->getFence()->setPending(mContext->getCurrentChunk());
		}
		else
		{
			_chunk->setCsShader(nullptr, nullptr);
		}

		// TODO more shader type
	}

	if (mDirtyFlag.get(DIRTY_TEXTURES))
	{
		for (auto shaderStage = 0; shaderStage < Gnm::kShaderStageCount; shaderStage++)
		{
			if (!mDelegateTextureDirtyFlag[shaderStage].empty())
			{
				for (auto slot = 0; slot < MAX_NUM_SAMPLERS; slot++)
				{
					if (mDelegateTextureDirtyFlag[shaderStage].get(1 << slot))
					{
						TextureView *_texture = _binding.mTextures[shaderStage][slot];
						if (_texture != nullptr)
						{
							_chunk->setTextures((Gnm::ShaderStage)shaderStage, slot, 1, _texture->getInternalObj());
							_texture->getFence()->setPending(mContext->getCurrentChunk());
						}
						else
						{
							_chunk->setTextures((Gnm::ShaderStage)shaderStage, slot, 1, nullptr);
						}
					}
				}
			}
		}
	}

	if (mDirtyFlag.get(DIRTY_SAMPLERS))
	{
		for (auto shaderStage = 0; shaderStage < Gnm::kShaderStageCount; shaderStage++)
		{
			if (!mDelegateSamplerDirtyFlag[shaderStage].empty())
			{
				_chunk->setSamplers((Gnm::ShaderStage)shaderStage, 0, MAX_NUM_SAMPLERS, mSamplerObjs[shaderStage]);
			}
		}
	}

	if (mDirtyFlag.get(DIRTY_RENDER_TARGETS))
	{
		for (auto slot = 0; slot < MAX_NUM_RENDER_TARGETS; slot++)
		{
			if (_binding.mRenderTargets[slot] != nullptr)
			{
				RenderTargetView *_rt = typeCast<RenderTargetView>(_binding.mRenderTargets[slot]);
				_chunk->setRenderTarget(slot, _rt->getInternalObj());
				_rt->getFence()->setPending(mContext->getCurrentChunk());
			}
			else
			{
				_chunk->setRenderTarget(slot, nullptr);
			}
		}
	}

	if (mDirtyFlag.get(DIRTY_DEPTH_STENCIL_TARGET))
	{
		if (_binding.mDepthStencilTarget != nullptr)
		{
			DepthStencilView *_dt = typeCast<DepthStencilView>(_binding.mDepthStencilTarget);
			_chunk->setDepthRenderTarget(_dt->getInternalObj());
			_dt->getFence()->setPending(mContext->getCurrentChunk());
		}
		else
		{
			_chunk->setDepthRenderTarget(nullptr);
		}
	}

	clearDirtyFlags();
}

void Framework::RenderStateUpdateEngine::internalSetSamplerState(SamplerStates &sampler, SamplerStateType state, U32 value)
{
	switch (state)
	{
	case Framework::SS_ADDRESS_U:
		sampler.mAddrModeU = (Gnm::WrapMode)value;
		break;
	case Framework::SS_ADDRESS_V:
		sampler.mAddrModeV = (Gnm::WrapMode)value;
		break;
	case Framework::SS_ADDRESS_W:
		sampler.mAddrModeW = (Gnm::WrapMode)value;
		break;
	case Framework::SS_BORDER_COLOR:
		sampler.mBorderColor = (Gnm::BorderColor)value;
		break;
	case Framework::SS_MAG_FILTER:
		sampler.mMagFilter = (Gnm::FilterMode)value;
		break;
	case Framework::SS_MIN_FILTER:
		sampler.mMinFilter = (Gnm::FilterMode)value;
		break;
	case Framework::SS_MIP_FILTER:
		sampler.mMipFilter = (Gnm::MipFilterMode)value;
		break;
	case Framework::SS_Z_FILTER:
		sampler.mZFilter = (Gnm::ZFilterMode)value;
		break;
	case Framework::SS_ANISO_RATIO:
		sampler.mAnisotropyRatio = (Gnm::AnisotropyRatio)value;
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}
}

Framework::U32 Framework::RenderStateUpdateEngine::internalGetSamplerState(const SamplerStates &sampler, SamplerStateType state) const
{
	U32 ret = MAX_VALUE_32;
	switch (state)
	{
	case Framework::SS_ADDRESS_U:
		ret = sampler.mAddrModeU;
		break;
	case Framework::SS_ADDRESS_V:
		ret = sampler.mAddrModeV;
		break;
	case Framework::SS_ADDRESS_W:
		ret = sampler.mAddrModeW;
		break;
	case Framework::SS_BORDER_COLOR:
		ret = sampler.mBorderColor;
		break;
	case Framework::SS_MAG_FILTER:
		ret = sampler.mMagFilter;
		break;
	case Framework::SS_MIN_FILTER:
		ret = sampler.mMinFilter;
		break;
	case Framework::SS_MIP_FILTER:
		ret = sampler.mMipFilter;
		break;
	case Framework::SS_Z_FILTER:
		ret = sampler.mZFilter;
		break;
	case Framework::SS_ANISO_RATIO:
		ret = sampler.mAnisotropyRatio;
		break;
	default:
		SCE_GNM_ASSERT_MSG(false, "Not support yet");
		break;
	}
	return ret;
}
