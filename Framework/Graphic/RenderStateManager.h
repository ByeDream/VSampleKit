#pragma once

namespace Framework
{
	class RenderStateManager
	{
	public:
		RenderStateManager();
		~RenderStateManager();

	};
}


#include "graphic/gfx/gfxExtension/gfxexdefinitions.h"

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
#include <queue>
#endif
#endif

popBEGIN_NAMESPACE

class GNMStateManager
{
public:
	static const ubiU32 AllBitsSet32 = 0xffffffff;
	static const ubiU64 AllBitsSet64 = 0xffffffffffffffff;


	// The different Dirty Falgs
	enum EDirtyFlags
	{
		DIRTY_VIEWPORT = 1 << 0,
		DIRTY_SCISSORRECT = 1 << 1,
		DIRTY_PIXELSHADER = 1 << 2,
		DIRTY_VERTEXSHADER = 1 << 3,
		DIRTY_VERTEX_STREAM = 1 << 4,
		DIRTY_INDEX_BUFFER = 1 << 5,
		DIRTY_DEPTH_STENCIL = 1 << 6,

		DIRTY_RENDER_TARGETS = 1 << 7,
		DIRTY_VS_SAMPLER = 1 << 8,
		DIRTY_VS_TEXTURE = 1 << 9,
		DIRTY_PS_SAMPLER = 1 << 10,
		DIRTY_PS_TEXTURE = 1 << 11,
		DIRTY_PS_BUFFER = 1 << 12,

		DIRTY_VS_CONTANT_BUFFER = 1 << 15,
		DIRTY_PS_CONTANT_BUFFER = 1 << 16,


		DIRTY_BLEND_CONTROL = 1 << 20,
		DIRTY_DEPTH_STENCIL_CONTROL = 1 << 21,
		DIRTY_STENCIL_CONTROL = 1 << 22,
		DIRTY_STENCIL_OP_CONTROL = 1 << 23,

		DIRTY_CLIP_CONTROL = 1 << 24,
		DIRTY_WRITE_COLOR_MASK = 1 << 25,
		DIRTY_ALPHA_TEST = 1 << 26,

		DIRTY_PRIMITIVE_SETUP = 1 << 27,
		DIRTY_DO_RENDER_CONTROL = 1 << 28,

		DIRTY_ALL = AllBitsSet32,
	};



	GNMStateManager();
	~GNMStateManager();

	void BeginGlobalConstantsUpdate();
	void EndGlobalConstantsUpdate();

	void ResetSamplerStates();

	// Shaders

	void SetVertexShader(VertexShader* pShader);
	Gear::RefCountedPtr<GNMWrappedResource::VertexShader> GetVertexShader() { return m_VertexShader; }
	void SetPixelShader(PixelShader* pShader);

	void SetVertexShader(const Gear::RefCountedPtr<GNMWrappedResource::VertexShader>& vs);
	void SetPixelShader(const Gear::RefCountedPtr<GNMWrappedResource::PixelShader>& ps);

	GfxexConstantsBuffer* GetVSConstantsBuffer();
	GfxexConstantsBuffer* GetPSConstantsBuffer();

	// Sampler states
	void SetSamplerState(GFX_SHADER_TYPE shaderType, ubiU32 Sampler, GFX_SAMPLER_STATE_TYPE state, ubiU32 Value);
	void SetSamplerStateForAllSamplers(GFX_SAMPLER_STATE_TYPE state, ubiU32 Value);

	// Textures
	void SetTexture(GFX_SHADER_TYPE shaderType, ubiU32 slot, GfxTextureView* texture);
	// hack used by craa
	void SetBuffer(ubiU32 slot, GfxBufferView* buffer);

	// Render targets
	void SetRenderTarget(ubiU32 target, GfxexSurface* rtView);
	void GetRenderTarget(ubiU32 target, GfxexSurface** rtView);

	void SetDepthStencilTarget(GfxexSurface* a_surface);
	void GetDepthStencilTarget(GfxexSurface** a_surface);

	// Viewports
	void SetViewport(const PlatformGfxViewport& vp);
	void GetViewport(PlatformGfxViewport& vp);
	void SetScissorRect(const GFX_RECT& rect);
	void GetScissorRect(GFX_RECT& rect);

	inline const PlatformGfxViewport& GetViewport() { return m_Viewports; }

	// Vertex stream
	void SetVertexDeclaration(PlatformGfxInputLayout* pDecl);
	PlatformGfxInputLayout* GetVertexDeclaration();
	void SetVertexBuffer(ubiUInt index, const PlatformGfxBaseBuffer* pBuffer, ubiUInt offset, ubiUInt stride);
	void SetIndexBuffer(const PlatformGfxBaseBuffer* pBuffer, GFXFORMAT fmt, ubiUInt offset);
	const PlatformGfxBaseBuffer *GetIndexBuffer() const;

	// Render States
	void CompileStates();
	void CommitStates(GnmContext* context);
	void SetRenderState(GFX_RENDER_STATE_TYPE state, ubiU32 value);
	ubiU32 GetRenderState(GFX_RENDER_STATE_TYPE State);
	void PushRenderStates();
	void PopRenderStates();
	void ResetRenderStates();

	// Texture filter default value overwrite
	void SetDefaultLODBias(ubiU32 lodBias);
	void SetAnisotropicFilteringOverride(ubiBool enabled, ubiU8 anisotropyDegree);

	void GetSamplerStateObject(sce::Gnm::Sampler &samplerObj, GfxSamplingState &sampler);

	void SetGNMDevice(GNMDevice* device) { m_Device = device; };

	// Compressed Render states
	static void StaticInitialize();
	static void InitRSDescTable();

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

	static sce::Gnm::FilterMode ConvertFilterModeToGnm(GFX_TEX_FILTER filter);
	static sce::Gnm::ZFilterMode ConvertZFilterModeToGnm(GFX_TEX_FILTER filter);
	static sce::Gnm::MipFilterMode ConvertMipFilterModeToGnm(GFX_TEX_FILTER filter);

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
	//@@LRF gnm RenderStates =================================================================
	struct GNMRenderStates
	{
		sce::Gnm::BlendControl              m_BlendControl;
		sce::Gnm::PrimitiveSetup            m_PrimitiveSetup;
		sce::Gnm::DbRenderControl           m_DbRenderControl;
		sce::Gnm::DepthStencilControl       m_DepthStencilControl;
		sce::Gnm::StencilControl            m_StencilControl;
		sce::Gnm::StencilOpControl          m_StencilOpControl;
		sce::Gnm::ClipControl               m_ClipControl;

		ubiBool								m_ZWriteEnabled;
		ubiBool								m_ZTestEnabled;
		ubiFloat                            m_DepthScale;
		ubiFloat                            m_DepthOffset;
		sce::Gnm::CompareFunc               m_ZCompareFunc;
		sce::Gnm::StencilOp					m_StencilOpFail;
		sce::Gnm::StencilOp					m_StencilOpZfail;
		sce::Gnm::StencilOp					m_StencilOpPass;

		ubiBool								m_BlendEnabled;
		ubiBool								m_BlendSeperateAlphaEnabled;
		sce::Gnm::BlendFunc                 m_BlendFuncColor;
		sce::Gnm::BlendFunc                 m_BlendFuncAlpha;
		sce::Gnm::BlendMultiplier           m_BlendMultiplierSrcColor;
		sce::Gnm::BlendMultiplier           m_BlendMultiplierDestColor;
		sce::Gnm::BlendMultiplier           m_BlendMultiplierSrcAlpha;
		sce::Gnm::BlendMultiplier           m_BlendMultiplierDestAlpha;
		ubiFloat                            m_BlendFactorR;
		ubiFloat                            m_BlendFactorG;
		ubiFloat                            m_BlendFactorB;
		ubiFloat                            m_BlendFactorA;
		ubiU32								m_BlendFactor;

		sce::Gnm::PrimitiveSetupPolygonMode	m_PolygonFillMode;
		sce::Gnm::PrimitiveSetupCullFaceMode m_CullFace;
		sce::Gnm::PrimitiveSetupFrontFace	m_FrontFace;
		ubiU32                              m_WriteColorMask;               // 4bits per RT

		ubiBool								m_StencilEnabled;
		sce::Gnm::CompareFunc				m_StencilFunc;

		ubiBool								m_DepthClearEnable;
		ubiBool								m_StencilClearEnable;
		ubiFloat							m_DepthClearValue;
		ubiU8								m_StencilClearValue;

		ubiBool								m_AlphaTestEnabled;
		ubiU8								m_AlphaRef;
		GFX_CMP_FUNC						m_AlphaFunc;

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
	};

	struct GNMRenderResources
	{
		Gear::RefCountedPtr<GNMWrappedResource::VertexShader> m_VertexShader;
		Gear::RefCountedPtr<GNMWrappedResource::PixelShader>  m_PixelShader;

		PlatformGfxInputLayout* mVertexDeclaration;

		GNMRenderResources()
		{
			reset();
		}

		void reset()
		{
			m_VertexShader = 0;
			m_PixelShader = 0;
			mVertexDeclaration = 0;
		}
	};

	void InternalSetRenderState(GFX_RENDER_STATE_TYPE State, ubiU32 Value);
	ubiU64 InternalSetSamplerState(ubiU32 slot, GFX_SAMPLER_STATE_TYPE Type, ubiU32 Value, GfxSamplingState& samplerStates, ubiU64 dirtyFlags);

	static void InitializeDefaultState(GFX_RENDER_STATE_TYPE State, ubiU32 Value);

	void GetDepthBiasValues(ubiFloat& slopeScaledDepthBias, ubiInt& depthBias);

	inline void UpdateSamplerStatesObj(ubiU64 dirtyFlags, ubiU32 arraySize, sce::Gnm::Sampler samplerStatesObj[], GfxSamplingState samplerStates[])
	{
		for (ubiU32 sampler = 0; sampler < arraySize; sampler++)
		{
			if ((dirtyFlags & (1 << sampler)) != 0)
			{
				GetSamplerStateObject(samplerStatesObj[sampler], samplerStates[sampler]);
			}
		}
	}

	// @@guoxx: TODO use dirty flag to indicate is necessary to update rendering states
	// for the moment, do everything in stupid way

	struct VertexBufferDesc
	{
		const PlatformGfxBaseBuffer*	_buffer;
		ubiU32		_offset;
		ubiU32		_stride;
	};

	struct IndexBufferDesc
	{
		const PlatformGfxBaseBuffer*	_buffer;
		GFXFORMAT	_format;
		ubiU32		_offset;
	};

	Gear::RefCountedPtr<GNMWrappedResource::VertexShader>					m_VertexShader;
	Gear::RefCountedPtr<GNMWrappedResource::PixelShader>					m_PixelShader;
	GfxexConstantsBuffer*																				m_ConstantsBuffers[GFX_SHADER_TYPE_COUNT];
	PlatformGfxInputLayout*					m_InputLayout;

	VertexBufferDesc				m_VertexBuffers[GNMDevice::max_vstreams];
	enum
	{
		//@@LRF to be check
		MaxVertexInputElements = 16
	};
	sce::Gnm::Buffer                m_InputStreamBuffers[MaxVertexInputElements];
	IndexBufferDesc					m_IndexBuffer;

	ubiU32                          m_IndexSizeInBytes;

	ubiU32							m_DefaultLODBias;
	ubiBool							m_AnisoFilterEnabled;
	ubiU32							m_AnisotropyDegree;

	// Sampler states
	GfxSamplingState                m_PSSamplerStates[GNMDevice::max_samplers];
	sce::Gnm::Sampler 				m_PSSamplerStatesObj[GNMDevice::max_samplers];

	GfxSamplingState                m_VSSamplerStates[GNMDevice::max_vsamplers];
	sce::Gnm::Sampler				m_VSSamplerStatesObj[GNMDevice::max_vsamplers];

	sce::Gnm::Texture			    m_VSShaderResources[GNMDevice::max_vsamplers];
	sce::Gnm::Texture			    m_PSShaderResources[GNMDevice::max_samplers];
	sce::Gnm::Buffer *				m_PSBuffers[GNMDevice::max_samplers];
#ifndef DISABLE_TEX_RT_FENCE
	OrbisGPUFence *					m_VSShaderResourceFences[GNMDevice::max_vsamplers];
	OrbisGPUFence *					m_PSShaderResourceFences[GNMDevice::max_samplers];
#endif
	GfxexSurface*					m_RenderTargets[GNMDevice::max_rts];
	GfxexSurface*					m_DepthTarget;

	ubiU32                          m_RenderTargetsMask;

	// @@guoxx: only support 1 viewport and 1 scissor rect for the moment, need to update if gona to use geometry shader
	PlatformGfxViewport						m_Viewports;
	GFX_RECT								m_ScissorRects;

	GNMDevice*						m_Device; //@@CK: GNMDevice, containers for renderstates

											  // Render states
	GNMRenderStates					m_CompressedRenderStates;

	GfxTextureView*					m_defaultShaderResourceView{ nullptr };

	/*
	// anything put in here will be initialized by a memset to 0xff...
	struct GNMDirtyFlags
	{

	//		ubiU64                          GraphicsPipeDirty : 1;    // any Draw command
	//		ubiU64                          ComputePipeDirty : 1;    // any Dispatch command //@@LRF for CS dispatch, no need for our game


	ubiU64                          RenderTargetsDirty : GNMDevice::max_rts;
	ubiU64                          VSSamplersDirty : GNMDevice::max_vsamplers;
	ubiU64                          PSSamplersDirty : GNMDevice::max_samplers;
	} m_DirtyFlags;
	*/


	ubiU32	m_dirtyFlags;
	ubiU32	m_renderTargetDirtyFlags;
	ubiU32	m_vsSamplerDirtyFlags;
	ubiU32	m_psSamplerDirtyFlags;
	ubiU32	m_psBufferDirtyFlags;

	// =======================================================================

	static void SetCompressedRenderState(GFX_RENDER_STATE_TYPE State, ubiU32 Value, GNMRenderStates &RenderStates, ubiU32* boolDirtFlag);
	static void GetCompressedRenderState(GFX_RENDER_STATE_TYPE State, const GNMRenderStates &RenderStates, ubiU32& Value);



#ifndef POP_OPTIMIZED
	static ubiBool                      ms_ForceAllStateDirty;
#endif

	//=========================================================================
	//@@LRF Render state stack
	enum
	{
		GNMSM_RENDERSTATES_STACK = 8,
	};
	ubiU32								m_RenderStatesStackLevel;
	GNMRenderStates						m_RenderStatesStack[GNMSM_RENDERSTATES_STACK];
	GNMRenderResources                  m_RenderResourcesStack[GNMSM_RENDERSTATES_STACK];

	//=========================================================================
	//@@LRF default render states (static)
	static GNMRenderStates						s_DefaultRenderStates;

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
	//@@LRF Only shaders and constants at this moment. to be add more.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
	struct ConstantLog
	{
		ubiU32 registerIndex;
		ubiU32 nbRegisters;
		ubiVector4* dataPtr;

		ConstantLog() : dataPtr(NULL) {}

		ConstantLog(ubiU32 _registerIndex, ubiU32 _nbRegisters, const ubiVector4* _dataPtr);


		ConstantLog(const ConstantLog& _cl);

		void Clear();
	};
	struct GPUSubmissionLog
	{
		const ubiChar *mPSName;
		const ubiChar *mVSName;

		ubiChar m_hashCode_V[48];
		ubiChar m_hashCode_P[48];

		ubiBool mHasCommitConstantBuffer_V;
		ubiU32	mNbConstant_V;
		std::vector<ConstantLog> mConstantLog_V;
		ubiBool mHasCommitConstantBuffer_P;
		ubiU32	mNbConstant_P;
		std::vector<ConstantLog> mConstantLog_P;

		ubiU32 mBatchCount;

		GPUSubmissionLog() {
			mPSName = "Unset";
			mVSName = "Unset";
			popMemSet(m_hashCode_V, 0, 48);
			popMemSet(m_hashCode_P, 0, 48);
			mHasCommitConstantBuffer_V = false;
			mNbConstant_V = 0;
			mHasCommitConstantBuffer_P = false;
			mNbConstant_P = 0;

			mBatchCount = 0;
		}

		~GPUSubmissionLog() {
			for (auto itor = mConstantLog_V.begin(); itor != mConstantLog_V.end(); itor++)
			{
				itor->Clear();
			}
			mConstantLog_V.clear();

			for (auto itor = mConstantLog_P.begin(); itor != mConstantLog_P.end(); itor++)
			{
				itor->Clear();
			}
			mConstantLog_P.clear();
		}
	};


	static std::queue<GPUSubmissionLog> mSubmissionLogs; // for recent 10 frames

	static ubiU32 mBatchCounter;

	static void PushSubmissionLog(const GPUSubmissionLog &log);
#endif
#endif // POP_OPTIMIZED

public:

	inline void SetDirtyFlag(ubiU32 a_flag)
	{
		m_dirtyFlags |= a_flag;
	}

#ifndef POP_OPTIMIZED //@@LRF logs for gpu submission.
#ifdef ENABLE_DUMP_GPU_SUBMISSION
	static void SkipBatchCounter() { mBatchCounter++; }
	static void ResetBatchCounter() { mBatchCounter = 0; }
	static void DumpSubmissionLog();
#endif
#endif // POP_OPTIMIZED
};

popEND_NAMESPACE
