#pragma once

namespace Framework
{
	class GraphicDevice;
	class RenderSurface;
	class GPUFence;

	class RenderContext
	{
	public:
		RenderContext(GraphicDevice *device);
		virtual ~RenderContext();

		virtual void roll();
		virtual void reset();
		virtual void submitAndFlip(bool asynchronous);
		virtual void appendLabelAtEOPWithInterrupt(void *dstGpuAddr, U64 value);	// writes value and and triggers an interrupt
		virtual void appendLabelAtEOP(void *dstGpuAddr, U64 value);					// writes value only


		inline void attachFence(GPUFence *fence) {}

		void setTextureSurface(U32 soltID, const RenderSurface *surface);
		void setRenderTargetSurface(U32 soltID, const RenderSurface *surface);
		void setDepthStencilTargetSurface(const RenderSurface *surface);

		void setViewport(U32 x, U32 y, U32 width, U32 height, Float32 minz = 0.0f, Float32 maxz = 1.0f);

	protected:
		GraphicDevice *					mDevice{ nullptr };

	};
}

////////////////////////////////////////////////

//============================================================================
// gnmwrapper.h
//
// Copyright(C) Ubisoft
//============================================================================

#ifndef __GRAPHIC__GNM__GNMWRAPPER__H__
#define __GRAPHIC__GNM__GNMWRAPPER__H__

#include "graphic/gfx/gfxdevice.h"
#include "graphic/gfx/gfxutils.h"
#include "graphic/gfx/gfxplatformtypes.h"

#include "graphic/gfx/gfxExtension/gfxexdebugdisplay.h"
#include "graphic/gnm/gnmshaderparammap.h"
#include "graphic/gfx/gfxExtension/gfxexutils.h"
#include "graphic/gnm/gnmtextureheader.h"
#include "graphic/gnm/gnmwrappedresource.h"

#include "graphic/common/textures/texture.h"

#ifdef POP_PLATFORM_WIN32
//#define POP_USE_D3DSTATECACHE
#endif

#ifdef POP_USE_D3DSTATECACHE

#define POP_CHECK_D3DSTATECACHE(condition) (condition)
#define POP_UPDATE_D3DSTATECACHE(assignment) (assignment)

#else

#define POP_CHECK_D3DSTATECACHE(condition) (true)
#define POP_UPDATE_D3DSTATECACHE(assignment)

#endif // POP_USE_D3DSTATECACHE

#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
#define GNMDevice_CALL(call) if(m_IsRecordingGNM) \
                             { \
                                m_RecordingCmdBuffer_##call; \
                             } \
                             else \
                             { \
                                GNM_CALL(call) \
                             }
#else
#define GNMDevice_CALL(call) GNM_CALL(call)
#endif

//----------------------------------------------------------------------------

// uncomment this line to disable rendering
//#define DISABLE_RENDERING

// bm todo: this optimization cannot be enabled yet on Xbox 360 due to an XDK bug...
#define EMULATE_GPUBEGIN_API

#define EMULATE_BEGINVERTICES_API

#ifndef POP_FINAL
#if !GNM_DISABLED
#define POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
#endif
#endif

//@@LRF CLANG forbids forward references to 'enum' types, SO I include the header file here.
#include "graphic/shaders/shaders.h"

//============================================================================
// interface
//============================================================================
// external
popBEGIN_NAMESPACE

#ifndef USING_SEPARATE_CUE_HEAP
#define MAX_CUE_HEAPS			2   // Maximum number of CUE heaps, two should be enough
#endif

enum BlendFunc
{
	BlendFunc_Zero = sce::Gnm::kBlendMultiplierZero,
	BlendFunc_One = sce::Gnm::kBlendMultiplierOne,
	BlendFunc_SrcColor = sce::Gnm::kBlendMultiplierSrcColor,
	BlendFunc_InvSrcColor = sce::Gnm::kBlendMultiplierOneMinusSrcColor,
	BlendFunc_SrcAlpha = sce::Gnm::kBlendMultiplierSrcAlpha,
	BlendFunc_InvSrcAlpha = sce::Gnm::kBlendMultiplierOneMinusSrcAlpha,
	BlendFunc_DestAlpha = sce::Gnm::kBlendMultiplierDestAlpha,
	BlendFunc_InvDestAlpha = sce::Gnm::kBlendMultiplierOneMinusDestAlpha,
	BlendFunc_DestColor = sce::Gnm::kBlendMultiplierDestColor,
	BlendFunc_InvDestColor = sce::Gnm::kBlendMultiplierOneMinusDestColor,
	BlendFunc_BlendFactor = sce::Gnm::kBlendMultiplierConstantAlpha,
	BlendFunc_InvBlendFactor = sce::Gnm::kBlendMultiplierOneMinusConstantAlpha,
	NbBlendFunc
};

enum CullMode
{
	CullMode_None = sce::Gnm::kPrimitiveSetupCullFaceNone,
	CullMode_CW = sce::Gnm::kPrimitiveSetupCullFaceFront,
	CullMode_CCW = sce::Gnm::kPrimitiveSetupCullFaceBack
};

enum FaceWinding {
	FaceWinding_CW = sce::Gnm::kPrimitiveSetupFrontFaceCw,
	FaceWinding_CCW = sce::Gnm::kPrimitiveSetupFrontFaceCcw
};

//@@LRF  CLANG forbids forward references to 'enum' types, SO I include the header file in the head of file.
// enum ShaderConstantEnum;
// enum PixelShaders;
// enum VertexShaders;
class GNMShaderConstantMap;
class PixelShader;
class VertexShader;
class ShaderManager;
class GfxTexture;

//============================================================================
// move following block to some reasonable place
//============================================================================
#if defined INLINE
#undef INLINE
#endif

#define INLINE __attribute__((always_inline))
template<typename T, typename U>
INLINE T raw_cast(const U &v_)
{
	return reinterpret_cast<const T&>(v_);
}

#define D3DSTATE_TO_INDEX(state)   ((ubiU32)state)
#define INDEX_TO_D3DSTATE(state)   (state)

//----------------------------------------------------------------------------

struct GfxexDeviceOptions;

//============================================================================
// GNMDevice
//============================================================================
#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
class GNMDevice : public GNMRecordingDevice
#else
class GNMDevice : public GfxBaseDevice
#endif
{
public:
#ifndef POP_OPTIMIZED
	ubiChar mName[32];
#endif // POP_OPTIMIZED
	// device config
	enum {
		max_samplers = 16, max_vsamplers = 4,
		max_rts = 4,
		max_vstreams = 2,
		max_vsc_f = 256, max_psc_f = 256,
		max_vsc_i = 16, max_psc_i = 16,
		max_vsc_b = 16, max_psc_b = 16
	};

	enum
	{
		// 		MaxRenderTargets = 8,
		// 		MaxVertexSamplers = 4,
		// 		MaxGeometrySamplers = 4,
		// 		MaxPixelSamplers = 16,
		// 		MaxComputeSamplers = 16,
		// 		MaxComputeOutputs = 4,
		// 		MaxVertexStreams = 4,
		// 		MaxConstantBuffers = 14,
		// 		MaxVertexShaderConstantsF = 120,
		// 		MaxVertexShaderConstantsI = 32,
		// 		MaxVertexShaderConstantsB = 32,
		// 		MaxPixelShaderConstantsF = 256,
		// 		MaxPixelShaderConstantsI = 32,
		// 		MaxPixelShaderConstantsB = 32,

		NumSwapchainBuffers = 3     // Should match NumFenceArrays
	};

#ifdef EMULATE_BEGINVERTICES_API
	enum
	{
#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
		DynamicVertexBufferSize = (12 * 256 * 1024),
		DynamicIndexBufferSize = (12 * 64 * 1024),
#else
		DynamicVertexBufferSize = (256 * 1024),
		DynamicIndexBufferSize = (64 * 1024),
#endif
	};
#endif

#ifdef POP_PLATFORM_WIN32
	enum
	{
		max_renderstates = 256,
		max_samplerstates = 16,
	};
#elif POP_PLATFORM_DURANGO
	enum
	{
		max_renderstates = 256, //@@CK TODO
		max_samplerstates = 16,
	};
#elif POP_PLATFORM_ORBIS
	enum
	{
		max_renderstates = 256, //@@CK TODO
		max_samplerstates = 16,
	};
#else
	enum
	{
		max_renderstates = (D3DSTATE_TO_INDEX(GFX_RS_MAX)),
		max_samplerstates = (D3DSTATE_TO_INDEX(GFX_SAMP_MAX)),
	};
#endif // POP_PLATFORM_WIN32

	//------------------------------------------------------------------------

	// construction
	GNMDevice(ubiBool isMainContext = true, ubiU32 deferredID = 0);
	virtual ~GNMDevice();


#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
	INLINE void CleanD3DDevice() { m_device = NULL; }
#endif
	ubiBool GetIsUsingExDevice() const { return m_isUsingExDevice; }

#ifndef GNM_PORTING_TODO
	HRESULT Reset(DXGI_SWAP_CHAIN_DESC&);
#endif

	void SetOptions(GfxexDeviceOptions& options);

	//------------------------------------------------------------------------

	// resource creation
	GNMTexture* CreateTexture(ubiU32 width_, ubiU32 height_, ubiU32 num_levels_, GFX_USAGE usage_, GFXFORMAT format_, ubiU32 bindFlags = 0);
	GNMTexture* CreateCubeTexture(ubiU32 edge_len_, ubiU32 num_levels_, GFX_USAGE usage_, GFXFORMAT format_, ubiU32 bindFlags = 0);
	GNMTexture* CreateVolumeTexture(ubiU32 width_, ubiU32 height_, ubiU32 depth_, ubiU32 num_levels_, GFX_USAGE usage_, GFXFORMAT format_, ubiU32 bindFlags = 0);
	PlatformGfxRenderTargetView* CreateRenderTargetView(ubiU32 width_, ubiU32 height_, GFXFORMAT format_, ubiBool isDynamic_, ubiBool isSwapChain_, GFX_MULTISAMPLE_TYPE msaaType);
	PlatformGfxDepthStencilView* CreateDepthStencilView(ubiU32 width_, ubiU32 height_, sce::Gnm::ZFormat zFormat_, sce::Gnm::StencilFormat stencilFormat_, ubiBool useHTile_, GFX_MULTISAMPLE_TYPE msaaType);
	GfxBufferView* CreateBufferView(ubiBool writable, sce::Gnm::DataFormat format, ubiU32 numElements);

	Gear::RefCountedPtr<GNMWrappedResource::IndexBuffer> CreateIndexBuffer(ubiU32 numBytes, GFX_USAGE usage, GFXFORMAT format, void* data = NULL);
	Gear::RefCountedPtr<GNMWrappedResource::VertexBuffer> CreateVertexBuffer(ubiU32 numBytes, GFX_USAGE usage, GFXPOOL pool = GFXPOOL_DEFAULT, void* data = NULL);
	Gear::RefCountedPtr<GNMWrappedResource::PixelShader> CreatePixelShader(const void* compiled_, ubiU32 size_);
	Gear::RefCountedPtr<GNMWrappedResource::VertexShader> CreateVertexShader(const void* compiled_, ubiU32 size_);
	Gear::RefCountedPtr<GNMWrappedResource::ComputeShader> CreateComputeShader(const void* compiled_, ubiU32 size_);
	GNMVertexDeclaration* CreateVertexDeclaration(const GNMVertexFormatDescriptionElement* input_elements_, ubiU32 num_elements_, ubiChar* debugInfo = NULL);
	//------------------------------------------------------------------------

	// scene management
	void BeginScene();
	void EndScene();
	//------------------------------------------------------------------------

	INLINE ubiBool IsDeferred() const;
	INLINE PtrArray<GnmCommandList> & GetPendingCommandLists();
	INLINE void PushPendingCommandList(GnmCommandList *cmdList);

	// state management
	INLINE void SetRenderState(GFX_RENDER_STATE_TYPE state, DWORD value);
	INLINE void SetSamplerState(ubiU32 sampler, GFX_SAMPLER_STATE_TYPE state, DWORD value);
	INLINE void SetVSamplerState(ubiU32 sampler, GFX_SAMPLER_STATE_TYPE state, DWORD value);
#if !GNM_DISABLED
	INLINE void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
#endif

	INLINE void SetTexture(ubiU32 sampler, GNMTexture* texture);
	INLINE void SetTexture(ubiU32 sampler, GfxTextureView* texture);
	INLINE void SetBuffer(ubiU32 sampler, GfxBufferView* buffer);
	INLINE void UnsetTexture(ubiU32 sampler);

	INLINE void SetVTexture(ubiU32 sampler, GfxTextureView* texture);
	INLINE void SetStreamSource(ubiU32 stream, const PlatformGfxBaseBuffer* vb, ubiU32 offset, ubiU32 stride);
	INLINE void SetStreamSource(ubiU32 stream, const Gear::RefCountedPtr<GNMWrappedResource::VertexBuffer>& vb, ubiU32 offset, ubiU32 stride);
	INLINE void SetIndices(const PlatformGfxBaseBuffer* ib, GFXFORMAT fmt, ubiU32 offset = 0);
	INLINE void SetIndices(const Gear::RefCountedPtr<GNMWrappedResource::IndexBuffer>& ib, ubiU32 offset = 0);
	INLINE void SetRenderTarget(ubiU32 rtIndex, GfxexSurface* rt);
	INLINE void GetRenderTarget(ubiU32 rtIndex, GfxexSurface** rt);
	INLINE void SetDepthStencilSurface(GfxexSurface* ds);
	INLINE void GetDepthStencilSurface(GfxexSurface** ds);
	INLINE void SetViewport(const PlatformGfxViewport& vp);
	INLINE void SetScissorRect(const GFX_RECT& rect);
	INLINE void GetViewport(PlatformGfxViewport* vp);

	INLINE const PlatformGfxViewport& GetViewport()const;
	void SetViewport(ubiU32 x, ubiU32 y, ubiU32 width, ubiU32 height, ubiFloat minz = 0.0f, ubiFloat maxz = 1.0f);


	void ResetStates();
	void FullResetStates();

	void SetDefaultLODBias(ubiFloat lodBias);
	void SetAnisotropicFilteringOverride(ubiBool enabled, ubiU8 anisotropyDegree);

	void SetDefaults(ViewSurface* viewSurface);

	void RestoreDefaultSamplerStates();

	//------------------------------------------------------------------------

	// shader management
	void SetVertexShaderInternal(PlatformGfxVertexShader* vs);
	INLINE void SetVertexShader(const Gear::RefCountedPtr<GNMWrappedResource::VertexShader>& vs);
	INLINE void SetVertexDeclaration(PlatformGfxInputLayout* vd);
	INLINE PlatformGfxInputLayout* GetVertexDeclaration();
	void SetVertexShaderConstantF(unsigned start_reg_, const float*, unsigned num_vec4f_);
	void SetVertexShaderConstantI(unsigned start_reg_, const int*, unsigned num_vec4i_);
	void SetVertexShaderConstantB(unsigned start_reg_, const bool*, unsigned num_bools_);
	void SetPixelShaderInternal(PlatformGfxPixelShader* ps);
	INLINE void SetPixelShader(const Gear::RefCountedPtr<GNMWrappedResource::PixelShader>& ps);
	void SetPixelShaderConstantF(unsigned start_reg_, const float*, unsigned num_vec4f_);
	void GetPixelShaderConstantF(unsigned start_reg_, const float*, unsigned num_vec4f_);
	void SetPixelShaderConstantI(unsigned start_reg_, const int*, unsigned num_vec4i_);
	void SetPixelShaderConstantB(unsigned start_reg_, const bool*, unsigned num_bools_);

	INLINE void GpuBeginVertexShaderConstantF4(ubiU32 start, float** data, unsigned vector4Count);
	INLINE void GpuEndVertexShaderConstantF4();
	INLINE void GpuBeginPixelShaderConstantF4(ubiU32 start, float** data, unsigned vector4Count);
	INLINE void GpuEndPixelShaderConstantF4();
	//------------------------------------------------------------------------

	// rendering
	INLINE void DrawPrimitive(GFXPRIMITIVETYPE pt, ubiU32 startVertex, ubiU32 primCount);
	INLINE void DrawIndexedPrimitive(GFXPRIMITIVETYPE pt, ubiInt baseVertexIndex, ubiU32 minIndex, ubiU32 numIndices, ubiU32 startIndex, ubiU32 primCount);

	INLINE void Clear(ubiU32 numRects, const GfxRect* rects, DWORD flags, D3DCOLOR colorARBG, ubiFloat z, ubiU8 stencil);
	void ClearRenderTargetView(GfxexSurface* rtv, ubiFloat r, ubiFloat g, ubiFloat b, ubiFloat a);
	void ClearDepthStencilView(GfxexSurface* dsv, ubiBool clearDepth = true, ubiFloat depth = 1.0f, ubiBool clearStencil = true, ubiU8 stencil = 0x00);

	void StretchRect(GfxexSurface* src, const RECT* srcRect, GfxexSurface* dest, const RECT* destRect, GFX_TEX_FILTER filter);
	void ApplyFilter2D(GfxexSurface* src, const RECT* srcRect, GfxexSurface* dest, const RECT* destRect, GFX_TEX_FILTER filter);
	void ResolveMSAASurface(GfxexSurface* src, GfxexSurface* dest, GFX_MULTISAMPLE_TYPE msaaType);

	// dynamic rendering
	INLINE void BeginVertices(GFXPRIMITIVETYPE type, ubiU32 vertexCount, ubiU32 streamStride, void** vertexData);
	INLINE void EndVertices();
	INLINE void BeginIndexedVertices(GFXPRIMITIVETYPE type, ubiU32 baseVertexIndex, ubiU32 vertexCount, ubiU32 indexCount, GFXFORMAT indexFormat, ubiU32 streamStride, void** indexData, void** vertexData);
	INLINE void EndIndexedVertices();

	INLINE void EndIndexedVertices_NoDraw(ubiU32& currentVBOffset, ubiU32& currentIBOffset);

	static INLINE ubiU32 GetMaxDynamicVertexCount(ubiU32 streamStride);

	INLINE ubiU32 GetRenderState(GFX_RENDER_STATE_TYPE state);

#ifdef INVERSE_DEPTH
	static INLINE ubiBool GetInverseMode() { return m_bInversedMode; }
	static INLINE void SetInverseMode(bool InverseMode) { m_bInversedMode = InverseMode; }
#endif //INVERSE_DEPTH
	//------------------------------------------------------------------------

#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
	void BeginGlobalConstantsUpdate();
	void EndGlobalConstantsUpdate();

	void BeginGlobalConstantsCheck();
	void EndGlobalConstantsCheck();
#else
	inline void BeginGlobalConstantsUpdate() {}
	inline void EndGlobalConstantsUpdate() {}

	inline void BeginGlobalConstantsCheck() {}
	inline void EndGlobalConstantsCheck() {}
#endif

	void CopyResource(PlatformGfxBaseTexture* dst, PlatformGfxBaseTexture* src);
	void CopySubresource(PlatformGfxBaseTexture* dst, ubiU32 dstSubresourceIndex, PlatformGfxBaseTexture* src, ubiU32 srcSubresourceIndex);

#ifdef POP_USE_PROFILER
	// debugging interface
	INLINE void PushMarker(const ubiChar* marker);
	INLINE void PopMarker();
#endif

	/*
	* start recording graphic command into a deferred command buffer [only valid for a deferred context device]
	*/
	INLINE void StartRecordingCommands();

	/*
	* finish recording into a deferred command buffer and prepare data for rendering [only valid for a deferred context device]
	*/
	INLINE GfxCmdList* FinishRecordingCommands();

	/*
	* execute previously recorded commands in the immediate context [only valid for a immediate context device]
	*/
	INLINE void ExecuteRecordedCommands(GfxCmdList* cmdList);

	INLINE void BeginCommandListExecution();

	INLINE void EndCommandListExecution();

	// Force the GPU to consume any commands that are pending in the command buffer
	void KickCommandBuffer(ubiBool a_waitComplete);

	void InsertResourceBarrierForDepthRenderTarget(GfxexSurface* surface);

	ubiU64 InsertFence();
	ubiBool IsFencePending(ubiU64 fence);
	void GPUWaitOnFence(ubiU64 fence);

	void FlushSurfaceCache(GfxexSurface* a_surface);

	void InvalidateGPUCache();

	void PushRenderStates();
	void PopRenderStates();


	//@@LRF Todo
	// for FMask
	//void DecompressSwapchainSurface(GfxexSurface* src);

	//void DecompressColorSurface(GfxexSurface* src, GfxexSurface* dest);

	void ResummarizeHtile(GfxexSurface* src);

	// for HTile
	void DecompressDepthSurface(GfxexSurface* src, GfxexSurface* dest, ubiBool resummarizeHtile = false);

	void DecompressFmaskSurface(GfxexSurface* src);

	/////////////////////////////////////////////////////////////////////////////////////////
	// GNMState
	//
	// Various state settings helpers that used to be in GNMState, now moved here and they
	// aren't static anymore so you must use a device object to set those
	//
	/////////////////////////////////////////////////////////////////////////////////////////
#if !defined(POP_FINAL) && defined(POP_USE_D3DSTATECACHE)
	inline void GNMDevice::GetRenderState(GFX_RENDER_STATE_TYPE state, DWORD *pvalue)
	{
		*pvalue = m_CurrentRenderState[D3DSTATE_TO_INDEX(state)];
	}
#endif

	inline ubiBool GetZEnabled()
	{
		return GetRenderState(GFX_RS_ZENABLE);
	}

	inline GFX_CMP_FUNC GetZCompare()
	{
		return static_cast<GFX_CMP_FUNC>(GetRenderState(GFX_RS_ZFUNC));
	}

	inline ubiBool GetStencilEnabled()
	{
		return GetRenderState(GFX_RS_STENCILENABLE);
	}

	inline void SetZWriteEnabled(ubiBool enabled)
	{
		SetRenderState(GFX_RS_ZWRITEENABLE, enabled);
	}

	inline void SetZEnabled(ubiBool enabled)
	{
		SetRenderState(GFX_RS_ZENABLE, enabled);
	}
	inline void SetZCompare(GFX_CMP_FUNC test)
	{
#ifdef INVERSE_DEPTH
		if (GetInverseMode())
		{
			switch (test)
			{
			case scimitar::GFX_CMP_LESS:
				test = GFX_CMP_GREATER;
				break;
			case scimitar::GFX_CMP_LESSEQUAL:
				test = GFX_CMP_GREATEREQUAL;
				break;
			case scimitar::GFX_CMP_GREATER:
				test = GFX_CMP_LESS;
				break;
			case scimitar::GFX_CMP_GREATEREQUAL:
				test = GFX_CMP_LESSEQUAL;
				break;
			default:
				break;
			}
		}
#endif //INVERSE_DEPTH
		SetRenderState(GFX_RS_ZFUNC, test);
	}
	inline void SetZTestEnabled(ubiBool enabled)
	{
#ifdef INVERSE_DEPTH
		if (GetInverseMode())
		{
			SetRenderState(GFX_RS_ZFUNC, enabled ? GFX_CMP_GREATEREQUAL : GFX_CMP_ALWAYS);
		}
		else
#endif //INVERSE_DEPTH
		{
			SetRenderState(GFX_RS_ZFUNC, enabled ? GFX_CMP_LESSEQUAL : GFX_CMP_ALWAYS);
		}
	}

	inline void SetAlphaBlendEnabled(ubiBool enabled)
	{
		SetRenderState(GFX_RS_ALPHABLENDENABLE, enabled);
	}

	inline void SetAlphaTestEnabled(ubiBool enabled, ubiBool useAlphaToCoverage = true)
	{
		SetRenderState(GFX_RS_ALPHATESTENABLE, enabled);
	}

	inline void SetAlphaTestValue(ubiU8 value)
	{
		SetRenderState(GFX_RS_ALPHAREF, value);
	}
	inline void SetAlphaTestCompare(GFX_CMP_FUNC test)
	{
		SetRenderState(GFX_RS_ALPHAFUNC, test);
	}

	inline void SetAlphaTest(ubiBool enabled, ubiU8 value, ubiBool inverted, ubiBool useAlphaToCoverage = true)
	{
		SetRenderState(GFX_RS_ALPHATESTENABLE, enabled);
		SetRenderState(GFX_RS_ALPHAREF, value);
		SetRenderState(GFX_RS_ALPHAFUNC, inverted ? GFX_CMP_LESS : GFX_CMP_GREATEREQUAL);
	}

	inline void SetAlphaBlendMode(BlendMode mode)
	{
		if (mode == Blend_Copy)
		{
			SetAlphaBlendEnabled(false);
		}
		else
		{
			SetAlphaBlendEnabled(true);
			SetRenderState(GFX_RS_SRCBLEND, m_BlendModeToSrcBlend[mode]);
			SetRenderState(GFX_RS_DESTBLEND, m_BlendModeToDstBlend[mode]);
			SetRenderState(GFX_RS_SRCBLENDALPHA, m_BlendModeToSrcBlend[mode]);
			SetRenderState(GFX_RS_DESTBLENDALPHA, m_BlendModeToDstBlend[mode]);
		}
	}

	inline void SetAlphaBlendModeSeparate(BlendMode colorMode, BlendMode alphaMode)
	{
		if ((colorMode == Blend_Copy) && (alphaMode == Blend_Copy))
		{
			SetAlphaBlendEnabled(false);
		}
		else
		{
			SetAlphaBlendEnabled(true);
			SetRenderState(GFX_RS_SEPARATEALPHABLENDENABLE, true);
			SetRenderState(GFX_RS_SRCBLEND, m_BlendModeToSrcBlend[colorMode]);
			SetRenderState(GFX_RS_DESTBLEND, m_BlendModeToDstBlend[colorMode]);
			SetRenderState(GFX_RS_SRCBLENDALPHA, m_BlendModeToSrcBlend[alphaMode]);
			SetRenderState(GFX_RS_DESTBLENDALPHA, m_BlendModeToDstBlend[alphaMode]);
		}
	}


	inline void DisableSeparateAlphaBlend()
	{
		SetRenderState(GFX_RS_SEPARATEALPHABLENDENABLE, false);
		SetAlphaBlendEnabled(false);    // like the other platforms...
	}


	inline void SetWireFrame(ubiBool wireFrame)
	{
		SetRenderState(GFX_RS_FILLMODE, wireFrame ? GFX_FILL_WIREFRAME : GFX_FILL_SOLID);
	}
	inline void SetCullingMode(GFX_CULL_MODE mode)
	{
		SetRenderState(GFX_RS_CULLMODE, mode);
	}
	inline void SetCulling(ubiBool enabled, ubiBool inverted)
	{
		SetRenderState(GFX_RS_CULLMODE, (!enabled) ? GFX_CULL_MODE_NONE : (inverted) ? GFX_CULL_MODE_CCW : GFX_CULL_MODE_CW);
	}

	inline void SetWriteColorMask(ubiU32 mask)
	{
		SetRenderState(GFX_RS_COLORWRITEENABLE, mask);
	}



	inline void SetBlendFactor(ubiU32 factor)
	{
		SetRenderState(GFX_RS_BLENDFACTOR, factor);
	}

	inline void SetStencilRefIfNotZero(ubiU8 ref)
	{
		// only set the stencil when doing the MaterialMask renderpass
		if (ref != 0)
		{
			SetRenderState(GFX_RS_STENCILENABLE, TRUE);
			SetRenderState(GFX_RS_STENCILREF, ref);
		}
		else
		{
			SetRenderState(GFX_RS_STENCILENABLE, FALSE);
		}
	}

	inline void SetStencilRef(ubiU8 ref)
	{
		SetRenderState(GFX_RS_STENCILREF, ref);
	}

	inline void SetStencilRefSpecial(ubiU8 ref, ubiBool active)
	{
		SetRenderState(GFX_RS_STENCILREF, ref);
	}

	inline void SetSRGBWrite(ubiBool enabled)
	{
#ifdef POP_USE_SRGB
		m_GNMDevice->SetRenderState(GFX_RS_SRGBWRITEENABLE, enabled);
#endif
	}

	inline void SetVertexTextureWrap(ubiU32 samplerIndex, GFX_TEX_ADDRESS wrapU, GFX_TEX_ADDRESS wrapV, GFX_TEX_ADDRESS wrapW = GFX_TEX_ADDRESS_WRAP)
	{
		SetVSamplerState(samplerIndex, GFX_SAMP_ADDRESSU, wrapU);
		SetVSamplerState(samplerIndex, GFX_SAMP_ADDRESSV, wrapV);
		SetVSamplerState(samplerIndex, GFX_SAMP_ADDRESSW, wrapW);
	}

	inline void SetVertexTextureFilter(ubiU32 samplerIndex, GFX_TEX_FILTER minFilter, GFX_TEX_FILTER magFilter, GFX_TEX_FILTER mipFilter)
	{
		SetVSamplerState(samplerIndex, GFX_SAMP_MINFILTER, minFilter);
		SetVSamplerState(samplerIndex, GFX_SAMP_MAGFILTER, magFilter);
		SetVSamplerState(samplerIndex, GFX_SAMP_MIPFILTER, mipFilter);
	}

	inline void UseTexture(ubiU32 sampler, GfxTexture* texture)
	{
		PlatformGfxTexture* platformTexture = static_cast<PlatformGfxTexture*>(texture);
		platformTexture->UseTexture(sampler, this);
	}

	inline void SetTexture(ShaderConstantEnum constant, GNMTexture* texture)
	{
		SetTexture(m_PixelShaderConstantMap->GetConstantIndex(constant), texture);
	}

	inline void SetTexture(ShaderConstantEnum constant, GfxTextureView* texture)
	{
		SetTexture(m_PixelShaderConstantMap->GetConstantIndex(constant), texture);
	}

	inline ubiU32 ResolvePixelInput(ShaderConstantEnum constant)
	{
		return m_PixelShaderConstantMap->GetConstantIndex(constant);
	}

	inline ubiU32 ResolveVertexInput(ShaderConstantEnum constant)
	{
		return m_VertexShaderConstantMap->GetConstantIndex(constant);
	}

	inline void SetTexWrap(ubiU32 samplerIndex, AddressMode wrapU, AddressMode wrapV, AddressMode wrapW = AddressMode_Wrap)
	{
		SetTextureWrap(samplerIndex, GetGfxAddressMode(wrapU), GetGfxAddressMode(wrapV), GetGfxAddressMode(wrapW));
	}

	inline void SetTextureWrap(ubiU32 samplerIndex, GFX_TEX_ADDRESS wrapU, GFX_TEX_ADDRESS wrapV, GFX_TEX_ADDRESS wrapW = GFX_TEX_ADDRESS_WRAP)
	{
		SetSamplerState(samplerIndex, GFX_SAMP_ADDRESSU, wrapU);
		SetSamplerState(samplerIndex, GFX_SAMP_ADDRESSV, wrapV);
		SetSamplerState(samplerIndex, GFX_SAMP_ADDRESSW, wrapW);
	}

	inline void SetTexFilter(ubiU32 samplerIndex, FilterType magFilter, FilterType minFilter, FilterType mipFilter, ubiU32 dummy)
	{
		SetTextureFilter(samplerIndex, GetGfxFilter(magFilter), GetGfxFilter(minFilter), GetGfxFilter(mipFilter));
	}

	inline void SetTextureFilter(ubiU32 samplerIndex, GFX_TEX_FILTER magFilter, GFX_TEX_FILTER minFilter, GFX_TEX_FILTER mipFilter)
	{
		SetSamplerState(samplerIndex, GFX_SAMP_MAGFILTER, magFilter);
		SetSamplerState(samplerIndex, GFX_SAMP_MINFILTER, minFilter);
		SetSamplerState(samplerIndex, GFX_SAMP_MIPFILTER, mipFilter);
	}

	inline void SetTextureBorderColor(ubiU32 samplerIndex, GFX_TEX_BORDER_COLOR color)
	{
		SetSamplerState(samplerIndex, GFX_SAMP_BORDERCOLOR, color);
	}

	inline void SetSRGBTexture(ubiU32 samplerIndex, ubiBool sRGBTexture)
	{
#ifdef POP_USE_SRGB
		SetSamplerState(samplerIndex, GFX_SAMP_SRGBTEXTURE, sRGBTexture);
#endif
	}

	inline void SetVectorVS(ubiU32 registerIndex, const ubiVector4& vector)
	{
		SetVertexShaderConstantF(registerIndex, (const float*)&vector, 1);
	}

	inline void SetVectorVS(ShaderConstantEnum constant, const ubiVector4& vector)
	{
		SetVertexShaderConstantF(m_VertexShaderConstantMap->GetConstantIndex(constant), (const float*)&vector, 1);
	}

	inline void SetVectorsVS(ubiU32 registerIndex, ubiU32 nbRegisters, const ubiVector4* vectors)
	{
		SetVertexShaderConstantF(registerIndex, (const float*)vectors, nbRegisters);
	}

	inline void SetVectorsVS(ShaderConstantEnum constant, ubiU32 nbRegisters, const ubiVector4* vectors)
	{
		SetVertexShaderConstantF(m_VertexShaderConstantMap->GetConstantIndex(constant), (const float*)vectors, nbRegisters);
	}

	inline void SetMatrixVS(ubiU32 registerIndex, const ubiMatrix44& matrix)
	{
		ubiMatrix44 temp = matrix.GetTranspose();
		SetVertexShaderConstantF(registerIndex, (const float*)&temp, 4);
	}

	inline void SetMatrixVS(ShaderConstantEnum constant, const ubiMatrix44& matrix)
	{
		ubiMatrix44 temp = matrix.GetTranspose();
		SetVertexShaderConstantF(m_VertexShaderConstantMap->GetConstantIndex(constant), (const float*)&temp, 4);
	}

	inline void SetMatrixVS(ShaderConstantEnum constant, ubiU32 index, const ubiMatrix44& matrix)
	{
		ubiMatrix44 temp = matrix.GetTranspose();
		SetVertexShaderConstantF(m_VertexShaderConstantMap->GetConstantIndex(constant) + (index * 4), (const float*)&temp, 4);
	}

	inline void SetMatricesVS(ubiU32 registerIndex, ubiU32 nbMatrices, const ubiMatrix44* matrices)
	{
		/* inconsistent with SetMatrixVS: doesn't transpose */
		SetVertexShaderConstantF(registerIndex, (const float*)matrices, nbMatrices * 4);
	}

	inline void SetFloatVS(ubiU32 registerIndex, ubiFloat value)
	{
		ubiVector4 v(value, value, value, value);
		SetVertexShaderConstantF(registerIndex, (ubiFloat*)v, 1);
	}

	inline void SetFloatVS(ShaderConstantEnum constant, ubiFloat value)
	{
		ubiVector4 v(value, value, value, value);
		SetVertexShaderConstantF(m_VertexShaderConstantMap->GetConstantIndex(constant), (ubiFloat*)&v, 1);
	}

	inline void SetVectorPS(ubiU32 registerIndex, const ubiVector4& vector)
	{
		SetPixelShaderConstantF(registerIndex, (const float*)&vector, 1);
	}

	inline void GetVectorPS(ubiU32 registerIndex, ubiVector4& vector)
	{
		GetPixelShaderConstantF(registerIndex, (const float*)&vector, 1);
	}

	inline void SetVectorPS(ShaderConstantEnum constant, const ubiVector4& vector)
	{
		SetPixelShaderConstantF(m_PixelShaderConstantMap->GetConstantIndex(constant), (const float*)&vector, 1);
	}

	inline void UnsetVectorVS(ShaderConstantEnum constant)
	{
#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
		InvalidateConstantOperation(false, m_VertexShaderConstantMap->GetConstantIndex(constant), 1);
#endif
	}

	inline void UnsetVectorPS(ShaderConstantEnum constant)
	{
#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
		InvalidateConstantOperation(true, m_PixelShaderConstantMap->GetConstantIndex(constant), 1);
#endif
	}

	inline void SetColorPS(ShaderConstantEnum constant, ubiU32 color)
	{
		// convert color to vector
		ubiVector4 colorVector;
		ConvertColorToVector(color, colorVector);
		SetPixelShaderConstantF(m_PixelShaderConstantMap->GetConstantIndex(constant), (const float*)&colorVector, 1);
	}

	inline void SetVectorsPS(ShaderConstantEnum constant, ubiU32 nbRegisters, const ubiVector4* vectors)
	{
		SetPixelShaderConstantF(m_PixelShaderConstantMap->GetConstantIndex(constant), (const float*)vectors, nbRegisters);
	}

	inline void SetVectorsPS(ubiU32 registerIndex, ubiU32 nbRegisters, const ubiVector4* vectors)
	{
		SetPixelShaderConstantF(registerIndex, (const float*)vectors, nbRegisters);
	}

	inline void SetMatrixPS(ubiU32 registerIndex, const ubiMatrix44& matrix)
	{
		ubiMatrix44 temp = matrix.GetTranspose();
		SetPixelShaderConstantF(registerIndex, (const float*)&temp, 4);
	}

	inline void SetMatrixPS(ShaderConstantEnum constant, const ubiMatrix44& matrix)
	{
		ubiMatrix44 temp = matrix.GetTranspose();
		SetPixelShaderConstantF(m_PixelShaderConstantMap->GetConstantIndex(constant), (const float*)&temp, 4);
	}

	inline void SetMatrixPS(ShaderConstantEnum constant, ubiU32 index, const ubiMatrix44& matrix)
	{
		ubiMatrix44 temp = matrix.GetTranspose();
		SetPixelShaderConstantF(m_PixelShaderConstantMap->GetConstantIndex(constant) + (index * 4), (const float*)&temp, 4);
	}

	inline void SetMatricesPS(ubiU32 registerIndex, ubiU32 nbMatrices, const ubiMatrix44* matrices)
	{
		SetPixelShaderConstantF(registerIndex, (const float*)matrices, nbMatrices * 4);
		/* inconsistent with SetMatrixPS: doesn't transpose */
	}

	inline void SetFloatPS(ubiU32 registerIndex, ubiFloat value)
	{
		ubiVector4 v(value, value, value, value);
		SetPixelShaderConstantF(registerIndex, (ubiFloat*)&v, 1);
	}

	void SetFloatPS(ShaderConstantEnum constant, ubiFloat value)
	{
		ubiVector4 v(value, value, value, value);
		SetPixelShaderConstantF(m_PixelShaderConstantMap->GetConstantIndex(constant), (ubiFloat*)&v, 1);
	}

	inline void SetIntegerVS(ubiU32 registerIndex, const ubiVector4I& integer)
	{
		SetVertexShaderConstantI(registerIndex, (const int*)&integer, 1);
	}

	inline void SetIntegerVS(ShaderConstantEnum constant, const ubiVector4I& integer)
	{
		SetVertexShaderConstantI(m_VertexShaderConstantMap->GetConstantIndex(constant), (const int*)&integer, 1);
	}

	inline void SetIntegersVS(ubiU32 registerIndex, ubiU32 nbRegisters, const ubiVector4I* integers)
	{
		SetVertexShaderConstantI(registerIndex, (const int*)integers, nbRegisters);
	}

	inline void SetIntegerPS(ubiU32 registerIndex, const ubiVector4I& integer)
	{
		SetPixelShaderConstantI(registerIndex, (const int*)&integer, 1);
	}

	inline void SetIntegerPS(ShaderConstantEnum constant, const ubiVector4I& integer)
	{
		SetPixelShaderConstantI(m_PixelShaderConstantMap->GetConstantIndex(constant), (const int*)&integer, 1);
	}

	inline void SetIntegersPS(ubiU32 registerIndex, ubiU32 nbRegisters, const ubiVector4I* integers)
	{
		SetPixelShaderConstantI(registerIndex, (const int*)integers, nbRegisters);
	}

	inline void ApplyBoolConstantsVS() {}

	inline void SetBoolVS(ShaderConstantEnum constant, const ubiU32& thebool)
	{
		SetVertexShaderConstantB(m_VertexShaderConstantMap->GetConstantIndex(constant), (const BOOL*)&thebool, 1);
	}

	inline void SetBoolVS(ubiU32 registerIndex, const ubiU32& thebool)
	{
		SetVertexShaderConstantB(registerIndex, (const BOOL*)&thebool, 1);
	}

	inline void SetBoolsVS(ubiU32 registerIndex, ubiU32 nbRegisters, const ubiU32* thebools)
	{
		SetVertexShaderConstantB(registerIndex, (const BOOL*)thebools, nbRegisters);
	}

	inline void SetBoolPS(ubiU32 registerIndex, ubiU32 thebool)
	{
		SetPixelShaderConstantB(registerIndex, (const BOOL*)&thebool, 1);
	}

	inline void SetBoolPS(ubiU32 registerIndex, ubiBool thebool)
	{
		BOOL b = thebool;
		SetPixelShaderConstantB(registerIndex, &b, 1);
	}

	inline void SetBoolPS(ShaderConstantEnum constant, ubiU32 thebool)
	{
		SetPixelShaderConstantB(m_PixelShaderConstantMap->GetConstantIndex(constant), (const BOOL*)&thebool, 1);
	}

	inline void SetBoolPS(ShaderConstantEnum constant, const ubiBool thebool)
	{
		BOOL b = thebool;
		SetPixelShaderConstantB(m_PixelShaderConstantMap->GetConstantIndex(constant), &b, 1);
	}

	inline void SetBoolsPS(ubiU32 registerIndex, ubiU32 nbRegisters, const ubiU32* thebools)
	{
		SetPixelShaderConstantB(registerIndex, (const BOOL*)thebools, nbRegisters);
	}

	inline void BeginSetMatricesVS(ubiU32 registerIndex, ubiMatrix44** matrices, ubiU32 nbMatrices)
	{
		GpuBeginVertexShaderConstantF4(registerIndex, (float**)matrices, nbMatrices * 4);
	}

	inline void BeginSetMatricesVS(ubiU32 registerIndex, ubiMatrix43** matrices, ubiU32 nbMatrices)
	{
		GpuBeginVertexShaderConstantF4(registerIndex, (float**)matrices, nbMatrices * (sizeof(ubiMatrix43) / sizeof(ubiVector4)));
	}
	inline void EndSetMatricesVS()

	{
		GpuEndVertexShaderConstantF4();
	}

	inline ubiMatrix44* BeginSetMatricesPS(ubiU32 registerIndex, ubiMatrix44** matrices, ubiU32 nbMatrices)
	{
		GpuBeginPixelShaderConstantF4(registerIndex, (float**)matrices, nbMatrices * 4);
		return *matrices; //@ZJ add return value
	}

	inline void EndSetMatricesPS()
	{
		GpuEndPixelShaderConstantF4();
	}

	// @@RVH: values from ACBF:
	// Those number are WIP - they work well for the shadow maps
	// Untested with all the other interface that use them
	// todo - make this a little cleaner and generic - not sure it would work well with modified value of shadow maps bias+slope
	ubiFloat g_mulValueBias = 1 << 24; // @@RVH: modified, as 22 was giving issues
	ubiFloat g_mulValueSlope = 15.0f;

	inline void SetDepthBiasInner(const ubiFloat slopesScaleDepthBias, const ubiFloat depthBias)
	{
		// @@guoxx: copy from ACBF
		SetRenderState(GFX_RS_SLOPESCALEDEPTHBIAS, raw_cast<DWORD>(slopesScaleDepthBias * g_mulValueSlope));
		// LB: From DX11 doc this formula works only for D24 DepthStencil buffers. Clamp to 16 bits due to CompressedRenderStates (assume this is sufficient...)
		// TODO_DX11: make this work for all kind of DepthBuffer

		ubiFloat val = depthBias * (ubiFloat)g_mulValueBias;
		SetRenderState(GFX_RS_DEPTHBIAS, raw_cast<DWORD>(val));
	}

	inline void SetDepthBias(const ubiFloat slopesScaleDepthBias, const ubiFloat depthBias)
	{
#ifdef INVERSE_DEPTH
		if (GetInverseMode())
		{
			m_SlopeScaleDepthBias = -slopesScaleDepthBias;
			m_DepthBias = -depthBias;
		}
		else
#endif //INVERSE_DEPTH
		{
			m_SlopeScaleDepthBias = slopesScaleDepthBias;
			m_DepthBias = depthBias;
		}
		SetDepthBiasInner(m_SlopeScaleDepthBias, m_DepthBias);
	}

	inline void SetDepthBiasOffset(ubiFloat slopeScaleDepthOffset, ubiFloat depthoffset)
	{
		// @@guoxx: this function have empty implementation on ACBF, but we still keep it on ACR

		ubiFloat factor = m_SlopeScaleDepthBias;
		ubiFloat units = m_DepthBias;

#ifdef INVERSE_DEPTH
		if (GetInverseMode())
		{
			// Increase the depth bias
			factor *= (-slopeScaleDepthOffset);
			units += (-depthoffset);
		}
		else
#endif //INVERSE_DEPTH
		{
			// Increase the depth bias
			factor *= slopeScaleDepthOffset;
			units += depthoffset;
		}
		SetDepthBiasInner(factor, units);
	}

	inline void ResetDepthBiasOffset()
	{
		SetDepthBiasInner(m_SlopeScaleDepthBias, m_DepthBias);
	}

	static inline void SetOverlayMaterialDepthBias(const ubiFloat bias)
	{
		ms_OverlayMaterial_DepthBias = bias;
	}

	static inline void SetOverlayMaterialDepthScale(const ubiFloat scale)
	{
		ms_OverlayMaterial_SlopeScaleDepthBias = scale;
	}


	static ubiFloat GetOverlayMaterialDepthBias()
	{
		return ms_OverlayMaterial_DepthBias;
	}

	static ubiFloat GetOverlayMaterialDepthScale()
	{
		return ms_OverlayMaterial_SlopeScaleDepthBias;
	}

	void SetPixelShader(PixelShaders pixelShader);
	void SetPixelShader(scimitar::PixelShader* pixelShader);

	void SetVertexShader(VertexShaders vertexShader);
	void SetVertexShader(scimitar::VertexShader* vertexShader);

	inline void SetVertexShaderConstantMap(GNMShaderConstantMap* map)
	{
		m_VertexShaderConstantMap = map;
	}

	inline void SetPixelShaderConstantMap(GNMShaderConstantMap* map)
	{
		m_PixelShaderConstantMap = map;
	}

	inline GNMShaderConstantMap* GetVertexShaderConstantMap()
	{
		return m_VertexShaderConstantMap;
	}

	inline GNMShaderConstantMap* GetPixelShaderConstantMap()
	{
		return m_PixelShaderConstantMap;
	}

	inline void SetIsUsingExDevice(ubiBool val)
	{
		m_isUsingExDevice = val;
	}

	inline GnmContext * GetGNMContext()
	{
		return m_ContextToFill;
	}

	void InitContext();
#ifndef USING_SEPARATE_CUE_HEAP
	void *GetCueHeap(void);
	ubiU32 GetNumRingEntries() const { return m_NumRingEntries; }
#endif
	GnmContext* ResetGnmContext();
	void Flip(ubiBool waitUtilGPUIdle = false);
	GnmContext *CreateNewContext();
	void DestoryContext(GnmContext *a_context);
	GnmContext *GetContext(ubiU32 a_contextHandle);
	uint32_t *GetCurrentDcbCmdPtr(ubiU32 a_allocationPlan);
	uint32_t *GetCurrentCcbCmdPtr(ubiU32 a_allocationPlan);
	void UpdateCmdPtr(uint32_t *a_dcbPtr, uint32_t *a_ccbPtr);

	/////////////////////////////////////////////////////////////////////////////////////////
	// GNMState - End
	/////////////////////////////////////////////////////////////////////////////////////////

	void ReleaseSamplers();
	void DeleteResources();

	// state objects (internal and gnm)

	inline ubiInt GetBaseVertexIndex() const { return m_BaseVertexIndex; }

private:
	void InitStates(ubiBool bInit = false);
	void CreateResources();
	void CreateStates();
	//----

	void InitMSAASettings();
	void InitCRAASetting();
	void SetupMSAASetting(GFX_MULTISAMPLE_TYPE msaaType);

	template<typename T, void(GNMDevice::*)(unsigned, const T*, unsigned)> inline void update_shader_constants(unsigned start_reg_, T *cache_, const T *src_, unsigned num_regs_);
	INLINE void SetVSC(unsigned start_reg_, const ubiVector4*, unsigned num_regs_);
	INLINE void SetVSC(unsigned start_reg_, const ubiVector4I*, unsigned num_regs_);
	INLINE void SetVSC(unsigned start_reg_, const bool*, unsigned num_regs_);
	INLINE void SetPSC(unsigned start_reg_, const ubiVector4*, unsigned num_regs_);
	INLINE void GetPSC(unsigned start_reg_, ubiVector4*, unsigned num_regs_);
	INLINE void SetPSC(unsigned start_reg_, const ubiVector4I*, unsigned num_regs_);
	INLINE void SetPSC(unsigned start_reg_, const bool*, unsigned num_regs_);
	//------------------------------------------------------------------------

	// Prepare and clean for each draw calls
	INLINE void SetPrimitiveTopology(GFXPRIMITIVETYPE primType);
	INLINE void PreDraw();
	INLINE void Draw(ubiU32 vertexCount, ubiU32 startVertexLocation);
	INLINE void DrawIndexed(ubiU32 indexCount, ubiU32 startIndexLocation, ubiU32 baseVertexLocation);
	INLINE void AfterDraw();

	ubiU32				m_NumRingEntries;

	ubiU32				m_DcbMemorySize;
	ubiU32				m_CcbMemorySize;

	ubiU32								m_ConextsCount;
	ubiU32								m_NextContextHandle;
	GnmContext *						m_FirstContext;
	GnmContext *						m_ContextToFill;
	PtrArray<GnmCommandList>			m_PendingCommandLists; //@@LRF shared between contexts.

#ifndef USING_SEPARATE_CUE_HEAP
	void *				m_CueHeapMemory[MAX_CUE_HEAPS];	//@@LRF shared between contexts.
	ubiU32				m_CueHeapIndex;
#endif

	//@@LRF the big CMD buffer will be shared between contexts.
	void *				m_DcbMemory[NumSwapchainBuffers];
	void *				m_CcbMemory[NumSwapchainBuffers];
	uint32_t *			m_DcbBeginPtr[NumSwapchainBuffers];
	uint32_t *			m_DcbCmdPtr{ nullptr };
	uint32_t *			m_DcbEndPtr[NumSwapchainBuffers];
	uint32_t *			m_CcbBeginPtr[NumSwapchainBuffers];
	uint32_t *			m_CcbCmdPtr{ nullptr };
	uint32_t *			m_CcbEndPtr[NumSwapchainBuffers];

	uint32_t			m_CurrentDcbMemoryIndex;
	uint32_t			m_CurrentCcbMemoryIndex;

	GfxStateManager*	m_StateManager;

	ubiBool             m_isUsingExDevice;

	ubiVector4 m_vsc_f[max_vsc_f], m_psc_f[max_psc_f];

	ubiU32                  m_SamplerChanged;

#ifdef POP_USE_D3DSTATECACHE
	// State cache used for PC only.
	// We have determined that it is actually slower to use on Xenon.
	DWORD                   m_CurrentSamplerState[max_samplers][max_samplerstates];
	DWORD                   m_CurrentVSamplerState[max_vsamplers][max_samplerstates];
	DWORD                   m_CurrentRenderState[max_renderstates];
	PlatformGfxVertexShader* m_CurrentVertexShader;
	PlatformGfxPixelShader*  m_CurrentPixelShader;
	IDirect3DSurface9*      m_CurrentRenderTarget[4];
	IDirect3DSurface9*      m_CurrentDepthStencilSurface;
	D3DVIEWPORT9            m_CurrentViewPort;
	GfxRect                 m_CurrentScissorRect;
	PlatformGfxBaseTexture*  m_CurrentPixelTexture[max_samplers];
	PlatformGfxBaseTexture*  m_CurrentVertexTexture[max_vsamplers];
	ubiU32 m_CurrentVertexBufferOffset[max_vstreams];
	ubiU32 m_CurrentVertexBufferStride[max_vstreams];
	PlatformGfxBaseBuffer* m_CurrentVertexBuffer[max_vstreams];
	PlatformGfxBaseBuffer*  m_CurrentIndexBuffer;
	PlatformGfxInputLayout* m_CurrentVertexDeclaration;
#endif

#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
	enum GlobalConstantCheckMode
	{
		SettingGlobalConstants,
		CheckingGlobalConstantsOverride,
		NotCheckingGlobalConstantsOverride
	};

	void InitGlobalConstantsCheck();

	void ValidateConstantOperation(ubiBool pixelConstant, ubiU32 index, ubiU32 range);
	void InvalidateConstantOperation(ubiBool pixelConstant, ubiU32 index, ubiU32 range);

	ubiBool     m_VS_LockedConstants[max_vsc_f];
	ubiBool     m_PS_LockedConstants[max_psc_f];
	GlobalConstantCheckMode m_GlobalConstantMode;
	InplaceArray<GlobalConstantCheckMode, 4> m_ModeStack;

#endif //POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE

	// high-level state wrapper members
	GNMShaderConstantMap*   m_PixelShaderConstantMap;
	GNMShaderConstantMap*   m_VertexShaderConstantMap;
	ShaderManager*          m_ShaderManager;
	static ubiU32   m_BlendModeToDstBlend[NbBlendModes];
	static ubiU32   m_BlendModeToSrcBlend[NbBlendModes];

	// overrides
	GfxexDeviceOptions*       m_DeviceOptions;


	//
	ubiFloat m_SlopeScaleDepthBias;
	ubiFloat m_DepthBias;

#ifdef EMULATE_GPUBEGIN_API
	// GpuBegin* API emulation for GNM PC
	ubiU32 m_GPUBeginVSIndex;
	ubiU32 m_GPUBeginVSCount;
	ubiU32 m_GPUBeginPSIndex;
	ubiU32 m_GPUBeginPSCount;
#endif

#ifdef EMULATE_BEGINVERTICES_API
	// Begin*Vertices API emulation for GNM PC
	GFXPRIMITIVETYPE    m_BeginVerticesPrimType;
	ubiU32              m_BeginVerticesBaseVtxIndex;
	ubiU32              m_BeginVerticesVertexCount;
	ubiU32              m_BeginVerticesIndexCount;
	GFXFORMAT         m_BeginVerticesIndexFormat;
	ubiU32              m_BeginVerticesStreamStride;

	Gear::RefCountedPtr<GNMWrappedResource::VertexBuffer> m_DynamicVB;
	Gear::RefCountedPtr<GNMWrappedResource::IndexBuffer> m_DynamicIB;
	ubiU32 m_DynamicVBOffset;
	ubiU32 m_DynamicIBOffset;
	GFXFORMAT m_DynamicIBFormat;
#endif // EMULATE_BEGINVERTICES_API

#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
	static IDirect3DStateBlock9* m_ResetStateBlock;
	DWORD						 m_InitSamplerState[max_samplers][max_samplerstates];
	DWORD						 m_InitVSamplerState[max_vsamplers][max_samplerstates];
	DWORD						 m_InitRenderState[max_renderstates];
#endif

	ubiU32                       m_LastIndexCount;
	ubiInt                       m_BaseVertexIndex;
	ubiBool                      m_IsDeferredDevice;
	ubiBool                      m_IsRecording;
	ubiU32						 m_NumDrawCalls;

	struct MSAA_SAMPLE_POSITION
	{
		int8_t x : 4;
		int8_t y : 4;
	};

	struct MSAA_SAMPLE_POSITIONS
	{
		union
		{
			struct
			{
				MSAA_SAMPLE_POSITION  SampleLocs00[16];  // Sample positions stored as one byte per component, X then Y
				MSAA_SAMPLE_POSITION  SampleLocs10[16];  // There are up to 16 possible sample positions per pixel
				MSAA_SAMPLE_POSITION  SampleLocs01[16];  // They can be set differently for each pixel in a 2x2 quad
				MSAA_SAMPLE_POSITION  SampleLocs11[16];
			};
			uint32_t SampleLocs[16];
		};
	};

	struct MSAASetting
	{
		MSAA_SAMPLE_POSITIONS m_SamplePositions;
	};

	MSAASetting                        m_MSAASettings[GFX_MULTISAMPLE_MAX];
	ubiBool	                           m_ResetMSAASetting;
	GFX_MULTISAMPLE_TYPE               m_MultiSampleType;
	void*                              m_CRAA8xLutAddress;
	Gear::RefCountedPtr<GfxBufferView> m_CRAA8xLutView;

public:
#ifdef INVERSE_DEPTH
	static ubiBool		  m_bInversedMode;
#endif //INVERSE_DEPTH
	static ubiFloat       ms_OverlayMaterial_DepthBias;
	static ubiFloat       ms_OverlayMaterial_SlopeScaleDepthBias;
};

// #ifdef POP_PLATFORM_WIN32
//     // double-check the GFX_RENDER_STATE_TYPE and GFX_SAMPLER_STATE_TYPE enums in the
//     // SDK headers if this fires, make sure GFX_RS_MAX and GFX_SAMP_MAX below are big enough!
//     popSTATIC_ASSERT(DIRECT3D_VERSION == 0x900);
// #endif

//----------------------------------------------------------------------------
struct DefaultSamplerState
{
	GFX_SAMPLER_STATE_TYPE m_SamplerState;
	DWORD   m_DefaultValue;
};

#define POP_NB_DEFAULTSAMPLERSTATES 6

extern DefaultSamplerState g_DefaultSamplerStates[];

popEND_NAMESPACE

#include "graphic/gfx/gfxExtension/gfxexcommandbuffer.h"

//============================================================================

#include "gnmwrapper.inl"



#endif // __GRAPHIC__GNM__GNMWRAPPER__H__


//============================================================================
// gnmwrapper.inl
//
// Copyright(C) Ubisoft
//============================================================================
#ifndef __GNMWRAPPER_INL__
#define __GNMWRAPPER_INL__


#include "graphic/gnm/gnmstatemanager.h"
#include "graphic/gnm/gnmcontext.h"

#include "graphic/gfx/gfxExtension/gfxexrendersurface.h"
#include "graphic/gfx/gfxExtension/gfxexconstantsbuffer.h"


popBEGIN_NAMESPACE

#ifdef POP_PLATFORM_XENON
extern D3DPRESENT_PARAMETERS g_DevicePresentParams;
#endif

#ifdef POP_PLATFORM_XENON
#define popSetCmdBufferFence(d3dresouce)    SetResourceFence(d3dresouce);
#else
#define popSetCmdBufferFence(d3dresouce)
#endif

//============================================================================
// GNMDevice
//============================================================================
inline ubiBool GNMDevice::IsDeferred() const
{
	return m_IsDeferredDevice;
}

//----------------------------------------------------------------------------
inline PtrArray<GnmCommandList> & GNMDevice::GetPendingCommandLists()
{
	return m_PendingCommandLists;
}

//----------------------------------------------------------------------------
inline void GNMDevice::PushPendingCommandList(GnmCommandList *cmdList)
{
	m_PendingCommandLists.Push(cmdList);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetRenderState(GFX_RENDER_STATE_TYPE state, DWORD value)
{
	m_StateManager->SetRenderState(state, value);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetSamplerState(ubiU32 sampler, GFX_SAMPLER_STATE_TYPE state, DWORD value)
{
	m_SamplerChanged |= (1 << sampler);
	m_StateManager->SetSamplerState(GFX_PIXEL_SHADER, sampler, state, value);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetVSamplerState(ubiU32 sampler, GFX_SAMPLER_STATE_TYPE state, DWORD value)
{
	m_StateManager->SetSamplerState(GFX_VERTEX_SHADER, sampler, state, value);
}

//----------------------------------------------------------------------------
#if !GNM_DISABLED
inline void GNMDevice::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	popLog(LogSpaceGraphic, LogLevel1, StringFormat("@@guoxx: SetTextureStageState not supported yet"));
}
#endif

inline void GNMDevice::SetTexture(ubiU32 sampler, GNMTexture* texture)
{
	SetTexture(sampler, texture->GetShaderResourceView());
}

inline void GNMDevice::SetTexture(ubiU32 sampler, GfxTextureView* texture)
{
	m_StateManager->SetTexture(GFX_PIXEL_SHADER, sampler, texture);
}

inline void GNMDevice::SetBuffer(ubiU32 sampler, GfxBufferView* buffer)
{
	m_StateManager->SetBuffer(sampler, buffer);
}

inline void GNMDevice::UnsetTexture(ubiU32 sampler)
{
	m_StateManager->SetTexture(GFX_PIXEL_SHADER, sampler, NULL);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetVTexture(ubiU32 sampler, GfxTextureView* texture)
{
	m_StateManager->SetTexture(GFX_VERTEX_SHADER, sampler, texture);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetStreamSource(ubiU32 stream, const PlatformGfxBaseBuffer* vb, ubiU32 offset, ubiU32 stride)
{
	m_StateManager->SetVertexBuffer(stream, vb, offset, stride);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetStreamSource(ubiU32 stream, const Gear::RefCountedPtr<GNMWrappedResource::VertexBuffer>& vb, ubiU32 offset, ubiU32 stride)
{
	SetStreamSource(stream, vb.GetPtr(), offset, stride); //@@LRF pass the original ptr, this is not safe, to be refactored, GNM_PORTING_TODO
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetIndices(const PlatformGfxBaseBuffer *ib, GFXFORMAT fmt, ubiU32 offset)
{
	m_StateManager->SetIndexBuffer(ib, fmt, offset);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetIndices(const Gear::RefCountedPtr<GNMWrappedResource::IndexBuffer>& ib, ubiU32 offset)
{
	SetIndices(ib.GetPtr(), (ib == nullptr ? GFXFMT_R16_UINT : ib->GetFormat()), offset);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetRenderTarget(ubiU32 rtIndex, GfxexSurface* rt)
{
	m_StateManager->SetRenderTarget(rtIndex, rt);

	// @@guoxx: assume render target 0 is the main render target
	if (rtIndex == 0 && rt != nullptr)
	{
		m_ResetMSAASetting = true;
		m_MultiSampleType = rt->GetMultisampleType();
	}
}

//----------------------------------------------------------------------------
inline void GNMDevice::GetRenderTarget(ubiU32 rtIndex, GfxexSurface** rt)
{
	m_StateManager->GetRenderTarget(rtIndex, rt);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetDepthStencilSurface(GfxexSurface* ds)
{
	m_StateManager->SetDepthStencilTarget(ds);

	if (ds != nullptr)
	{
		// @@guoxx: depth buffer multi sample configuration will overwrite settings from color buffer
		// we may use multi sampled depth but non multi sampled color buffer
		// anyway, you can't have correct multi sampled color buffer without correct configurations for depth buffer
		m_ResetMSAASetting = true;
		m_MultiSampleType = ds->GetMultisampleType();
	}
}

//----------------------------------------------------------------------------
inline void GNMDevice::GetDepthStencilSurface(GfxexSurface** ds)
{
	m_StateManager->GetDepthStencilTarget(ds);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetViewport(const PlatformGfxViewport& vp)
{
	m_StateManager->SetViewport(vp);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetScissorRect(const GFX_RECT& rect)
{
	m_StateManager->SetScissorRect(rect);
}

//----------------------------------------------------------------------------
inline void GNMDevice::GetViewport(PlatformGfxViewport* vp)
{
	m_StateManager->GetViewport(*vp);
}

//----------------------------------------------------------------------------
inline const PlatformGfxViewport& GNMDevice::GetViewport()const
{
	return m_StateManager->GetViewport();
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
inline void GNMDevice::SetVertexShader(const Gear::RefCountedPtr<GNMWrappedResource::VertexShader>& vs)
{
	m_StateManager->SetVertexShader(vs);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetVertexDeclaration(PlatformGfxInputLayout* vd)
{
	m_StateManager->SetVertexDeclaration(vd);
}

//----------------------------------------------------------------------------
inline PlatformGfxInputLayout* GNMDevice::GetVertexDeclaration()
{
	return m_StateManager->GetVertexDeclaration();
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetPixelShader(const Gear::RefCountedPtr<GNMWrappedResource::PixelShader>& ps)
{
	m_StateManager->SetPixelShader(ps);
}

//----------------------------------------------------------------------------
inline void GNMDevice::GpuBeginVertexShaderConstantF4(ubiU32 start, float** data, unsigned vector4Count)
{
	// register index must be a multiple of four
	popAssert((start & 3) == 0);

#ifdef EMULATE_GPUBEGIN_API
	// GpuBegin* API emulation for GNM PC
	popAssert(m_GPUBeginVSCount == 0);
	*data = m_vsc_f[start];
	m_GPUBeginVSIndex = start;
	m_GPUBeginVSCount = vector4Count;
#else
	// Xbox360-specific API to update shader constants
	m_device->GpuOwnVertexShaderConstantF(start, vector4Count);
	GNM_CALL(m_device->GpuBeginVertexShaderConstantF4(start, (D3DVECTOR4**)data, vector4Count));
	popAssert(*data != NULL);
#endif
}

//----------------------------------------------------------------------------
inline void GNMDevice::GpuEndVertexShaderConstantF4()
{
#ifdef EMULATE_GPUBEGIN_API
	// GpuBegin* API emulation for GNM PC
	popAssert(m_GPUBeginVSCount != 0);
	m_StateManager->GetVSConstantsBuffer()->SetVectorsF(m_GPUBeginVSIndex, m_GPUBeginVSCount, &m_vsc_f[m_GPUBeginVSIndex]);
	m_GPUBeginVSCount = 0;
#else
	// Xbox360-specific API to update shader constants
	m_device->GpuEndVertexShaderConstantF4();
#endif
}

//----------------------------------------------------------------------------
inline void GNMDevice::GpuBeginPixelShaderConstantF4(ubiU32 start, float** data, unsigned vector4Count)
{
	// register index must be a multiple of four
	popAssert((start & 3) == 0);

#ifdef EMULATE_GPUBEGIN_API
	// GpuBegin* API emulation for GNM PC
	popAssert(m_GPUBeginPSCount == 0);
	*data = m_psc_f[start];
	m_GPUBeginPSIndex = start;
	m_GPUBeginPSCount = vector4Count;
#else
	// Xbox360-specific API to update shader constants
	m_device->GpuOwnPixelShaderConstantF(start, vector4Count);
	GNM_CALL(m_device->GpuBeginPixelShaderConstantF4(start, (D3DVECTOR4**)data, vector4Count));
	popAssert(*data != NULL);
#endif
}

//----------------------------------------------------------------------------
inline void GNMDevice::GpuEndPixelShaderConstantF4()
{
#ifdef EMULATE_GPUBEGIN_API
	// GpuBegin* API emulation for GNM PC
	popAssert(m_GPUBeginPSCount != 0);
	m_StateManager->GetPSConstantsBuffer()->SetVectorsF(m_GPUBeginPSIndex, m_GPUBeginPSCount, &m_psc_f[m_GPUBeginPSIndex]);
	m_GPUBeginPSCount = 0;
#else
	// Xbox360-specific API to update shader constants
	m_device->GpuEndPixelShaderConstantF4();
#endif
}

//----------------------------------------------------------------------------
inline void GNMDevice::DrawPrimitive(GFXPRIMITIVETYPE pt, ubiU32 startVertex, ubiU32 primCount)
{
#ifndef DISABLE_RENDERING
	if (!m_StateManager->DrawCallGuard())
		return;

	SetPrimitiveTopology(pt);
	Draw(PrimitiveCountToIndexCount(pt, primCount), startVertex);
#endif
}

//----------------------------------------------------------------------------
inline void GNMDevice::DrawIndexedPrimitive(GFXPRIMITIVETYPE pt, ubiInt baseVertexIndex, ubiU32 minIndex, ubiU32 numIndices, ubiU32 startIndex, ubiU32 primCount)
{
#ifndef DISABLE_RENDERING
	if (!m_StateManager->DrawCallGuard())
		return;

	SetPrimitiveTopology(pt);
	DrawIndexed(PrimitiveCountToIndexCount(pt, primCount), startIndex, baseVertexIndex);
#endif
}

//----------------------------------------------------------------------------
inline void GNMDevice::Clear(ubiU32 numRects, const GfxRect* rects, DWORD flags, D3DCOLOR colorARBG, ubiFloat z, ubiU8 stencil)
{
	if (flags & GFX_CLEAR_COLOR)
	{
		for (ubiU32 i = 0; i < GNMDevice::max_rts; ++i)
		{
			GfxexSurface* rtv;
			m_StateManager->GetRenderTarget(i, &rtv);
			if (rtv != NULL)
			{
				ubiFloat a = ((colorARBG >> 24) & 0xFF) / 255.0f;
				ubiFloat r = ((colorARBG >> 16) & 0xFF) / 255.0f;
				ubiFloat g = ((colorARBG >> 8) & 0xFF) / 255.0f;
				ubiFloat b = ((colorARBG >> 0) & 0xFF) / 255.0f;
				ClearRenderTargetView(rtv, r, g, b, a);
			}
		}
	}

	GfxexSurface* dsv;
	m_StateManager->GetDepthStencilTarget(&dsv);
	if ((dsv != NULL) && ((flags & GFX_CLEAR_DEPTH) || (flags & GFX_CLEAR_STENCIL)))
	{
		ClearDepthStencilView(dsv, flags & GFX_CLEAR_DEPTH, z, flags & GFX_CLEAR_STENCIL, stencil);
	}
}

//----------------------------------------------------------------------------
inline void GNMDevice::BeginVertices(GFXPRIMITIVETYPE type, ubiU32 vertexCount, ubiU32 streamStride, void** vertexData)
{
	// Begin*Vertices API emulation for GNM PC
	m_BeginVerticesPrimType = type;
	m_BeginVerticesBaseVtxIndex = 0;
	m_BeginVerticesVertexCount = vertexCount;
	m_BeginVerticesIndexCount = 0;
	m_BeginVerticesStreamStride = streamStride;

	ubiU32 lockSize = streamStride * vertexCount;
	GFX_MAP lockFlags = GFX_NOOVERWRITE;

	// if this fires, consider splitting your primitives (or increase the dynamic VB size...)
	popAssertWithMsg(lockSize < DynamicVertexBufferSize, "GNMDevice::BeginIndexedVertices: Too many primitives");

	if (m_DynamicVBOffset + lockSize > DynamicVertexBufferSize)
	{
		// no more space in the dynamic VB, restart from the beginning
#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
		lockFlags = GFX_NOOVERWRITE;
#else
		lockFlags = GFX_DISCARD;
#endif
		m_DynamicVBOffset = 0;
	}

	ubiU32 pitchVB;
	m_DynamicVB->Lock(this, pitchVB, vertexData, lockFlags);
	//MapResource(m_DynamicVB, 0, *vertexData, rowPitch, depthPicth, lockFlags);
	*vertexData = GfxexBufferOffset(*vertexData, m_DynamicVBOffset);
	//popAssert((rowPitch - m_DynamicVBOffset) >= lockSize);
}

//----------------------------------------------------------------------------
inline void GNMDevice::EndVertices()
{
	// Begin*Vertices API emulation for GNM PC
	m_DynamicVB->Unlock();
	//UnmapResource(m_DynamicVB, 0);


	SetStreamSource(0,
		m_DynamicVB,
		m_DynamicVBOffset,
		m_BeginVerticesStreamStride);

#ifndef DISABLE_RENDERING
	DrawPrimitive(m_BeginVerticesPrimType, m_BeginVerticesBaseVtxIndex, IndexCountToPrimitiveCount(m_BeginVerticesPrimType, m_BeginVerticesVertexCount));
#endif

	m_DynamicVBOffset += m_BeginVerticesVertexCount * m_BeginVerticesStreamStride;
}

//----------------------------------------------------------------------------
inline void GNMDevice::BeginIndexedVertices(GFXPRIMITIVETYPE type, ubiU32 baseVertexIndex, ubiU32 vertexCount, ubiU32 indexCount, GFXFORMAT indexFormat, ubiU32 streamStride, void** indexData, void** vertexData)
{
	// bm todo only 16bit indices supported for now
	m_DynamicIBFormat = GFXFMT_R16_UINT;

	// Begin*Vertices API emulation for GNM PC
	m_BeginVerticesPrimType = type;
	m_BeginVerticesBaseVtxIndex = baseVertexIndex;
	m_BeginVerticesVertexCount = vertexCount;
	m_BeginVerticesIndexCount = indexCount;
	m_BeginVerticesIndexFormat = indexFormat;
	m_BeginVerticesStreamStride = streamStride;

	ubiU32 lockSizeVB = streamStride * vertexCount;
	GFX_MAP lockFlagsVB = GFX_NOOVERWRITE;

	// if this fires, consider splitting your primitives (or increase the dynamic VB size...)
	popAssertWithMsg(lockSizeVB < DynamicVertexBufferSize, "GNMDevice::BeginIndexedVertices: Too many primitives");

	if (m_DynamicVBOffset + lockSizeVB > DynamicVertexBufferSize)
	{
		// no more space in the dynamic VB, restart from the beginning
#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
		lockFlagsVB = GFX_NOOVERWRITE;
#else
		lockFlagsVB = GFX_DISCARD;
#endif

		m_DynamicVBOffset = 0;
	}

	ubiU32 pitchVB;
	m_DynamicVB->Lock(this, pitchVB, vertexData, lockFlagsVB);
	//MapResource(m_DynamicVB, 0, *vertexData, rowPitchVB, depthPitchVB, lockFlagsVB);
	*vertexData = GfxexBufferOffset(*vertexData, m_DynamicVBOffset);
	//popAssert((rowPitchVB - m_DynamicVBOffset) >= lockSizeVB);

	ubiU32 lockSizeIB = 2 * indexCount;
	GFX_MAP lockFlagsIB = GFX_NOOVERWRITE;

	// if this fires, increase the dynamic IB size... (used on PC only so it doesn't really matter)
	popAssert(lockSizeIB < DynamicIndexBufferSize);

	if (m_DynamicIBOffset + lockSizeIB > DynamicIndexBufferSize)
	{
		// no more space in the dynamic IB, restart from the beginning
#if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
		lockFlagsIB = GFX_NOOVERWRITE;
#else
		lockFlagsIB = GFX_DISCARD;
#endif
		m_DynamicIBOffset = 0;
	}

	ubiU32 pitchIB;
	m_DynamicIB->Lock(this, pitchIB, indexData, lockFlagsIB);
	//MapResource(m_DynamicIB, 0, *indexData, rowPitchIB, depthPitchIB, lockFlagsIB);
	*indexData = GfxexBufferOffset(*indexData, m_DynamicIBOffset);
	//popAssert((rowPitchIB - m_DynamicIBOffset) > lockSizeIB);
}


//----------------------------------------------------------------------------
inline void GNMDevice::EndIndexedVertices()
{
	// Begin*Vertices API emulation for GNM PC
	m_DynamicVB->Unlock();
	m_DynamicIB->Unlock();
	//UnmapResource(m_DynamicVB, 0);
	//UnmapResource(m_DynamicIB, 0);


	SetIndices(m_DynamicIB.GetPtr(), m_DynamicIBFormat);

	SetStreamSource(0,
		m_DynamicVB,
		m_DynamicVBOffset,
		m_BeginVerticesStreamStride);

	ubiU32 indexSize = 2;

#ifndef DISABLE_RENDERING
	DrawIndexedPrimitive(m_BeginVerticesPrimType,
		m_BeginVerticesBaseVtxIndex,
		0,
		m_BeginVerticesVertexCount,
		m_DynamicIBOffset / indexSize,
		IndexCountToPrimitiveCount(m_BeginVerticesPrimType, m_BeginVerticesIndexCount));
#endif

	m_DynamicVBOffset += m_BeginVerticesVertexCount * m_BeginVerticesStreamStride;
	m_DynamicIBOffset += m_BeginVerticesIndexCount * indexSize;
}

//----------------------------------------------------------------------------
inline void GNMDevice::EndIndexedVertices_NoDraw(ubiU32& currentVBOffset, ubiU32& currentIBOffset)
{
#ifdef EMULATE_BEGINVERTICES_API
	m_DynamicVB->Unlock();
	m_DynamicIB->Unlock();
	//UnmapResource(m_DynamicVB, 0);
	//UnmapResource(m_DynamicIB, 0);

	currentVBOffset = m_DynamicVBOffset;
	currentIBOffset = m_DynamicIBOffset;

	m_DynamicVBOffset += m_BeginVerticesVertexCount * m_BeginVerticesStreamStride;
	m_DynamicIBOffset += m_BeginVerticesIndexCount * 2;
#endif

}

//----------------------------------------------------------------------------
inline ubiU32 GNMDevice::GetMaxDynamicVertexCount(ubiU32 streamStride)
{
	// emulation is only limited by the dynamic VB size
	return (DynamicVertexBufferSize / streamStride);
}

//----------------------------------------------------------------------------
inline void GNMDevice::SetPrimitiveTopology(GFXPRIMITIVETYPE primType)
{
	m_ContextToFill->setPrimitiveType(primType);
}

inline void GNMDevice::PreDraw()
{
	m_StateManager->CompileStates();
	m_StateManager->CommitStates(m_ContextToFill);
	if (m_ResetMSAASetting)
	{
		SetupMSAASetting(m_MultiSampleType);
		m_ResetMSAASetting = false;
	}
}

inline void GNMDevice::AfterDraw()
{
	//m_StateManager->GetDirtyFlags().GraphicsPipeDirty = true;
	m_NumDrawCalls += 1;

#ifdef VALIDATE_DCB_BEFORE_SUBMISSION
	m_ContextToFill->ValidateResult(m_context->validate());
#endif

#ifndef GNM_PORTING_TODO
#ifdef TRACK_GPU_PRIMITIVES
	m_ContextToFill->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, (void*)ms_LastPrimitiveGPU, Gear::Atomic::Add(ms_LastPrimitiveCPU, 1), Gnm::kCacheActionNone);
#endif
#endif
	// tc todo - we shouldn't have to kick here every time...
#ifdef FORCE_KICK_COMMANDERBUFFER
	KickCommandBuffer();
#endif // FORCE_KICK_COMMANDERBUFFER
}

inline void GNMDevice::Draw(ubiU32 vertexCount, ubiU32 startVertexLocation)
{
#ifndef DISABLE_RENDERING
	m_LastIndexCount = vertexCount;
	m_BaseVertexIndex = startVertexLocation;
	PreDraw();
	m_ContextToFill->drawIndexAuto(m_LastIndexCount);
	AfterDraw();
#endif
}

inline void GNMDevice::DrawIndexed(ubiU32 indexCount, ubiU32 startIndexLocation, ubiU32 baseVertexLocation)
{
#ifndef DISABLE_RENDERING

#if defined(TEMP_SKIP_ERROR_VS) && !defined (POP_FINAL)
	if (strcmp(m_StateManager->GetVertexShader()->m_hashCode, "758202C3 00000000") == 0)
	{
		return;
	}
#endif // TEMP_SKIP_ERROR_VS

	m_LastIndexCount = indexCount;
	m_BaseVertexIndex = baseVertexLocation;
	PreDraw();
	{
		Gear::AdaptiveLock& t_lock = m_StateManager->GetIndexBuffer()->GetLock();
		popAutoLock(t_lock);

		ubiU8* indicesAddr = (ubiU8*)m_StateManager->GetIndexBuffer()->GetBuffer()->getBaseAddress();
		indicesAddr += startIndexLocation * m_StateManager->GetCurrentIndexSizeInBytes();

		ubiU32 numIndicesInBuffer = (m_StateManager->GetIndexBuffer()->GetSizeInBytes() / m_StateManager->GetCurrentIndexSizeInBytes()) - startIndexLocation;
		popAssertWithMsg(numIndicesInBuffer >= m_LastIndexCount, "IndexBuffer too small!");

		m_ContextToFill->drawIndex(m_LastIndexCount, indicesAddr);
	}

	AfterDraw();
#endif
}

#ifndef POP_USE_D3DSTATECACHE
inline ubiU32 GNMDevice::GetRenderState(GFX_RENDER_STATE_TYPE state)
{
	return m_StateManager->GetRenderState(state);
}
#endif



#ifdef POP_USE_PROFILER
inline void GNMDevice::PushMarker(const ubiChar* marker)
{
	if (m_ContextToFill)
	{
		m_ContextToFill->pushMarker(marker);
	}
}


inline void GNMDevice::PopMarker()
{
	if (m_ContextToFill)
	{
		m_ContextToFill->popMarker();
	}
}
#endif

inline void GNMDevice::StartRecordingCommands()
{
	popAssert(m_IsDeferredDevice);
	popAssert(m_IsRecording == false);
	popAssert(m_NumDrawCalls == 0);

	m_IsRecording = true;

	// we are starting a new command list so invalidate our internal state cache to make sure we commit all states to the HW
	m_StateManager->FullResetStates();

	m_ContextToFill->StartCommandList();
}

inline GfxCmdList* GNMDevice::FinishRecordingCommands()
{
	popAssert(m_IsDeferredDevice);
	popAssert(m_IsRecording == true);
	m_IsRecording = false;

	GfxCmdList* cmdList = m_ContextToFill->EndCommandList();
	if (cmdList != nullptr)
	{
		ResetGnmContext();
	}
	return cmdList;
}

inline void GNMDevice::ExecuteRecordedCommands(GfxCmdList* cmdList)
{
	popAssert(!m_IsDeferredDevice);

	if (cmdList == nullptr)
	{
		return;
	}
	GnmCommandList *t_CmdList = m_ContextToFill->ExecuteCommandList(cmdList);
	if (t_CmdList != nullptr)
	{
		ResetGnmContext();
	}
}

inline void GNMDevice::BeginCommandListExecution()
{
	popAssert(!m_IsDeferredDevice);

	//@@LRF Do nothing on GNM
}

inline void GNMDevice::EndCommandListExecution()
{
	popAssert(!m_IsDeferredDevice);

	//@@LRF Do nothing on GNM
}

popEND_NAMESPACE
#endif