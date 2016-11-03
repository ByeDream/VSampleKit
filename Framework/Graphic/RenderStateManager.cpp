#include "stdafx.h"

#ifdef POP_PLATFORM_GNM

#include "graphic/gnm/gnmstatemanager.h"
#include "graphic/gfx/gfxresources.h" //@@LRF as I need to use black texture.
#include "graphic/neoutils.h"

using namespace sce;


popBEGIN_NAMESPACE

#define ALPHATEST_PARAM_RESTSTER 255 // This value defined in shaders/common.h

popDeclareProfile(GnmCompileStates);
popDeclareProfile(GnmCommitStates);

// @@guoxx: index by D3D_SHADER_TYPE, ensure order are correctly when add a new variable
// @@RVH: size must match GFX_SHADER_TYPE_COUNT
static const ubiU32 s_NumRegistersPerConstantBuffer[] = {
	NUM_REGISTER_PER_CONSTANTBUFFER_VS, // GFX_VERTEX_SHADER
	NUM_REGISTER_PER_CONSTANTBUFFER_PS, // GFX_PIXEL_SHADER

	NUM_REGISTER_PER_CONSTANTBUFFER_CS,
	NUM_REGISTER_PER_CONSTANTBUFFER_HS,
	NUM_REGISTER_PER_CONSTANTBUFFER_DS,
	NUM_REGISTER_PER_CONSTANTBUFFER_GS,
};
#ifndef POP_OPTIMIZED

#ifdef ENABLE_DUMP_GPU_SUBMISSION
std::queue<GNMStateManager::GPUSubmissionLog> GNMStateManager::mSubmissionLogs;
ubiU32 GNMStateManager::mBatchCounter = 0;
#endif // ENABLE_DUMP_GPU_SUBMISSION

// ubiBool GNMStateManager::ms_ForceAllStateDirty = false;
// 
// #define CHECK_DIRTY(flag)           (ms_ForceAllStateDirty || (m_DirtyFlags.flag))
// #define CHECK_DIRTY_BIT(flag, bit)  (ms_ForceAllStateDirty || (m_DirtyFlags.flag & (1 << bit)))
// 
// #else
// #define CHECK_DIRTY(flag)           (m_DirtyFlags.flag)
// #define CHECK_DIRTY_BIT(flag, bit)  (m_DirtyFlags.flag & (1 << bit))
#endif

#ifndef POP_OPTIMIZED //@@ for shader debug
bool needdebugShader = false;
ubiChar debugVSShaderHashCode[48] = "";
ubiChar debugPSShaderHashCode[48] = "F38DAF27 00000000";
#endif


#define BOOL_CHECK_DIRTY( flag) ( m_dirtyFlags & flag )
#define BOOL_CHECK_DIRTY2( dirtyFlag , flag) ( dirtyFlag & ( 1 << flag )  )

#define BOOL_SET_DIRTY( flag)	m_dirtyFlags |= flag 
#define BOOL_SET_DIRTY2( dirtyFlag , flag)	dirtyFlag |= flag 


//=========================================================================
//@@LRF default render states (static)
scimitar::GNMStateManager::GNMRenderStates scimitar::GNMStateManager::s_DefaultRenderStates;

void GNMStateManager::StaticInitialize()
{
	InitRSDescTable();
}


GNMStateManager::GNMStateManager()
{
	m_IndexSizeInBytes = 2;

	m_DefaultLODBias = 0;
	m_AnisoFilterEnabled = false;
	m_AnisotropyDegree = 1;

	m_Device = NULL;

	//@@LRF
	FullResetStates();



	for (int type = GFX_SHADER_TYPE_FIST; type < GFX_SHADER_TYPE_COUNT; ++type)
	{
		m_ConstantsBuffers[type] = popNew(GfxexConstantsBuffer, "GfxexConstantsBuffer", this)(s_NumRegistersPerConstantBuffer[type]);
	}

}

GNMStateManager::~GNMStateManager()
{
	for (int type = GFX_SHADER_TYPE_FIST; type < GFX_SHADER_TYPE_COUNT; ++type)
	{
		popDelete(m_ConstantsBuffers[type]);
	}
}

void GNMStateManager::BeginGlobalConstantsUpdate()
{
	for (int type = GFX_SHADER_TYPE_FIST; type < GFX_SHADER_TYPE_COUNT; ++type)
	{
		m_ConstantsBuffers[type]->BeginGlobalConstantsUpdate();
	}
}

void GNMStateManager::EndGlobalConstantsUpdate()
{
	for (int type = GFX_SHADER_TYPE_FIST; type < GFX_SHADER_TYPE_COUNT; ++type)
	{
		m_ConstantsBuffers[type]->EndGlobalConstantsUpdate();
	}
}

void GNMStateManager::InitRSDescTable()
{
	// Compute default RenderStates
	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE1, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE2, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE3, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE4, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE5, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE6, false);
	// 	InitializeDefaultState(GFX_RS_ALPHABLENDENABLE7, false);
	InitializeDefaultState(GFX_RS_BLENDOP, GFX_BLEND_OP_ADD);
	InitializeDefaultState(GFX_RS_SRCBLEND, GFX_BLEND_MODE_ONE);
	InitializeDefaultState(GFX_RS_DESTBLEND, GFX_BLEND_MODE_ZERO);
	InitializeDefaultState(GFX_RS_BLENDOPALPHA, GFX_BLEND_OP_ADD);
	InitializeDefaultState(GFX_RS_SRCBLENDALPHA, GFX_BLEND_MODE_ONE);
	InitializeDefaultState(GFX_RS_DESTBLENDALPHA, GFX_BLEND_MODE_ZERO);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE, GFX_COLOR_WRITE_ENABLE_ALL);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE1, GFX_COLOR_WRITE_ENABLE_ALL);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE2, GFX_COLOR_WRITE_ENABLE_ALL);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE3, GFX_COLOR_WRITE_ENABLE_ALL);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE4, 0);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE5, 0);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE6, 0);
	InitializeDefaultState(GFX_RS_COLORWRITEENABLE7, 0);

	InitializeDefaultState(GFX_RS_ZENABLE, true);
	InitializeDefaultState(GFX_RS_ZFUNC, GFX_CMP_LESSEQUAL);
	InitializeDefaultState(GFX_RS_ZWRITEENABLE, true);
	InitializeDefaultState(GFX_RS_STENCILENABLE, false);
	InitializeDefaultState(GFX_RS_STENCILWRITEMASK, AllBitsSet32);
	InitializeDefaultState(GFX_RS_STENCILFUNC, GFX_CMP_ALWAYS);
	InitializeDefaultState(GFX_RS_STENCILMASK, AllBitsSet32);
	InitializeDefaultState(GFX_RS_STENCILPASS, GFX_STENCIL_OP_KEEP);
	InitializeDefaultState(GFX_RS_STENCILFAIL, GFX_STENCIL_OP_KEEP);
	InitializeDefaultState(GFX_RS_STENCILZFAIL, GFX_STENCIL_OP_KEEP);

	InitializeDefaultState(GFX_RS_DEPTH_CLEAR_ENABLE, false);
	InitializeDefaultState(GFX_RS_DEPTH_CLEAR_VALUE, 1);
	InitializeDefaultState(GFX_RS_STENCIL_CLEAR_ENABLE, false);
	InitializeDefaultState(GFX_RS_STENCIL_CLEAR_VALUE, 0);


	InitializeDefaultState(GFX_RS_FILLMODE, GFX_FILL_SOLID);
	InitializeDefaultState(GFX_RS_CULLMODE, GFX_CULL_MODE_NONE);
	InitializeDefaultState(GFX_RS_FRONTCOUNTERCLOCKWISE, false);
	InitializeDefaultState(GFX_RS_SCISSORTESTENABLE, false);
	InitializeDefaultState(GFX_RS_DEPTHBIAS, 0);
	InitializeDefaultState(GFX_RS_SLOPESCALEDEPTHBIAS, 0);

	InitializeDefaultState(GFX_RS_ALPHATESTENABLE, false);
	InitializeDefaultState(GFX_RS_ALPHAFUNC, GFX_CMP_ALWAYS);
	InitializeDefaultState(GFX_RS_ALPHAREF, 0);
	InitializeDefaultState(GFX_RS_SEPARATEALPHABLENDENABLE, false);
	InitializeDefaultState(GFX_RS_STENCILREF, 0);
	InitializeDefaultState(GFX_RS_BLENDFACTOR, 0);
	InitializeDefaultState(GFX_RS_CLIPSPACE, sce::Gnm::kClipControlClipSpaceDX);
}

Gnm::FilterMode GNMStateManager::ConvertFilterModeToGnm(GFX_TEX_FILTER filter)
{
	switch (filter)
	{
	case GFX_TEX_FILTER_POINT:  return Gnm::kFilterModePoint;
	case GFX_TEX_FILTER_LINEAR: return Gnm::kFilterModeBilinear;
	case GFX_TEX_FILTER_ANISOTROPIC:  return Gnm::kFilterModeAnisoBilinear;
	}

	popAssert(0);
	return Gnm::kFilterModeBilinear;
}

Gnm::ZFilterMode GNMStateManager::ConvertZFilterModeToGnm(GFX_TEX_FILTER filter)
{
	switch (filter)
	{
	case GFX_TEX_FILTER_POINT:  return Gnm::kZFilterModePoint;
	case GFX_TEX_FILTER_LINEAR: return Gnm::kZFilterModeLinear;
	case GFX_TEX_FILTER_ANISOTROPIC:  return Gnm::kZFilterModeLinear; // no aniso on z
	}

	popAssert(0);
	return Gnm::kZFilterModeLinear;
}

Gnm::MipFilterMode GNMStateManager::ConvertMipFilterModeToGnm(GFX_TEX_FILTER filter)
{
	switch (filter)
	{
	case GFX_TEX_FILTER_POINT:  return Gnm::kMipFilterModePoint;
	case GFX_TEX_FILTER_ANISOTROPIC:  Gnm::kMipFilterModeLinear;
	case GFX_TEX_FILTER_LINEAR: return Gnm::kMipFilterModeLinear;
	case GFX_TEX_FILTER_NONE:  return Gnm::kMipFilterModeNone;
	}

	popAssert(0);
	return Gnm::kMipFilterModeLinear;
}

void GNMStateManager::FullResetStates()
{
	//@@LRF TODO
	ResetDefaultStates();
	ResetRenderStates();
	ResetSamplerStates();

	//@@LRF reset all of dirty flags
	SetAllDirtyFlags();
}

void scimitar::GNMStateManager::ResetDefaultStates()
{
	m_VertexShader = NULL;
	m_PixelShader = NULL;
	m_InputLayout = NULL;

	popMemSet(m_VertexBuffers, NULL, sizeof(m_VertexBuffers));
	popMemSet(&m_IndexBuffer, NULL, sizeof(m_IndexBuffer));

	popMemSet(m_RenderTargets, NULL, sizeof(m_RenderTargets));
	m_DepthTarget = NULL;

	m_RenderTargetsMask = 0;

	memset(&m_ScissorRects, 0, sizeof(m_ScissorRects));
	memset(&m_Viewports, 0, sizeof(m_Viewports));
}

void GNMStateManager::ResetRenderStates()
{
	//popStateManagerMultiThreadDetector();
	popMemCopy(&m_CompressedRenderStates, &s_DefaultRenderStates, sizeof(m_CompressedRenderStates));
	m_RenderStatesStackLevel = 0;
	ResetRenderStatesDirtyFlags();
}

void GNMStateManager::ResetSamplerStates()
{
	//popStateManagerMultiThreadDetector();

	// TODO TCARLE: Verify MaxAnisotropy. Also, compare this function with ACU (many states are different)

	// Sampler States and textures
	for (ubiU32 samp = 0; samp < popGetArraySize(m_PSSamplerStates); samp++)
	{
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_ADDRESSU, GFX_TEX_ADDRESS_WRAP);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_ADDRESSV, GFX_TEX_ADDRESS_WRAP);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_ADDRESSW, GFX_TEX_ADDRESS_WRAP);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_BORDERCOLOR, GFX_TEX_BORDER_COLOR_TRANS_BLACK);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_MAGFILTER, GFX_TEX_FILTER_LINEAR);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_MINFILTER, GFX_TEX_FILTER_LINEAR);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_MIPFILTER, GFX_TEX_FILTER_LINEAR);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_MAXANISOTROPY, m_AnisotropyDegree);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_ANISOFILTER, 0);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_MIPMAPLODBIAS, 0);
		m_PSSamplerStates[samp].SetSamplerState(GFX_SAMP_SRGBTEXTURE, 0);
	}
	m_psSamplerDirtyFlags = ((0x01 << popGetArraySize(m_PSSamplerStates)) - 1);

	for (ubiU32 samp = 0; samp < popGetArraySize(m_VSSamplerStates); samp++)
	{
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_ADDRESSU, GFX_TEX_ADDRESS_WRAP);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_ADDRESSV, GFX_TEX_ADDRESS_WRAP);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_ADDRESSW, GFX_TEX_ADDRESS_WRAP);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_BORDERCOLOR, GFX_TEX_BORDER_COLOR_TRANS_BLACK);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_MAGFILTER, GFX_TEX_FILTER_LINEAR);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_MINFILTER, GFX_TEX_FILTER_LINEAR);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_MIPFILTER, GFX_TEX_FILTER_LINEAR);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_MAXANISOTROPY, m_AnisotropyDegree);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_ANISOFILTER, 0);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_MIPMAPLODBIAS, 0);
		m_VSSamplerStates[samp].SetSamplerState(GFX_SAMP_SRGBTEXTURE, 0);
	}
	m_vsSamplerDirtyFlags = ((0x01 << popGetArraySize(m_VSSamplerStates)) - 1);

	m_defaultShaderResourceView = g_GfxResourceManager.GetPlatformGfxTexture(g_BlackTexture)->GetTextureObject();
	popAssert(m_defaultShaderResourceView != nullptr);
	popAssert(m_defaultShaderResourceView->GetGNMObject() != nullptr);

	for (ubiU32 samp = 0; samp < popGetArraySize(m_VSShaderResources); samp++)
	{
		m_VSShaderResources[samp] = *(m_defaultShaderResourceView->GetGNMObject());
		m_VSSamplerStatesObj[samp].init();
		// @@YangJun: we don't use zfilter, so zfilter should be none, other else some visible issues occur
		// but on Orbis zfilter default is point, so need fouce it to be kZFilterModeNone
		m_VSSamplerStatesObj[samp].setZFilterMode(sce::Gnm::ZFilterMode::kZFilterModeNone);
	}
	for (ubiU32 samp = 0; samp < popGetArraySize(m_PSShaderResources); samp++)
	{
		m_PSShaderResources[samp] = *(m_defaultShaderResourceView->GetGNMObject());
		m_PSSamplerStatesObj[samp].init();
		// @@YangJun: we don't use zfilter, so zfilter should be none, otherwise some visible issues occur
		// but on Orbis zfilter default is point, so need fouce it to be kZFilterModeNone
		m_PSSamplerStatesObj[samp].setZFilterMode(sce::Gnm::ZFilterMode::kZFilterModeNone);
	}
	popMemSet(m_PSBuffers, 0, sizeof(m_PSBuffers));
	m_psBufferDirtyFlags = ((0x01 << popGetArraySize(m_PSBuffers)) - 1);

#ifndef DISABLE_TEX_RT_FENCE
	popMemSet(m_VSShaderResourceFences, 0, sizeof(m_VSShaderResourceFences));
	popMemSet(m_PSShaderResourceFences, 0, sizeof(m_PSShaderResourceFences));
#endif

	BOOL_SET_DIRTY(DIRTY_VS_SAMPLER);
	BOOL_SET_DIRTY(DIRTY_VS_TEXTURE);
	BOOL_SET_DIRTY(DIRTY_PS_SAMPLER);
	BOOL_SET_DIRTY(DIRTY_PS_TEXTURE);
	BOOL_SET_DIRTY(DIRTY_PS_BUFFER);
}

// Those number are WIP - they work well for the shadow maps
// Untested with all the other interface that use them
// todo - make this a little cleaner and generic - not sure it would work well with modified value of shadow maps bias+slope

void GNMStateManager::SetCompressedRenderState(GFX_RENDER_STATE_TYPE State, ubiU32 Value, GNMRenderStates &RenderStates, ubiU32* boolDirtFlag)
{
	switch (State)
	{
		// -- Depth -------------------------------------------------------------
	case GFX_RS_ZWRITEENABLE:
		RenderStates.m_ZWriteEnabled = Value;
		RenderStates.m_DepthStencilControl.setDepthControl(RenderStates.m_ZWriteEnabled ? sce::Gnm::kDepthControlZWriteEnable : sce::Gnm::kDepthControlZWriteDisable, RenderStates.m_ZCompareFunc);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case GFX_RS_ZENABLE:
		RenderStates.m_ZTestEnabled = Value;
		RenderStates.m_DepthStencilControl.setDepthEnable(RenderStates.m_ZTestEnabled);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case GFX_RS_ZFUNC:
		//RenderStates.m_ZCompareFunc = RenderStates.m_CompareFuncToOrbis[Value];
		RenderStates.m_ZCompareFunc = (sce::Gnm::CompareFunc)Value; //@@LRF need check
		RenderStates.m_DepthStencilControl.setDepthControl(RenderStates.m_ZWriteEnabled ? sce::Gnm::kDepthControlZWriteEnable : sce::Gnm::kDepthControlZWriteDisable, RenderStates.m_ZCompareFunc);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case GFX_RS_SCISSORTESTENABLE:
		//@@LRF TODO
		break;
	case GFX_RS_DEPTH_CLEAR_ENABLE:
		RenderStates.m_DepthClearEnable = Value;
		RenderStates.m_DbRenderControl.setDepthClearEnable(RenderStates.m_DepthClearEnable);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DO_RENDER_CONTROL);
		break;
	case GFX_RS_DEPTH_CLEAR_VALUE:
		RenderStates.m_DepthClearValue = (ubiFloat)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DO_RENDER_CONTROL);
		break;
		// -- Blending ----------------------------------------------------------
	case GFX_RS_ALPHABLENDENABLE:
	case GFX_RS_ALPHABLENDENABLE1:
	case GFX_RS_ALPHABLENDENABLE2:
	case GFX_RS_ALPHABLENDENABLE3:
	case GFX_RS_ALPHABLENDENABLE4:
	case GFX_RS_ALPHABLENDENABLE5:
	case GFX_RS_ALPHABLENDENABLE6:
	case GFX_RS_ALPHABLENDENABLE7:
		RenderStates.m_BlendEnabled = Value;
		RenderStates.m_BlendControl.setBlendEnable(RenderStates.m_BlendEnabled);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_SRCBLEND:
		RenderStates.m_BlendMultiplierSrcColor = (Gnm::BlendMultiplier)Value;
		RenderStates.m_BlendControl.setColorEquation(RenderStates.m_BlendMultiplierSrcColor, RenderStates.m_BlendFuncColor, RenderStates.m_BlendMultiplierDestColor);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_DESTBLEND:
		RenderStates.m_BlendMultiplierDestColor = (Gnm::BlendMultiplier)Value;
		RenderStates.m_BlendControl.setColorEquation(RenderStates.m_BlendMultiplierSrcColor, RenderStates.m_BlendFuncColor, RenderStates.m_BlendMultiplierDestColor);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_SRCBLENDALPHA:
		RenderStates.m_BlendMultiplierSrcAlpha = (Gnm::BlendMultiplier)Value;
		RenderStates.m_BlendControl.setAlphaEquation(RenderStates.m_BlendMultiplierSrcAlpha, RenderStates.m_BlendFuncAlpha, RenderStates.m_BlendMultiplierDestAlpha);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_DESTBLENDALPHA:
		RenderStates.m_BlendMultiplierDestAlpha = (Gnm::BlendMultiplier)Value;
		RenderStates.m_BlendControl.setAlphaEquation(RenderStates.m_BlendMultiplierSrcAlpha, RenderStates.m_BlendFuncAlpha, RenderStates.m_BlendMultiplierDestAlpha);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_SEPARATEALPHABLENDENABLE:
		RenderStates.m_BlendSeperateAlphaEnabled = Value;
		RenderStates.m_BlendControl.setSeparateAlphaEnable(RenderStates.m_BlendSeperateAlphaEnabled);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_BLENDFACTOR:
		RenderStates.m_BlendFactor = Value;
		RenderStates.m_BlendFactorR = ubiFloat((Value >> 24) & 0xFF) / 255.f;
		RenderStates.m_BlendFactorG = ubiFloat((Value >> 16) & 0xFF) / 255.f;
		RenderStates.m_BlendFactorB = ubiFloat((Value >> 8) & 0xFF) / 255.f;
		RenderStates.m_BlendFactorA = ubiFloat(Value & 0xFF) / 255.f;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_BLENDOP:
		//RenderStates.m_BlendFuncColor = RenderStates.m_BlendFuncAlpha = RenderStates.m_BlendOpToGnm[Value];
		RenderStates.m_BlendFuncColor = (sce::Gnm::BlendFunc)Value; //@@LRF To be checked.
		RenderStates.m_BlendControl.setColorEquation(RenderStates.m_BlendMultiplierSrcColor, RenderStates.m_BlendFuncColor, RenderStates.m_BlendMultiplierDestColor);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_BLENDOPALPHA:
		//RenderStates.m_BlendFuncColor = RenderStates.m_BlendFuncAlpha = RenderStates.m_BlendOpToGnm[Value];
		RenderStates.m_BlendFuncAlpha = (sce::Gnm::BlendFunc)Value; //@@LRF To be checked.
		RenderStates.m_BlendControl.setAlphaEquation(RenderStates.m_BlendMultiplierSrcAlpha, RenderStates.m_BlendFuncAlpha, RenderStates.m_BlendMultiplierDestAlpha);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_BLEND_CONTROL);
		break;
	case GFX_RS_ALPHATESTENABLE:
		RenderStates.m_AlphaTestEnabled = (ubiBool)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_ALPHA_TEST);
		break;
	case GFX_RS_ALPHAREF:
		RenderStates.m_AlphaRef = (ubiU8)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_ALPHA_TEST);
		break;
	case GFX_RS_ALPHAFUNC:
		RenderStates.m_AlphaFunc = (GFX_CMP_FUNC)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_ALPHA_TEST);
		break;
		// -- PrimitiveSetup ----------------------------------------------------
	case GFX_RS_FILLMODE:
		RenderStates.m_PolygonFillMode = (Gnm::PrimitiveSetupPolygonMode)Value; //@@LRF todo check
		RenderStates.m_PrimitiveSetup.setPolygonMode(RenderStates.m_PolygonFillMode, RenderStates.m_PolygonFillMode);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_PRIMITIVE_SETUP);
		break;
	case GFX_RS_CULLMODE:
		RenderStates.m_CullFace = (Gnm::PrimitiveSetupCullFaceMode)Value;
		RenderStates.m_PrimitiveSetup.setCullFace(RenderStates.m_CullFace);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_PRIMITIVE_SETUP);
		break;
	case GFX_RS_FRONTCOUNTERCLOCKWISE:
		RenderStates.m_FrontFace = (sce::Gnm::PrimitiveSetupFrontFace)!Value; //@@LRF true means Counter-clockwise
		RenderStates.m_PrimitiveSetup.setFrontFace(RenderStates.m_FrontFace);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_PRIMITIVE_SETUP);
		break;
	case GFX_RS_DEPTHBIAS:
		RenderStates.m_DepthOffset = raw_cast<ubiFloat>(Value);
		if (RenderStates.m_DepthScale == 0.0f && RenderStates.m_DepthOffset == 0.0f)
			RenderStates.m_PrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetDisable, Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			RenderStates.m_PrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetEnable, Gnm::kPrimitiveSetupPolygonOffsetEnable);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_PRIMITIVE_SETUP);
		break;
	case GFX_RS_SLOPESCALEDEPTHBIAS:
		RenderStates.m_DepthScale = raw_cast<ubiFloat>(Value);
		if (RenderStates.m_DepthScale == 0.0f && RenderStates.m_DepthOffset == 0.0f)
			RenderStates.m_PrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetDisable, Gnm::kPrimitiveSetupPolygonOffsetDisable);
		else
			RenderStates.m_PrimitiveSetup.setPolygonOffsetEnable(Gnm::kPrimitiveSetupPolygonOffsetEnable, Gnm::kPrimitiveSetupPolygonOffsetEnable);

		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_PRIMITIVE_SETUP);
		break;
		// -- Color write mask --------------------------------------------------
	case GFX_RS_COLORWRITEENABLE:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xFFFFFFF0) | (Value & 0xF));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE1:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xFFFFFF0F) | ((Value & 0xF) << 4));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE2:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xFFFFF0FF) | ((Value & 0xF) << 8));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE3:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xFFFF0FFF) | ((Value & 0xF) << 12));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE4:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xFFF0FFFF) | ((Value & 0xF) << 16));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE5:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xFF0FFFFF) | ((Value & 0xF) << 20));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE6:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0xF0FFFFFF) | ((Value & 0xF) << 24));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
	case GFX_RS_COLORWRITEENABLE7:
		RenderStates.m_WriteColorMask = ((RenderStates.m_WriteColorMask & 0x0FFFFFFF) | ((Value & 0xF) << 28));
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_WRITE_COLOR_MASK);
		break;
		// -- Stencil -----------------------------------------------------------
	case GFX_RS_STENCILENABLE:
		RenderStates.m_StencilEnabled = Value;
		RenderStates.m_DepthStencilControl.setStencilEnable(RenderStates.m_StencilEnabled);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case GFX_RS_STENCILFUNC:
		RenderStates.m_StencilFunc = Gnm::CompareFunc(Value);
		RenderStates.m_DepthStencilControl.setStencilFunction(RenderStates.m_StencilFunc);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DEPTH_STENCIL_CONTROL);
		break;
	case GFX_RS_STENCILREF:
		RenderStates.m_StencilControl.m_testVal = Value;
		RenderStates.m_StencilControl.m_opVal = 1;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_STENCIL_CONTROL);
		break;
	case GFX_RS_STENCILMASK:
		RenderStates.m_StencilControl.m_mask = (ubiU8)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_STENCIL_CONTROL);
		break;
	case GFX_RS_STENCILWRITEMASK:
		RenderStates.m_StencilControl.m_writeMask = (ubiU8)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_STENCIL_CONTROL);
		break;
	case GFX_RS_STENCILPASS:
		RenderStates.m_StencilOpPass = (Gnm::StencilOp)Value;
		RenderStates.m_StencilOpControl.setStencilOps(RenderStates.m_StencilOpFail, RenderStates.m_StencilOpPass, RenderStates.m_StencilOpZfail);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_STENCIL_OP_CONTROL);
		break;
	case GFX_RS_STENCILFAIL:
		RenderStates.m_StencilOpFail = (Gnm::StencilOp)Value;
		RenderStates.m_StencilOpControl.setStencilOps(RenderStates.m_StencilOpFail, RenderStates.m_StencilOpPass, RenderStates.m_StencilOpZfail);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_STENCIL_OP_CONTROL);
		break;
	case GFX_RS_STENCILZFAIL:
		RenderStates.m_StencilOpZfail = (Gnm::StencilOp)Value;
		RenderStates.m_StencilOpControl.setStencilOps(RenderStates.m_StencilOpFail, RenderStates.m_StencilOpPass, RenderStates.m_StencilOpZfail);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_STENCIL_OP_CONTROL);
		break;
	case GFX_RS_STENCIL_CLEAR_ENABLE:
		RenderStates.m_StencilClearEnable = Value;
		RenderStates.m_DbRenderControl.setStencilClearEnable(RenderStates.m_StencilClearEnable);
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DO_RENDER_CONTROL);
		break;
	case GFX_RS_STENCIL_CLEAR_VALUE:
		RenderStates.m_StencilClearValue = (ubiU8)Value;
		BOOL_SET_DIRTY2(*boolDirtFlag, DIRTY_DO_RENDER_CONTROL);
		break;
	case GFX_RS_CLIPSPACE:
		RenderStates.m_ClipControl.setClipSpace((sce::Gnm::ClipControlClipSpace)Value);
		break;
	default:
		popAssert(false); // not supported yet
		break;
	}
}

void scimitar::GNMStateManager::GetCompressedRenderState(GFX_RENDER_STATE_TYPE State, const GNMRenderStates &RenderStates, ubiU32& Value)
{
	switch (State)
	{
		// -- Depth -------------------------------------------------------------
	case GFX_RS_ZWRITEENABLE:
		Value = RenderStates.m_ZWriteEnabled;
		break;
	case GFX_RS_ZENABLE:
		Value = RenderStates.m_ZTestEnabled;
		break;
	case GFX_RS_ZFUNC:
		Value = (ubiU32)RenderStates.m_ZCompareFunc;
		break;
	case GFX_RS_SCISSORTESTENABLE:
		//@@LRF TODO
		Value = 0;
		break;
	case GFX_RS_DEPTH_CLEAR_ENABLE:
		Value = RenderStates.m_DepthClearEnable;
		break;
	case GFX_RS_DEPTH_CLEAR_VALUE:
		Value = (ubiU32)RenderStates.m_DepthClearValue;
		break;
		// -- Blending ----------------------------------------------------------
	case GFX_RS_ALPHABLENDENABLE:
	case GFX_RS_ALPHABLENDENABLE1:
	case GFX_RS_ALPHABLENDENABLE2:
	case GFX_RS_ALPHABLENDENABLE3:
	case GFX_RS_ALPHABLENDENABLE4:
	case GFX_RS_ALPHABLENDENABLE5:
	case GFX_RS_ALPHABLENDENABLE6:
	case GFX_RS_ALPHABLENDENABLE7:
		Value = RenderStates.m_BlendEnabled;
		break;
	case GFX_RS_SRCBLEND:
		Value = (ubiU32)RenderStates.m_BlendMultiplierSrcColor;
		break;
	case GFX_RS_DESTBLEND:
		Value = (ubiU32)RenderStates.m_BlendMultiplierDestColor;
		break;
	case GFX_RS_SRCBLENDALPHA:
		Value = (ubiU32)RenderStates.m_BlendMultiplierSrcAlpha;
		break;
	case GFX_RS_DESTBLENDALPHA:
		Value = (ubiU32)RenderStates.m_BlendMultiplierDestAlpha;
		break;
	case GFX_RS_SEPARATEALPHABLENDENABLE:
		Value = (ubiU32)RenderStates.m_BlendSeperateAlphaEnabled;
		break;
	case GFX_RS_BLENDFACTOR:
		Value = RenderStates.m_BlendFactor;
		break;
	case GFX_RS_BLENDOP:
		Value = (ubiU32)RenderStates.m_BlendFuncColor;
		break;
	case GFX_RS_BLENDOPALPHA:
		Value = (ubiU32)RenderStates.m_BlendFuncAlpha;
		break;
	case GFX_RS_ALPHATESTENABLE:
		Value = (ubiU32)RenderStates.m_AlphaTestEnabled;
		break;
	case GFX_RS_ALPHAREF:
		Value = (ubiU32)RenderStates.m_AlphaRef;
		break;
	case GFX_RS_ALPHAFUNC:
		Value = (ubiU32)RenderStates.m_AlphaFunc;
		break;
		// -- PrimitiveSetup ----------------------------------------------------
	case GFX_RS_FILLMODE:
		Value = (ubiU32)RenderStates.m_PolygonFillMode;
		break;
	case GFX_RS_CULLMODE:
		Value = (ubiU32)RenderStates.m_CullFace;
		break;
	case GFX_RS_FRONTCOUNTERCLOCKWISE:
		Value = !RenderStates.m_FrontFace;
		break;
	case GFX_RS_DEPTHBIAS:
		Value = RenderStates.m_DepthOffset;
		break;
	case GFX_RS_SLOPESCALEDEPTHBIAS:
		Value = RenderStates.m_DepthScale;
		break;
		// -- Color write mask --------------------------------------------------
	case GFX_RS_COLORWRITEENABLE:
		Value = (RenderStates.m_WriteColorMask & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE1:
		Value = ((RenderStates.m_WriteColorMask >> 4) & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE2:
		Value = ((RenderStates.m_WriteColorMask >> 8) & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE3:
		Value = ((RenderStates.m_WriteColorMask >> 12) & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE4:
		Value = ((RenderStates.m_WriteColorMask >> 16) & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE5:
		Value = ((RenderStates.m_WriteColorMask >> 20) & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE6:
		Value = ((RenderStates.m_WriteColorMask >> 24) & 0xF);
		break;
	case GFX_RS_COLORWRITEENABLE7:
		Value = ((RenderStates.m_WriteColorMask >> 28) & 0xF);
		break;
		// -- Stencil -----------------------------------------------------------
	case GFX_RS_STENCILENABLE:
		Value = RenderStates.m_StencilEnabled;
		break;
	case GFX_RS_STENCILFUNC:
		Value = RenderStates.m_StencilFunc;
		break;
	case GFX_RS_STENCILREF:
		Value = RenderStates.m_StencilControl.m_testVal;
		break;
	case GFX_RS_STENCILMASK:
		Value = (ubiU32)RenderStates.m_StencilControl.m_mask;
		break;
	case GFX_RS_STENCILWRITEMASK:
		Value = (ubiU32)RenderStates.m_StencilControl.m_writeMask;
		break;
	case GFX_RS_STENCILPASS:
		Value = (ubiU32)RenderStates.m_StencilOpPass;
		break;
	case GFX_RS_STENCILFAIL:
		Value = (ubiU32)RenderStates.m_StencilOpFail;
		break;
	case GFX_RS_STENCILZFAIL:
		Value = (ubiU32)RenderStates.m_StencilOpZfail;
		break;
	case GFX_RS_STENCIL_CLEAR_ENABLE:
		Value = RenderStates.m_StencilClearEnable;
		break;
	case GFX_RS_STENCIL_CLEAR_VALUE:
		Value = (ubiU32)RenderStates.m_StencilClearValue;
		break;
	default:
		popAssert(false); // not supported yet
		break;
	}
}

void GNMStateManager::InitializeDefaultState(GFX_RENDER_STATE_TYPE State, ubiU32 Value)
{
	ubiU32 temp;
	SetCompressedRenderState(State, Value, s_DefaultRenderStates, &temp);
}

void GNMStateManager::InternalSetRenderState(GFX_RENDER_STATE_TYPE State, ubiU32 Value)
{
	//popStateManagerMultiThreadDetector();
	SetCompressedRenderState(State, Value, m_CompressedRenderStates, &m_dirtyFlags);
}

void scimitar::GNMStateManager::ResetRenderStatesDirtyFlags()
{
	BOOL_SET_DIRTY(DIRTY_BLEND_CONTROL);
	BOOL_SET_DIRTY(DIRTY_DEPTH_STENCIL_CONTROL);
	BOOL_SET_DIRTY(DIRTY_STENCIL_CONTROL);
	BOOL_SET_DIRTY(DIRTY_STENCIL_OP_CONTROL);
	BOOL_SET_DIRTY(DIRTY_WRITE_COLOR_MASK);
	BOOL_SET_DIRTY(DIRTY_ALPHA_TEST);
	BOOL_SET_DIRTY(DIRTY_PRIMITIVE_SETUP);
	BOOL_SET_DIRTY(DIRTY_DO_RENDER_CONTROL);
}

ubiU64 GNMStateManager::InternalSetSamplerState(ubiU32 slot, GFX_SAMPLER_STATE_TYPE Type, ubiU32 Value, GfxSamplingState& samplerStates, ubiU64 dirtyFlags)
{
	//popStateManagerMultiThreadDetector();

	if (samplerStates.SetSamplerState(Type, Value))
	{
		dirtyFlags |= 1 << slot;
	}

	return dirtyFlags;
}

void GNMStateManager::SetRenderState(GFX_RENDER_STATE_TYPE State, ubiU32 Value)
{
	//@@CK: Set internally renderstate in our U64 bitfields (called compressed rendrstate). This do nothing more.
	//popStateManagerMultiThreadDetector();

	InternalSetRenderState(State, Value);
}

ubiU32 GNMStateManager::GetRenderState(GFX_RENDER_STATE_TYPE State)
{
	//popStateManagerMultiThreadDetector();
	ubiU32 value = 0;
	GetCompressedRenderState(State, m_CompressedRenderStates, value);
	return value;
}

void GNMStateManager::PushRenderStates()
{
	//popStateManagerMultiThreadDetector();

	popAssert(m_RenderStatesStackLevel < GNMSM_RENDERSTATES_STACK);
	popMemCopy(&m_RenderStatesStack[m_RenderStatesStackLevel], &m_CompressedRenderStates, sizeof(m_CompressedRenderStates));
	// @@ChenTiao: we also need push resources
	m_RenderResourcesStack[m_RenderStatesStackLevel].m_VertexShader = m_VertexShader;
	m_RenderResourcesStack[m_RenderStatesStackLevel].m_PixelShader = m_PixelShader;
	m_RenderResourcesStack[m_RenderStatesStackLevel].mVertexDeclaration = m_InputLayout;

	m_RenderStatesStackLevel++;
}

void GNMStateManager::PopRenderStates()
{
	//popStateManagerMultiThreadDetector();

	popAssert(m_RenderStatesStackLevel > 0);
	m_RenderStatesStackLevel--;

	popMemCopy(&m_CompressedRenderStates, &m_RenderStatesStack[m_RenderStatesStackLevel], sizeof(m_CompressedRenderStates));
	ResetRenderStatesDirtyFlags();

	// @@ChenTiao pop resources
	SetVertexShader(m_RenderResourcesStack[m_RenderStatesStackLevel].m_VertexShader);
	SetPixelShader(m_RenderResourcesStack[m_RenderStatesStackLevel].m_PixelShader);
	SetVertexDeclaration(m_RenderResourcesStack[m_RenderStatesStackLevel].mVertexDeclaration);
	m_RenderResourcesStack[m_RenderStatesStackLevel].reset();
}

void GNMStateManager::GetDepthBiasValues(ubiFloat& slopeScaledDepthBias, ubiInt& depthBias)
{
	//@@RVH:function not used in GNM
	popAssert(false);

	/*
	//popStateManagerMultiThreadDetector();
	//@@LRF to be checked
	ubiFloat g_mulValueBias = 1 << 22;
	ubiFloat g_mulValueSlope = 15;
	slopeScaledDepthBias = (ubiFloat)GetRenderState(GFX_RS_SLOPESCALEDEPTHBIAS) / g_mulValueSlope;
	depthBias = GetRenderState(GFX_RS_DEPTHBIAS) / g_mulValueBias;
	*/
}

void GNMStateManager::GetSamplerStateObject(Gnm::Sampler &samplerObj, GfxSamplingState &sampler)
{
	//popStateManagerMultiThreadDetector();

	if (!m_AnisoFilterEnabled)
	{
		// reset sampler state bits for aniso filter
		sampler.SetSamplerState(GFX_SAMP_ANISOFILTER, 0);
	}

	sce::Gnm::AnisotropyRatio ratio = sce::Gnm::kAnisotropyRatio1;
	ubiU32 anisotropyDegree = sampler.m_State.MaxAnisotropy;
	if (anisotropyDegree > 1)
		ratio = sce::Gnm::kAnisotropyRatio2;
	if (anisotropyDegree > 2)
		ratio = sce::Gnm::kAnisotropyRatio4;
	if (anisotropyDegree > 4)
		ratio = sce::Gnm::kAnisotropyRatio8;
	if (anisotropyDegree > 8)
		ratio = sce::Gnm::kAnisotropyRatio16;

	if (sampler.m_State.UseAnisoFilter)
	{
		samplerObj.setXyFilterMode(Gnm::kFilterModeAnisoBilinear, Gnm::kFilterModeAnisoBilinear);    // mag min
		samplerObj.setMipFilterMode(ConvertMipFilterModeToGnm((GFX_TEX_FILTER)sampler.m_State.MipFilter));
	}
	else
	{
		samplerObj.setXyFilterMode(ConvertFilterModeToGnm((GFX_TEX_FILTER)sampler.m_State.MagFilter), ConvertFilterModeToGnm((GFX_TEX_FILTER)sampler.m_State.MinFilter));    // mag min
		samplerObj.setMipFilterMode(ConvertMipFilterModeToGnm((GFX_TEX_FILTER)sampler.m_State.MipFilter));
	}

	samplerObj.setAnisotropyRatio(ratio);

	samplerObj.setBorderColor((Gnm::BorderColor)sampler.m_State.BorderColor);

	samplerObj.setWrapMode(
		(Gnm::WrapMode)sampler.m_State.AddrModeU,
		(Gnm::WrapMode)sampler.m_State.AddrModeV,
		(Gnm::WrapMode)sampler.m_State.AddrModeW
		);
}

void GNMStateManager::CompileStates()
{
	//popProfile(GnmCompileStates);
	// Compile SamplerStates (if not really dirty, reset the flags now)
	if (BOOL_CHECK_DIRTY(DIRTY_PS_SAMPLER))
		UpdateSamplerStatesObj(m_psSamplerDirtyFlags, popGetArraySize(m_PSSamplerStatesObj), m_PSSamplerStatesObj, m_PSSamplerStates);

	if (BOOL_CHECK_DIRTY(DIRTY_VS_SAMPLER))
		UpdateSamplerStatesObj(m_vsSamplerDirtyFlags, popGetArraySize(m_VSSamplerStatesObj), m_VSSamplerStatesObj, m_VSSamplerStates);
}

void GNMStateManager::CommitStates(GnmContext* context)

{
	//popProfile(GnmCommitStates);

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
	GPUSubmissionLog currentlog;
#endif
#endif

	// Render States
	{
		if (BOOL_CHECK_DIRTY(DIRTY_PRIMITIVE_SETUP))
		{
			context->setPrimitiveSetup(m_CompressedRenderStates.m_PrimitiveSetup);
			context->setPolygonOffsetZFormat(Gnm::kZFormat32Float);

			context->setPolygonOffsetFront(m_CompressedRenderStates.m_DepthScale, m_CompressedRenderStates.m_DepthOffset);
			context->setPolygonOffsetBack(m_CompressedRenderStates.m_DepthScale, m_CompressedRenderStates.m_DepthOffset);
		}

		if (BOOL_CHECK_DIRTY(DIRTY_DO_RENDER_CONTROL))
		{
#ifdef INVERSE_DEPTH
			if (GNMDevice::GetInverseMode())
			{
				m_CompressedRenderStates.m_DepthClearValue = 1.0f - m_CompressedRenderStates.m_DepthClearValue;
			}
#endif //INVERSE_DEPTH

			context->setDepthClearValue(m_CompressedRenderStates.m_DepthClearValue);
			context->setStencilClearValue(m_CompressedRenderStates.m_StencilClearValue);
			context->setDbRenderControl(m_CompressedRenderStates.m_DbRenderControl);
		}

		if (BOOL_CHECK_DIRTY(DIRTY_DEPTH_STENCIL_CONTROL))
		{
			context->setDepthStencilControl(m_CompressedRenderStates.m_DepthStencilControl);
		}

		if (BOOL_CHECK_DIRTY(DIRTY_STENCIL_CONTROL))
		{
			context->setStencil(m_CompressedRenderStates.m_StencilControl);
		}

		if (BOOL_CHECK_DIRTY(DIRTY_STENCIL_OP_CONTROL))
		{
			context->setStencilOpControl(m_CompressedRenderStates.m_StencilOpControl);
		}

		if (BOOL_CHECK_DIRTY(DIRTY_BLEND_CONTROL))
		{
			for (ubiU32 i = 0; i < GNMDevice::max_rts; ++i)
				context->setBlendControl(i, m_CompressedRenderStates.m_BlendControl);
			context->setBlendColor(m_CompressedRenderStates.m_BlendFactorR, m_CompressedRenderStates.m_BlendFactorG, m_CompressedRenderStates.m_BlendFactorB, m_CompressedRenderStates.m_BlendFactorA);
		}

		if (BOOL_CHECK_DIRTY(DIRTY_ALPHA_TEST))
		{
			// Parse alphatest state to const buffer
			ubiFloat AlphaTestFun(0.0f);
			ubiFloat ReferenceAlphaValue = m_CompressedRenderStates.m_AlphaRef / 255.0f; // Convert ubiU8 to ubiFloat.
			if (m_CompressedRenderStates.m_AlphaTestEnabled) // AlphaTest enabled
			{
				if (m_CompressedRenderStates.m_AlphaFunc == GFX_CMP_LESS)
				{
					AlphaTestFun = -1.0f;
				}
				else
				{
					AlphaTestFun = 1.0f;
				}
			}
			ubiVector4 AlphatestParam(ReferenceAlphaValue, AlphaTestFun);
			GetPSConstantsBuffer()->SetVectorsF(ALPHATEST_PARAM_RESTSTER, 1, &AlphatestParam);
			//		m_DirtyFlags.PSConstantBuffersDirty = true;
		}
	}

	if (BOOL_CHECK_DIRTY(DIRTY_VIEWPORT))
	{
		context->setupScreenViewport(m_Viewports.X,
			m_Viewports.Y,
			m_Viewports.X + m_Viewports.Width,
			m_Viewports.Y + m_Viewports.Height,
			m_Viewports.MaxDepth - m_Viewports.MinDepth,
			m_Viewports.MinDepth);

		//=============================================================================================================
		//@@LRF currently I have no idea how to disable the scissor test for gnm, so I just set scissor rect as same as viewport when everytime the viewport size is changed
		//@@LRF should be ok for current ACR's usage (there is no function call for SetScissorRect from outside)
		//@@LRF to be removed, disable scissor test by default, only setRect when it is enabled. (as same as DX11)
		GFX_RECT t_scissorRect;
		t_scissorRect.left = m_Viewports.X;
		t_scissorRect.top = m_Viewports.Y;
		t_scissorRect.right = m_Viewports.Width + m_Viewports.X;
		t_scissorRect.bottom = m_Viewports.Height + m_Viewports.Y;

		SetScissorRect(t_scissorRect);
	}

	if (BOOL_CHECK_DIRTY(DIRTY_SCISSORRECT))
	{
		context->setScreenScissor(m_ScissorRects.left, m_ScissorRects.top, m_ScissorRects.right, m_ScissorRects.bottom);
	}

	if (BOOL_CHECK_DIRTY(DIRTY_RENDER_TARGETS) || BOOL_CHECK_DIRTY(DIRTY_WRITE_COLOR_MASK))
	{
		for (ubiU32 mrtSlot = 0; mrtSlot < GNMDevice::max_rts; ++mrtSlot)
		{
			if (!BOOL_CHECK_DIRTY2(m_renderTargetDirtyFlags, mrtSlot))
				continue;

			Gnm::RenderTarget* rt = NULL;
			if (m_RenderTargets[mrtSlot] != NULL && m_RenderTargets[mrtSlot]->GetRenderTargetView() != NULL)
			{
#ifndef DISABLE_TEX_RT_FENCE
				m_RenderTargets[mrtSlot]->GetRenderTargetView()->SetFence(context, false);
#endif
				rt = m_RenderTargets[mrtSlot]->GetRenderTargetView()->GetGNMObject();
				m_RenderTargetsMask |= (0xf << (mrtSlot * 4));
			}
			else
			{
				m_RenderTargetsMask &= ~(0xf << (mrtSlot * 4));
			}

			context->setRenderTarget(mrtSlot, rt);
		}
#ifdef GNM_USE_OBJECT_IDS
		// @@4K HACK: Enable MRT 7 when using object ids
		if (NeoUtils::GetInstance().UseObjectIds())
			m_RenderTargetsMask |= (0xf << (7 * 4));
#endif

		// don't set the color write mask if a rendertarget isn't set for that RT index...
		context->setRenderTargetMask(m_CompressedRenderStates.m_WriteColorMask & m_RenderTargetsMask);
	}

#ifndef POP_OPTIMIZED //@@HJW for shader debug
	static bool needReloadVSShader = false;
	static bool needReloadPsShader = false;
	if (needReloadVSShader)
	{
		GfxexGraphicDevice::GetInstance()->GetShaderManager().ReloadVSDebugShader();
		needReloadVSShader = false;
	}
	if (needReloadPsShader)
	{
		GfxexGraphicDevice::GetInstance()->GetShaderManager().ReloadPSDebugShader();
		needReloadPsShader = false;
	}
#endif

	//	if (CHECK_DIRTY(VertexShaderDirty))
	if (BOOL_CHECK_DIRTY(DIRTY_VERTEXSHADER))
	{
		context->setActiveShaderStages(Gnm::kActiveShaderStagesVsPs);
		context->setGsModeOff();

		if (m_VertexShader != NULL)
		{
#ifndef POP_OPTIMIZED //@@HJW for shader debug
			if (needdebugShader)
			{
				if (!strcmp(m_VertexShader->m_hashCode, debugVSShaderHashCode))
				{
					VertexShader* debugShader = GfxexGraphicDevice::GetInstance()->GetShaderManager().GetVertexShader(VS_DebugShader);
					popAssert(debugShader != NULL);
					m_VertexShader = debugShader->GetWrappedShader();
				}
			}
#endif
			context->setVsShader(m_VertexShader->GetGNMObject(), m_VertexShader->GetShaderModifier(), m_VertexShader->GetFetchShaderAddr());
#ifndef DISABLE_TEX_RT_FENCE
			m_VertexShader->SetFence(context, false);
#endif
#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
			currentlog.mVSName = m_VertexShader->GetName();
			popMemCopy(currentlog.m_hashCode_V, &(m_VertexShader->m_hashCode[0]), 48);
#endif
#endif
		}
		else
		{
			context->setVsShader(NULL, 0, (void*)NULL);
#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
			currentlog.mVSName = "NULL";
#endif
#endif
		}
	}

	//	if (CHECK_DIRTY(PixelShaderDirty))
	if (BOOL_CHECK_DIRTY(DIRTY_PIXELSHADER))
	{
		if (m_PixelShader != NULL)
		{
#ifndef POP_OPTIMIZED //@@HJW for shader debug
			if (needdebugShader)
			{
				if (!strcmp(m_PixelShader->m_hashCode, debugPSShaderHashCode))
				{
					PixelShader* debugShader = GfxexGraphicDevice::GetInstance()->GetShaderManager().GetPixelShader(PS_DebugShader);
					popAssert(debugShader != NULL);
					m_PixelShader = debugShader->GetWrappedShader();
				}
			}
#endif
			context->setPsShader(m_PixelShader->GetGNMObject());

#ifndef DISABLE_TEX_RT_FENCE
			m_PixelShader->SetFence(context, false);
#endif
#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
			currentlog.mPSName = m_PixelShader->GetName();
			popMemCopy(currentlog.m_hashCode_P, &(m_PixelShader->m_hashCode[0]), 48);
#endif
#endif
		}
		else
		{
			context->setPsShader(NULL);
#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
			currentlog.mPSName = "NULL";
#endif
#endif
		}
	}

	//if (CHECK_DIRTY(VSConstantBuffersDirty))
	{
		if (m_ConstantsBuffers[GFX_VERTEX_SHADER] != NULL)
		{
			m_ConstantsBuffers[GFX_VERTEX_SHADER]->CommitBuffer(m_Device);
			context->setConstantBuffers(Gnm::kShaderStageVs, 0, 1, m_ConstantsBuffers[GFX_VERTEX_SHADER]->GetBuffer());
			//m_ConstantsBuffers[GFX_VERTEX_SHADER]->SetFence(context, false);

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
			currentlog.mHasCommitConstantBuffer_V = true;
			const GNMWrappedResource::ConstantsBuffer::LogList &t_logs = m_ConstantsBuffers[GFX_VERTEX_SHADER]->GetLogs();
			currentlog.mNbConstant_V = t_logs.size();
			for (auto itor = t_logs.begin(); itor != t_logs.end(); itor++)
			{
				currentlog.mConstantLog_V.push_back(ConstantLog(itor->registerIndex, itor->nbRegisters, itor->dataPtr));
			}

			m_ConstantsBuffers[GFX_VERTEX_SHADER]->ClearLogs();
#endif
#endif
		}
		else
		{
			context->setConstantBuffers(Gnm::kShaderStageVs, 0, 1, NULL);
		}
	}

	//if (CHECK_DIRTY(PSConstantBuffersDirty))
	{
		if (m_ConstantsBuffers[GFX_PIXEL_SHADER] != NULL)
		{
			m_ConstantsBuffers[GFX_PIXEL_SHADER]->CommitBuffer(m_Device);
			context->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, m_ConstantsBuffers[GFX_PIXEL_SHADER]->GetBuffer());
			//m_ConstantsBuffers[GFX_PIXEL_SHADER]->SetFence(context, false);

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
			currentlog.mHasCommitConstantBuffer_P = true;
			const GNMWrappedResource::ConstantsBuffer::LogList &t_logs = m_ConstantsBuffers[GFX_PIXEL_SHADER]->GetLogs();
			currentlog.mNbConstant_P = t_logs.size();
			for (auto itor = t_logs.begin(); itor != t_logs.end(); itor++)
			{
				currentlog.mConstantLog_P.push_back(ConstantLog(itor->registerIndex, itor->nbRegisters, itor->dataPtr));
			}

			m_ConstantsBuffers[GFX_PIXEL_SHADER]->ClearLogs();
#endif
#endif
		}
		else
		{
			context->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, NULL);
		}
	}

	// Pixel shader sampler state
	if (BOOL_CHECK_DIRTY(DIRTY_PS_SAMPLER))
	{
		context->setSamplers(Gnm::kShaderStagePs, 0, GNMDevice::max_samplers, m_PSSamplerStatesObj);
	}

	if (BOOL_CHECK_DIRTY(DIRTY_PS_TEXTURE))
	{
		context->setTextures(Gnm::kShaderStagePs, 0, GNMDevice::max_samplers, m_PSShaderResources);
#ifndef DISABLE_TEX_RT_FENCE
		for (auto i = 0; i < GNMDevice::max_samplers; i++)
		{
			if (m_PSShaderResourceFences[i] != nullptr)
			{
				m_PSShaderResourceFences[i]->Set(context, false);
			}
		}
		popMemSet(m_PSShaderResourceFences, 0, sizeof(m_PSShaderResourceFences));
#endif
	}

	if (BOOL_CHECK_DIRTY(DIRTY_PS_BUFFER))
	{
		for (ubiU32 i = 0; i < GNMDevice::max_samplers; ++i)
		{
			if (!BOOL_CHECK_DIRTY2(m_psBufferDirtyFlags, i))
				continue;

			if (m_PSBuffers[i])
			{
				context->setBuffers(Gnm::kShaderStagePs, i, 1, m_PSBuffers[i]);
			}
		}
	}

	if (BOOL_CHECK_DIRTY(DIRTY_VS_SAMPLER))
	{
		context->setSamplers(Gnm::kShaderStageVs, 0, GNMDevice::max_vsamplers, m_VSSamplerStatesObj);
	}

	if (BOOL_CHECK_DIRTY(DIRTY_VS_TEXTURE))
	{
		context->setTextures(Gnm::kShaderStageVs, 0, GNMDevice::max_vsamplers, m_VSShaderResources);
#ifndef DISABLE_TEX_RT_FENCE
		for (auto i = 0; i < GNMDevice::max_vsamplers; i++)
		{
			if (m_VSShaderResourceFences[i] != nullptr)
			{
				m_VSShaderResourceFences[i]->Set(context, false);
			}
		}
		popMemSet(m_VSShaderResourceFences, 0, sizeof(m_VSShaderResourceFences));
#endif
	}

	if (BOOL_CHECK_DIRTY(DIRTY_VERTEX_STREAM))
	{
		context->setVertexBuffers(Gnm::kShaderStageVs, 0, Gnm::kSlotCountVertexBuffer, NULL);

		if (m_InputLayout != NULL)
		{
			ubiU32 numVertexStream = 0;
			// basic vertexdecl emulation
			ubiU32 numAttributes = m_InputLayout->GetElements().Size();
			for (ubiU32 elemIdx = 0; elemIdx < numAttributes; ++elemIdx)
			{
				const GNMVertexDeclarationElem& vtxElem = m_InputLayout->GetElements()[elemIdx];
				ubiU8 streamIdx = vtxElem.GetStreamIndex();
				popAssert(streamIdx < GNMDevice::max_vstreams);

				ubiU8* baseAddr = 0;
				ubiU32 numVerts = 0;
				if (m_VertexBuffers[streamIdx]._buffer != NULL)
				{
					Gear::AdaptiveLock& t_lock = m_VertexBuffers[streamIdx]._buffer->GetLock();
					popAutoLock(t_lock);

					numVertexStream += 1;

					baseAddr = (ubiU8*)m_VertexBuffers[streamIdx]._buffer->GetBuffer()->getBaseAddress() + m_VertexBuffers[streamIdx]._offset + vtxElem.GetOffset()
						+ m_Device->GetBaseVertexIndex() * m_VertexBuffers[streamIdx]._stride;
					numVerts = (m_VertexBuffers[streamIdx]._buffer->GetSizeInBytes() - m_VertexBuffers[streamIdx]._offset - m_Device->GetBaseVertexIndex() * m_VertexBuffers[streamIdx]._stride) / m_VertexBuffers[streamIdx]._stride;

					m_VertexBuffers[streamIdx]._buffer->SetFence(context, false);

					m_InputStreamBuffers[elemIdx].initAsVertexBuffer(baseAddr, vtxElem.GetFormat(), m_VertexBuffers[streamIdx]._stride, numVerts);
				}
			}

			context->setVertexBuffers(Gnm::kShaderStageVs, 0, numVertexStream, m_InputStreamBuffers);
		}
	}

	if (BOOL_CHECK_DIRTY(DIRTY_INDEX_BUFFER))
	{
		if (m_IndexBuffer._buffer != NULL)
		{
			Gear::AdaptiveLock& t_lock = m_IndexBuffer._buffer->GetLock();
			popAutoLock(t_lock);

			const GNMWrappedResource::IndexBuffer *_indexBuffer = static_cast<const GNMWrappedResource::IndexBuffer *>(m_IndexBuffer._buffer);
#ifndef POP_FINAL
			popAssert(m_IndexBuffer._format == _indexBuffer->m_Format);
#endif
			if (_indexBuffer->m_Format.m_asInt == Gnm::kDataFormatR16Uint.m_asInt)
			{
				context->setIndexSize(Gnm::kIndexSize16);
				m_IndexSizeInBytes = 2;
			}
			else
			{
				context->setIndexSize(Gnm::kIndexSize32);
				m_IndexSizeInBytes = 4;
			}

			_indexBuffer->SetFence(context, false);
		}
	}

	if (BOOL_CHECK_DIRTY(DIRTY_DEPTH_STENCIL))
	{
		Gnm::DepthRenderTarget* drt = NULL;
		if (m_DepthTarget != NULL && m_DepthTarget->GetDepthStencilView() != NULL)
		{
			drt = m_DepthTarget->GetDepthStencilView()->GetGNMObject();
#ifndef DISABLE_TEX_RT_FENCE
			m_DepthTarget->GetDepthStencilView()->SetFence(context, false);
#endif
		}

		context->setDepthRenderTarget(drt);
	}

	if (BOOL_CHECK_DIRTY(DIRTY_CLIP_CONTROL))
	{
		context->setClipControl(m_CompressedRenderStates.m_ClipControl);
	}

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
	currentlog.mBatchCount = mBatchCounter++;
	PushSubmissionLog(currentlog);
#endif
#endif

	m_dirtyFlags = 0;
	m_renderTargetDirtyFlags = 0;
	m_vsSamplerDirtyFlags = 0;
	m_psSamplerDirtyFlags = 0;
	m_psBufferDirtyFlags = 0;
}

void scimitar::GNMStateManager::SetVertexShader(const Gear::RefCountedPtr<GNMWrappedResource::VertexShader>& vs)
{
	//if( m_VertexShader != vs )
	{
		m_VertexShader = vs;
		// 	#ifndef POP_FINAL
		// 		static unsigned long breakVSHash = 0;
		//  		if ( breakVSHash != 0 && vs.IsValid() && strtoul(m_VertexShader->m_hashCode, 0, 16) == breakVSHash)
		//  		{
		//  			int i = 0;
		//  		}
		// 	#endif

		BOOL_SET_DIRTY(DIRTY_VERTEXSHADER);
	}
}

void scimitar::GNMStateManager::SetPixelShader(const Gear::RefCountedPtr<GNMWrappedResource::PixelShader>& ps)
{
	//if( m_PixelShader != ps )
	{
		m_PixelShader = ps;
		// 	#ifndef POP_FINAL
		// 		static unsigned long breakPsHash = 0;
		// 		if (breakPsHash != 0 && ps.IsValid() && strtoul(m_PixelShader->m_hashCode, 0, 16) == breakPsHash)
		// 		{
		// 			int i = 0;
		// 		}
		// 	#endif
		//	m_DirtyFlags.PixelShaderDirty = true;
		BOOL_SET_DIRTY(DIRTY_PIXELSHADER);
	}
}

void GNMStateManager::SetSamplerState(GFX_SHADER_TYPE shaderType, ubiU32 Sampler, GFX_SAMPLER_STATE_TYPE Type, ubiU32 Value)
{
	popAssert(shaderType == GFX_VERTEX_SHADER || shaderType == GFX_PIXEL_SHADER);

	switch (shaderType)
	{
	case GFX_VERTEX_SHADER:
		m_vsSamplerDirtyFlags = InternalSetSamplerState(Sampler, Type, Value, m_VSSamplerStates[Sampler], m_vsSamplerDirtyFlags);
		BOOL_SET_DIRTY(DIRTY_VS_SAMPLER);
		break;
	case GFX_PIXEL_SHADER:
		m_psSamplerDirtyFlags = InternalSetSamplerState(Sampler, Type, Value, m_PSSamplerStates[Sampler], m_psSamplerDirtyFlags);
		BOOL_SET_DIRTY(DIRTY_PS_SAMPLER);
		break;
	default:
		popAssertWithMsg(false, "Not supported yet");
	}
}

void GNMStateManager::SetSamplerStateForAllSamplers(GFX_SAMPLER_STATE_TYPE state, ubiU32 Value)
{
	for (ubiU32 samp = 0; samp < popGetArraySize(m_PSSamplerStates); ++samp)
	{
		m_PSSamplerStates[samp].SetSamplerState(state, Value);
	}
	m_psSamplerDirtyFlags = ((0x01 << GNMDevice::max_samplers) - 1);

	for (ubiU32 samp = 0; samp < popGetArraySize(m_VSSamplerStates); ++samp)
	{
		m_VSSamplerStates[samp].SetSamplerState(state, Value);
	}
	m_vsSamplerDirtyFlags = ((0x01 << GNMDevice::max_vsamplers) - 1);

	BOOL_SET_DIRTY(DIRTY_VS_SAMPLER);
	BOOL_SET_DIRTY(DIRTY_PS_SAMPLER);

}


void GNMStateManager::SetTexture(GFX_SHADER_TYPE shaderType, ubiU32 slot, GfxTextureView* texture)
{
	switch (shaderType)
	{
	case GFX_VERTEX_SHADER:
		popAssert(slot < GNMDevice::max_vsamplers);
		if (texture != nullptr)
		{
			popAssert(texture->GetGNMObject() != nullptr);
			m_VSShaderResources[slot] = *(texture->GetGNMObject());
#ifndef DISABLE_TEX_RT_FENCE
			m_VSShaderResourceFences[slot] = texture->GetFence();
#endif
		}
		else
		{
			popAssert(m_defaultShaderResourceView != nullptr);
			m_VSShaderResources[slot] = *(m_defaultShaderResourceView->GetGNMObject());
#ifndef DISABLE_TEX_RT_FENCE
			m_VSShaderResourceFences[slot] = nullptr;
#endif
		}
		BOOL_SET_DIRTY(DIRTY_VS_TEXTURE);
		break;
	case GFX_PIXEL_SHADER:
		popAssert(slot < GNMDevice::max_samplers);
		if (texture != nullptr)
		{
			popAssert(texture->GetGNMObject() != nullptr);
			m_PSShaderResources[slot] = *(texture->GetGNMObject());
#ifndef DISABLE_TEX_RT_FENCE
			m_PSShaderResourceFences[slot] = texture->GetFence();
#endif
		}
		else
		{
			popAssert(m_defaultShaderResourceView != nullptr);
			m_PSShaderResources[slot] = *(m_defaultShaderResourceView->GetGNMObject());
#ifndef DISABLE_TEX_RT_FENCE
			m_PSShaderResourceFences[slot] = nullptr;
#endif
		}
		BOOL_SET_DIRTY(DIRTY_PS_TEXTURE);
		break;
	case GFX_COMPUTE_SHADER:
	case GFX_HULL_SHADER:
	case GFX_DOMAIN_SHADER:
	case GFX_GEOMETRY_SHADER:
	default:
		popAssert(false);
		break;
	}
}

void GNMStateManager::SetBuffer(ubiU32 slot, GfxBufferView* buffer)
{
	popAssert(slot < GNMDevice::max_samplers);
	m_PSBuffers[slot] = ((buffer != nullptr) ? buffer->GetGNMObject() : nullptr);

	m_psBufferDirtyFlags |= (1 << slot);
	BOOL_SET_DIRTY(DIRTY_PS_BUFFER);
}

void GNMStateManager::SetRenderTarget(ubiU32 target, GfxexSurface* a_surface)
{
	popAssert(target < GNMDevice::max_rts);
	m_RenderTargets[target] = a_surface;

	m_renderTargetDirtyFlags |= (1 << target);
	BOOL_SET_DIRTY(DIRTY_RENDER_TARGETS);
}

void GNMStateManager::GetRenderTarget(ubiU32 target, GfxexSurface** a_surface)
{
	if (a_surface != NULL)
	{
		popAssert(target < GNMDevice::max_rts);
		*a_surface = m_RenderTargets[target];
	}
}

void GNMStateManager::SetDepthStencilTarget(GfxexSurface* a_surface)
{
	m_DepthTarget = a_surface;

	BOOL_SET_DIRTY(DIRTY_DEPTH_STENCIL);
}

void GNMStateManager::GetDepthStencilTarget(GfxexSurface** a_surface)
{
	if (a_surface != NULL)
	{
		*a_surface = m_DepthTarget;
	}
}

void GNMStateManager::SetViewport(const PlatformGfxViewport& vp)
{
	m_Viewports = vp;
	BOOL_SET_DIRTY(DIRTY_VIEWPORT);

}

void GNMStateManager::GetViewport(PlatformGfxViewport& vp)
{
	vp = m_Viewports;
}

void GNMStateManager::SetScissorRect(const GFX_RECT& rect)
{
	m_ScissorRects = rect;
	BOOL_SET_DIRTY(DIRTY_SCISSORRECT);
}

void scimitar::GNMStateManager::GetScissorRect(GFX_RECT& rect)
{
	rect = m_ScissorRects;
}

void GNMStateManager::SetVertexDeclaration(PlatformGfxInputLayout* pDecl)
{
	m_InputLayout = pDecl;

	BOOL_SET_DIRTY(DIRTY_VERTEX_STREAM);
}

PlatformGfxInputLayout* GNMStateManager::GetVertexDeclaration()
{
	return m_InputLayout;
}

void GNMStateManager::SetVertexBuffer(ubiUInt index, const PlatformGfxBaseBuffer* pBuffer, ubiUInt offset, ubiUInt stride)
{
	popAssert(index < GNMDevice::max_vstreams);
	m_VertexBuffers[index]._buffer = pBuffer;
	m_VertexBuffers[index]._offset = offset;
	m_VertexBuffers[index]._stride = stride;

	BOOL_SET_DIRTY(DIRTY_VERTEX_STREAM);
}

void GNMStateManager::SetIndexBuffer(const PlatformGfxBaseBuffer* pBuffer, GFXFORMAT fmt, ubiUInt offset)
{
	m_IndexBuffer._buffer = pBuffer;
	m_IndexBuffer._format = fmt;
	m_IndexBuffer._offset = offset;

	BOOL_SET_DIRTY(DIRTY_INDEX_BUFFER);
}

const PlatformGfxBaseBuffer * scimitar::GNMStateManager::GetIndexBuffer() const
{
	return m_IndexBuffer._buffer;
}

void GNMStateManager::SetVertexShader(VertexShader* pShader)
{
	SetVertexShader(pShader != NULL ? pShader->GetWrappedShader() : NULL);
}

void GNMStateManager::SetPixelShader(PixelShader* pShader)
{
	SetPixelShader(pShader != NULL ? pShader->GetWrappedShader() : NULL);
}

GfxexConstantsBuffer* GNMStateManager::GetVSConstantsBuffer()
{
	return m_ConstantsBuffers[GFX_VERTEX_SHADER];
}

GfxexConstantsBuffer* GNMStateManager::GetPSConstantsBuffer()
{
	return m_ConstantsBuffers[GFX_PIXEL_SHADER];
}

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
void scimitar::GNMStateManager::PushSubmissionLog(const GPUSubmissionLog &log)
{
	const int max_szie = 50;
	mSubmissionLogs.push(log);
	if (mSubmissionLogs.size() > max_szie)
	{
		mSubmissionLogs.pop();
	}
}


void scimitar::GNMStateManager::DumpSubmissionLog()
{
	//@@LRF Todo : log it in to a log file.
	LogMessage(LogGraphics, "=== @@LRF Dump GPU Submission Logs ===");
	int t_size = mSubmissionLogs.size();
	for (int i = 0; i < t_size; i++)
	{
		const GPUSubmissionLog& t_log = mSubmissionLogs.front();

		LogMessage(LogGraphics, StringFormat("  Batch : %d --------------------------", t_log.mBatchCount));
		LogMessage(LogGraphics, "    Shaders :");
		LogMessage(LogGraphics, StringFormat("      vs : %s , %s", t_log.mVSName, t_log.m_hashCode_V));
		LogMessage(LogGraphics, StringFormat("      ps : %s , %s", t_log.mPSName, t_log.m_hashCode_P));
		LogMessage(LogGraphics, " ");
		LogMessage(LogGraphics, "    Constants VS :");
		LogMessage(LogGraphics, StringFormat("      commited : %s", (t_log.mHasCommitConstantBuffer_V ? "yes" : "no")));
		LogMessage(LogGraphics, StringFormat("      number of constants : %d", t_log.mNbConstant_V));
		for (auto itor = t_log.mConstantLog_V.begin(); itor != t_log.mConstantLog_V.end(); itor++)
		{
			LogMessage(LogGraphics, StringFormat("        [registerIndex : %d, nbRegisters %d]", itor->registerIndex, itor->nbRegisters));
			if (itor->dataPtr)
			{
				for (int j = 0; j < itor->nbRegisters; j++)
				{
					const ubiVector4 &t_v = itor->dataPtr[j];
					LogMessage(LogGraphics, StringFormat("          ( %2f, %2f, %2f, %2f )", t_v[0], t_v[1], t_v[2], t_v[3]));
				}
			}
		}
		LogMessage(LogGraphics, "    Constants PS :");
		LogMessage(LogGraphics, StringFormat("      commited : %s", (t_log.mHasCommitConstantBuffer_P ? "yes" : "no")));
		LogMessage(LogGraphics, StringFormat("      number of constants : %d", t_log.mNbConstant_P));
		for (auto itor = t_log.mConstantLog_P.begin(); itor != t_log.mConstantLog_P.end(); itor++)
		{
			LogMessage(LogGraphics, StringFormat("        [registerIndex : %d, nbRegisters %d]", itor->registerIndex, itor->nbRegisters));
			if (itor->dataPtr)
			{
				for (int j = 0; j < itor->nbRegisters; j++)
				{
					const ubiVector4 &t_v = itor->dataPtr[j];
					LogMessage(LogGraphics, StringFormat("          ( %2f, %2f, %2f, %2f )", t_v[0], t_v[1], t_v[2], t_v[3]));
				}
			}
		}

		mSubmissionLogs.pop();
	}
	popAssert(mSubmissionLogs.empty());
	LogMessage(LogGraphics, "=================================");
}

GNMStateManager::ConstantLog::ConstantLog(ubiU32 _registerIndex, ubiU32 _nbRegisters, const ubiVector4* _dataPtr)
{
	registerIndex = _registerIndex;
	nbRegisters = _nbRegisters;
	ubiU32 t_dataSize = nbRegisters * sizeof(ubiVector4);
	dataPtr = static_cast<ubiVector4*>(popAlloc(t_dataSize, "ConstantLog", this));
	popMemCopy(dataPtr, _dataPtr, t_dataSize);
}

void scimitar::GNMStateManager::ConstantLog::Clear()
{
	popSafeFree(dataPtr);
}

scimitar::GNMStateManager::ConstantLog::ConstantLog(const ConstantLog& _cl)
{
	registerIndex = _cl.registerIndex;
	nbRegisters = _cl.nbRegisters;

	ubiU32 t_dataSize = nbRegisters * sizeof(ubiVector4);
	dataPtr = static_cast<ubiVector4*>(popAlloc(t_dataSize, "ConstantLog", this));
	popMemCopy(dataPtr, _cl.dataPtr, t_dataSize);
}
#endif
#endif

void GNMStateManager::SetDefaultLODBias(ubiU32 lodBias)
{
	m_DefaultLODBias = lodBias;
	SetSamplerStateForAllSamplers(GFX_SAMP_MIPMAPLODBIAS, m_DefaultLODBias);
}

void GNMStateManager::SetAnisotropicFilteringOverride(ubiBool enabled, ubiU8 anisotropyDegree)
{
	m_AnisoFilterEnabled = enabled;
	// @@guoxx: use 4 bits to store aniso filter
	m_AnisotropyDegree = Gear::Max(anisotropyDegree - 1, 0);

	if (m_AnisoFilterEnabled)
	{
		SetSamplerStateForAllSamplers(GFX_SAMP_MAXANISOTROPY, m_AnisotropyDegree);
	}
}

popEND_NAMESPACE

#endif // POP_PLATFORM_GNM
