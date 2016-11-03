#include "stdafx.h"

#include "RenderContext.h"
#include "GraphicDevice.h"
#include "GPUResourceViews.h"
#include "RenderSurface.h"
#include "RenderableTexture.h"
#include "Swapchain.h"
#include "OutputDevice.h"

Framework::RenderContext::RenderContext(GraphicDevice *device)
	: mDevice(device)
{

}

Framework::RenderContext::~RenderContext()
{

}

void Framework::RenderContext::roll()
{
	// stall the CPU until the GPU is finished with the context chunk we're entering into
}

void Framework::RenderContext::reset()
{


	//if (isMainContext)
	{
		//->initializeDefaultHardwareState();
		//->waitUntilSafeForRendering(mDevice->getOutput()->getHandle(), mDevice->getSwapChain()->getCurrentBufferIndex());
	}
}

void Framework::RenderContext::submitAndFlip(bool asynchronous)
{
	OutputDevice::DeviceHandle _handle = mDevice->getOutput()->getHandle();
	SceVideoOutFlipMode _flipMode = mDevice->getSwapChain()->getDescription().mFilpMode;
	U32 _displayBufferIndex = mDevice->getSwapChain()->getCurrentBufferIndex();
	// A user - provided argument with no internal meaning.The <c><i>flipArg< / i>< / c> associated with the most recently completed flip is
	// included in the <c>SceVideoOutFlipStatus< / c> object retrieved by <c>sceVideoOutGetFlipStatus() < / c > ; it could therefore
	// be used to uniquely identify each flip.
	U64 _flipArg = mDevice->getSwapChain()->getFrameCount();



	// TODO parse and report the validation error
	//SCE_GNM_ASSERT_MSG(ret == sce::Gnm::kSubmissionSuccess, "Command buffer validation error.");
}

void Framework::RenderContext::appendLabelAtEOPWithInterrupt(void *dstGpuAddr, U64 value)
{
	SCE_GNM_ASSERT(dstGpuAddr != nullptr);
	//->writeImmediateAtEndOfPipeWithInterrupt(Gnm::kEopFlushCbDbCaches, dstGpuAddr, value, Gnm::kCacheActionNone);
}

void Framework::RenderContext::appendLabelAtEOP(void *dstGpuAddr, U64 value)
{
	SCE_GNM_ASSERT(dstGpuAddr != nullptr);
	//->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, dstGpuAddr, value, Gnm::kCacheActionNone);
}

void Framework::RenderContext::setTextureSurface(U32 soltID, const RenderSurface *surface)
{
	TextureView *_view = surface->getTexture()->getShaderResourceView();
	SCE_GNM_ASSERT_MSG(_view != nullptr, "failed to get correct view from surface");
}

void Framework::RenderContext::setRenderTargetSurface(U32 soltID, const RenderSurface *surface)
{
	RenderTargetView *_view = typeCast<BaseTargetView, RenderTargetView>(surface->getTexture()->getTargetView());
	SCE_GNM_ASSERT_MSG(_view != nullptr, "failed to get correct view from surface");
}

void Framework::RenderContext::setDepthStencilTargetSurface(const RenderSurface *surface)
{
	DepthStencilView *_view = typeCast<BaseTargetView, DepthStencilView>(surface->getTexture()->getTargetView());
	SCE_GNM_ASSERT_MSG(_view != nullptr, "failed to get correct view from surface");
}

void Framework::RenderContext::setViewport(U32 x, U32 y, U32 width, U32 height, Float32 minz /*= 0.0f*/, Float32 maxz /*= 1.0f*/)
{

}


////////////////////////////////////////////////////////////////


//============================================================================
// gnmwrapper.cpp
//
// Copyright(C) Ubisoft
//============================================================================

#include "graphic/precomp.h"

#ifdef POP_PLATFORM_GNM

#include "graphic/neoutils.h"

#include "graphic/gnm/gnmwrapper.h"

#include "graphic/gfx/gfxshadermanager.h"
#include "graphic/gnm/gnmocclusionquery.h"
#include "graphic/common/memtagdisplay.h"
#include "graphic/gnm/gnmstatemanager.h"
#include "graphic/gnm/orbisgpufence.h"
#include "../shaders/shaders.h"
#include "graphic/gnm/gnmcontext.h"
#include <shader/binary.h>
#include <gnmx/shader_parser.h>
#include <gnmx/fetchshaderhelper.h>
#include "system/memory/physmemalloc.h"
//#include "graphic/gnm/gnmvertexformat.h"
#include "graphic/gfx/gfxExtension/gfxexconstantsbuffer.h"

#include "graphic/gnm/gnmhelpers.h"

using namespace sce;

//----------------------------------------------------------------------------
popBEGIN_NAMESPACE

popDeclareProfile(GnmResetGnmContext);

#ifdef ENABLE_DUMP_CMDBUFFER_USAGE
// for dump commander buffer usage
ubiU32 g_totalDcbSize_main = 0;
ubiU32 g_maxDcbUsage_main = 0;
ubiU32 g_totalCcbSize_main = 0;
ubiU32 g_maxCcbUsage_main = 0;
ubiU32 g_totalDcbSize_deferred = 0;
ubiU32 g_maxDcbUsage_deferred = 0;
ubiU32 g_totalCcbSize_deferred = 0;
ubiU32 g_maxCcbUsage_deferred = 0;
// for dump the number of commander buffer chunks
ubiU32 g_numberOfChunk_main = 0;
ubiU32 g_numberOfChunk_deferred = 0;
#endif

ubiU32  GNMDevice::m_BlendModeToDstBlend[NbBlendModes] =
{
	BlendFunc_Zero,         // Blend_Copy
	BlendFunc_One,          // Blend_Add
	BlendFunc_InvSrcColor,  // Blend_Sub
	BlendFunc_Zero,         // Blend_Mul
	BlendFunc_InvSrcAlpha,  // Blend_Alpha
	BlendFunc_One,          // Blend_Dodge
	BlendFunc_One,          // Blend_AlphaAdditive
	BlendFunc_InvDestAlpha, // Blend_DestAlpha
};

ubiU32  GNMDevice::m_BlendModeToSrcBlend[NbBlendModes] =
{
	BlendFunc_One,          // Blend_Copy
	BlendFunc_One,          // Blend_Add
	BlendFunc_One,          // Blend_Sub
	BlendFunc_DestColor,    // Blend_Mul
	BlendFunc_SrcAlpha,     // Blend_Alpha
	BlendFunc_DestColor,    // Blend_Dodge
	BlendFunc_SrcAlpha,     // Blend_AlphaAdditive
	BlendFunc_DestAlpha,    // Blend_DestAlpha
};


//============================================================================
// GNMDevice
//============================================================================

// Overlay depth bias
ubiFloat GNMDevice::ms_OverlayMaterial_SlopeScaleDepthBias = -0.00001f;
ubiFloat GNMDevice::ms_OverlayMaterial_DepthBias = -0.00001f;

#ifdef INVERSE_DEPTH
ubiBool GNMDevice::m_bInversedMode = false;
#endif //INVERSE_DEPTH

extern bool needReloadPsShader;

GNMDevice::GNMDevice(ubiBool isMainContext /*= true*/, ubiU32 deferredID /*= 0*/)
	: m_isUsingExDevice(0)
	, m_IsDeferredDevice(!isMainContext)
	, m_IsRecording(false)
	, m_NumDrawCalls(0)
	, m_ContextToFill(nullptr)
{
#ifndef POP_OPTIMIZED
	if (isMainContext)
	{
		sprintf(mName, "Main device");
	}
	else
	{
		sprintf(mName, "Deferred device %d", deferredID);
	}
#endif // POP_OPTIMIZED

	if (isMainContext)
	{
		OrbisGPUFence::Alloc();
	}

	m_ShaderManager = &GfxexGraphicDevice::GetInstance()->GetShaderManager();

	m_SamplerChanged = 0;

	m_SlopeScaleDepthBias = 0.0f;
	m_DepthBias = 0.0f;

	popMemZeroArray(m_vsc_f);
	popMemZeroArray(m_psc_f);

	InitStates(true);

#ifdef EMULATE_GPUBEGIN_API
	// GpuBegin* API emulation for GNM PC
	m_GPUBeginVSIndex = 0;
	m_GPUBeginVSCount = 0;
	m_GPUBeginPSIndex = 0;
	m_GPUBeginPSCount = 0;
#endif

#ifdef EMULATE_BEGINVERTICES_API
	m_DynamicVB = NULL;
	m_DynamicIB = NULL;
#endif

#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
	InitGlobalConstantsCheck();
#endif

	CreateResources();

	CreateStates(); // Initialise containers for render state storage (map) and default states

					// The immediate context is responsible for statically initializing the state manager,
					// that way it only happens once
	GNMStateManager::StaticInitialize(); // Init table

	m_StateManager = popNew(GfxStateManager, "GfxStateManager", this);
	m_StateManager->SetGNMDevice(this); //@@CK: need this to call back the storage/creation functions of renderstates

	m_LastIndexCount = 0;
	m_BaseVertexIndex = 0;

	// Initialize render context
	InitContext();

	//@@LRF to reset hardware state, should be do it only once in the beginning of the frame by immediate device. otherwise will cause the GPU Hang
	//@@LRF put here for first frame, for the rest, the GNMSwapchain::Flip will takecare of them.
	//@@LRF and why not BeginScene function? because there is a draw call came from clear before it.
	if (isMainContext)
	{
		m_ContextToFill->initializeDefaultHardwareState();
		//@@LRF no need to force kick here, as we will kick the main context at GfxMultiThreadRenderer::BeginFrame->FlushAllCommandListInDeferredContexts
		//KickCommandBuffer();
	}

	InitMSAASettings();
}

//----

GNMDevice::~GNMDevice()
{
	for (uint32_t i = 0; i < m_ConextsCount; i++)
	{
		GnmContext *t_next = m_FirstContext->m_pNextContext;
		DestoryContext(m_FirstContext);
		m_FirstContext = t_next;
	}
	popAssert(m_ConextsCount == 0);

	for (auto i = 0; i < NumSwapchainBuffers; i++)
	{
		g_PhysMemAllocator->Free(m_DcbMemory[i]);
		g_PhysMemAllocator->Free(m_CcbMemory[i]);
	}
	g_PhysMemAllocator->Free(m_CRAA8xLutAddress);

#ifndef USING_SEPARATE_CUE_HEAP
	for (uint32_t i = 0; i < MAX_CUE_HEAPS; i++)
	{
		g_PhysMemAllocator->Free(m_CueHeapMemory[i]);
		m_CueHeapMemory[i] = NULL;
	}
#endif


	popDelete(m_StateManager);



	DeleteResources();

	// #if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
	// 	if (m_device != NULL)
	// #endif
	// 	{
	// 		ubiU32 refCount = 0;
	// 		do 
	// 		{
	// 			refCount = m_device->Release();
	// 		} while (refCount != 0);
	// 
	// 		m_device = NULL;
	// 	}

	OrbisGPUFence::Free();
}
//----

#ifndef GNM_PORTING_TODO
HRESULT GNMDevice::Reset(DXGI_SWAP_CHAIN_DESC &pp_)
{
#if !GNM_DISABLED
	DeleteResources();

	HRESULT hr = m_device->Reset(&pp_);

	if (SUCCEEDED(hr))
	{
		g_GraphicManager->SetDeviceLost(false);
		CreateResources();
	}

	return hr;
#endif
	return S_OK;
}
#endif // GNM_PORTING_TODO
GNMTexture* GNMDevice::CreateTexture(ubiU32 width_, ubiU32 height_, ubiU32 num_levels_, GFX_USAGE usage_, GFXFORMAT format_, ubiU32 bindFlags)
{
	//@@RVH:Dynamic is should always be false, code that needs dynamic should not call this function
	return GNMTexture::CreateTexture(width_, height_, num_levels_, NULL, format_, usage_ & GFXUSAGE_DYNAMIC);
}
//----

GNMTexture* GNMDevice::CreateCubeTexture(ubiU32 edge_len_, ubiU32 num_levels_, GFX_USAGE usage_, GFXFORMAT format_, ubiU32 bindFlags)
{
	return GNMTexture::CreateCubeTexture(edge_len_, format_, usage_ & GFXUSAGE_DYNAMIC);
}
//----

GNMTexture* GNMDevice::CreateVolumeTexture(ubiU32 width_, ubiU32 height_, ubiU32 depth_, ubiU32 num_levels_, GFX_USAGE usage_, GFXFORMAT format_, ubiU32 bindFlags)
{
	return GNMTexture::CreateVolumeTexture(width_, height_, depth_, NULL, format_, usage_ & GFXUSAGE_DYNAMIC);
}
//----

PlatformGfxRenderTargetView* GNMDevice::CreateRenderTargetView(ubiU32 width_, ubiU32 height_, GFXFORMAT format_, ubiBool isDynamic_, ubiBool isSwapChain_, GFX_MULTISAMPLE_TYPE msaaType)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateRenderTargetView");

	GNMWrappedResource::RenderTargetView* container = popNew(GNMWrappedResource::RenderTargetView, "GnmRenderTargetContainer", this)(width_, height_, format_, isDynamic_, isSwapChain_, msaaType);

	return container;
}
//----

PlatformGfxDepthStencilView* GNMDevice::CreateDepthStencilView(ubiU32 width_, ubiU32 height_, sce::Gnm::ZFormat zFormat_, sce::Gnm::StencilFormat stencilFormat_, ubiBool useHTile_, GFX_MULTISAMPLE_TYPE msaaType)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateDepthStencilView");

	GNMWrappedResource::DepthStencilView* container = popNew(GNMWrappedResource::DepthStencilView, "GnmDepthStencilTargetContainer", this)(width_, height_, zFormat_, stencilFormat_, useHTile_, msaaType);

	return container;
}
//----

GfxBufferView* GNMDevice::CreateBufferView(ubiBool writable, sce::Gnm::DataFormat format, ubiU32 numElements)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateBufferView");

	GfxBufferView* container = popNew(GfxBufferView, "GnmBufferContainer", this)(writable, format, numElements);
	return container;
}
//----

Gear::RefCountedPtr<GNMWrappedResource::IndexBuffer> GNMDevice::CreateIndexBuffer(ubiU32 numBytes, GFX_USAGE usage, GFXFORMAT format, void* data)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateIndexBuffer");

	// create index buffer
	GNMWrappedResource::IndexBuffer* ib = popNew(GNMWrappedResource::IndexBuffer, "GnmIndexBufferContainer", this)(numBytes, usage, format, data);

#ifndef POP_FINAL
	g_GfxexRenderStats.AddIndexBuffer(ib);
#endif

	return Gear::RefCountedPtr<GNMWrappedResource::IndexBuffer>(ib);
}
//----

Gear::RefCountedPtr<GNMWrappedResource::VertexBuffer> GNMDevice::CreateVertexBuffer(ubiU32 numBytes, GFX_USAGE usage, GFXPOOL pool, void* data)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateVertexBuffer");

	// create vertex buffer
	GNMWrappedResource::VertexBuffer* vb = popNew(GNMWrappedResource::VertexBuffer, "GnmVertexBufferContainer", this)(numBytes, usage, data);

#ifndef POP_FINAL
	g_GfxexRenderStats.AddVertexBuffer(vb);
#endif

	return Gear::RefCountedPtr<GNMWrappedResource::VertexBuffer>(vb);
}
//----

Gear::RefCountedPtr<GNMWrappedResource::PixelShader> GNMDevice::CreatePixelShader(const void* compiled_, ubiU32 size_)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreatePixelShader");

#ifdef POP_HACK_RESOURCE_SHADER_AS_DUMMY
	if (m_ShaderManager->g_InitializeState && !needReloadPsShader)
		return m_ShaderManager->GetPixelShader(PS_DummyShader_HACK)->GetWrappedShader();
#endif

	// create pixel shader
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, compiled_);

	void* gpuMem = g_PhysMemAllocator->Alloc(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	popMemCopy(gpuMem, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);

	void* buffer = popAlloc(shaderInfo.m_psShader->computeSize(), "OrbisPixelShader", this);
	popMemCopy(buffer, shaderInfo.m_shaderStruct, shaderInfo.m_psShader->computeSize());

	GNMWrappedResource::PixelShader* ps = popNew(GNMWrappedResource::PixelShader, "PixelShader", this)(static_cast<Gnmx::PsShader*>(buffer));
	ps->GetGNMObject()->patchShaderGpuAddress(gpuMem);

#ifndef POP_OPTIMIZED  //@@LRF for shader debug
	ps->SetBinaryData(compiled_, size_);
#endif

	return Gear::RefCountedPtr<GNMWrappedResource::PixelShader>(ps);
}
//----

#ifdef ENABLE_FETCH_SHADER_REMAPING
static void GenerateVertexInputRemapTable(const Shader::Binary::Program* vsp, ubiU32 *remapTable)
{
	popAssert(vsp != NULL);
	popAssert(remapTable != NULL);

#if defined(_DEBUG) || defined(DEBUG)
	// loop through all vertex input attributes and check if each attribute has corresponding vertex buffer descriptor
	for (uint8_t i = 0; i < vsp->m_numInputAttributes; ++i)
	{
		sce::Shader::Binary::Attribute* attrib = vsp->m_inputAttributes + i;

		sce::Shader::Binary::PsslSemantic psslSemantic = (sce::Shader::Binary::PsslSemantic)attrib->m_psslSemantic;
		// An input with system semantic does not need a slot in the remap table; ignore it
		if (psslSemantic >= sce::Shader::Binary::kSemanticSClipDistance && psslSemantic <= sce::Shader::Binary::kSemanticSInsideTessFactor)
			continue;

		// In PSSL all input semantics to Vs/Ls/Es shaders where a fetch shader is required are user defined!
		// Note: this is not the case when using orbis-cgc
		popAssert(psslSemantic == sce::Shader::Binary::kSemanticUserDefined);

		// look for the corresponding vertex buffer semantic name
		bool found = false;
		for (uint32_t j = 0; j < VtxInputUsage::MaxNbUsage; j++)
		{
			if (strcmp((const char*)attrib->getSemanticName(), GNMVertexFormatManager::GnmSemanticNames[j]) == 0 &&
				attrib->m_semanticIndex == GNMVertexFormatManager::GnmSemanticIndex[j])
			{
				found = true;
				break;
			}
		}

		popAssertWithMsg(found, ("Error: vertex shader input semantic was not found in the vertex buffer semantic name list"));
	}
#endif

	// loop through all vertex buffer descriptors and fill out remap table entries
	for (uint32_t i = 0; i < VtxInputUsage::MaxNbUsage; i++)
	{
		sce::Shader::Binary::Attribute *inputAttribute = vsp->getInputAttributeBySemanticNameAndIndex(
			GNMVertexFormatManager::GnmSemanticNames[i],
			GNMVertexFormatManager::GnmSemanticIndex[i]);

		// 		UInt8 usage = VERTEX_DECLARATION_GNM::GnmUsages[i];
		// 		if (inputAttribute && remapTable[usage] == 0xFFFFFFFF)
		// 		{
		// 			remapTable[usage] = inputAttribute->m_resourceIndex;
		// 		}
	}
}
#endif // ENABLE_FETCH_SHADER_REMAPING

Gear::RefCountedPtr<GNMWrappedResource::VertexShader> GNMDevice::CreateVertexShader(const void* compiled_, ubiU32 size_)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateVertexShader");

#ifdef POP_HACK_RESOURCE_SHADER_AS_DUMMY
	if (m_ShaderManager->g_InitializeState)
		return m_ShaderManager->GetVertexShader(VS_DummyShader_HACK)->GetWrappedShader();
#endif
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, compiled_);

	// create vertex shader
	// GPU side allocation
	void* gpuMem = g_PhysMemAllocator->Alloc(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	popMemCopy(gpuMem, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	// CPU side allocation (patched VsShader structure)
	void* buffer = popAlloc(shaderInfo.m_vsShader->computeSize(), "OrbisVertexShader", this);
	popMemCopy(buffer, shaderInfo.m_shaderStruct, shaderInfo.m_vsShader->computeSize());
	GNMWrappedResource::VertexShader* vs = popNew(GNMWrappedResource::VertexShader, "VertexShader", this)(static_cast<Gnmx::VsShader*>(buffer));

	vs->GetGNMObject()->patchShaderGpuAddress(gpuMem);

	ubiU32 fetchShaderSize = Gnmx::computeVsFetchShaderSize(vs->GetGNMObject());
	void *fetchShaderAddr = NULL;
	ubiU32 modifier = 0;
	if (fetchShaderSize)
	{
#ifdef ENABLE_FETCH_SHADER_REMAPING
		void *remapTable = g_PhysMemAllocator->Alloc(VtxInputUsage::MaxNbUsage * sizeof(ubiU32), sce::Gnm::kAlignmentOfShaderInBytes);
		popMemSet(remapTable, 0xff, VtxInputUsage::MaxNbUsage * sizeof(ubiU32));
		Shader::Binary::Program vsp;
		vsp.loadFromMemory(compiled_, size_);
		GenerateVertexInputRemapTable(&vsp, (ubiU32 *)remapTable);
#endif // ENABLE_FETCH_SHADER_REMAPING

		fetchShaderAddr = g_PhysMemAllocator->Alloc(fetchShaderSize, Gnm::kAlignmentOfFetchShaderInBytes);
#ifdef ENABLE_FETCH_SHADER_REMAPING
		Gnmx::generateVsFetchShader(fetchShaderAddr, &modifier, vs->GetGNMObject(), (Gnm::FetchShaderInstancingMode*)NULL, 0, remapTable, VtxInputUsage::MaxNbUsage);
#else
		Gnmx::generateVsFetchShader(fetchShaderAddr, &modifier, vs->GetGNMObject(), (Gnm::FetchShaderInstancingMode*)NULL, 0);
#endif // ENABLE_FETCH_SHADER_REMAPING
	}

	vs->SetFetchShaderAddr(fetchShaderAddr);
	vs->SetShaderModifier(modifier);

#ifndef POP_OPTIMIZED  //@@LRF for shader debug
	vs->SetBinaryData(compiled_, size_);
#endif

	return Gear::RefCountedPtr<GNMWrappedResource::VertexShader>(vs);
}

Gear::RefCountedPtr<GNMWrappedResource::ComputeShader> GNMDevice::CreateComputeShader(const void* compiled_, ubiU32 size_)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateComputeShader");

	// create pixel shader
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, compiled_);

	void* gpuMem = g_PhysMemAllocator->Alloc(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	popMemCopy(gpuMem, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);

	void* buffer = popAlloc(shaderInfo.m_csShader->computeSize(), "OrbisComputeShader", this);
	popMemCopy(buffer, shaderInfo.m_shaderStruct, shaderInfo.m_csShader->computeSize());

	GNMWrappedResource::ComputeShader* cs = popNew(GNMWrappedResource::ComputeShader, "ComputeShader", this)(static_cast<Gnmx::CsShader*>(buffer));
	cs->GetGNMObject()->patchShaderGpuAddress(gpuMem);

#ifndef POP_OPTIMIZED  //@@LRF for shader debug
	cs->SetBinaryData(compiled_, size_);
#endif

	return Gear::RefCountedPtr<GNMWrappedResource::ComputeShader>(cs);
}

GNMVertexDeclaration* GNMDevice::CreateVertexDeclaration(const GNMVertexFormatDescriptionElement* input_elements_, ubiU32 num_elements_, ubiChar* debugInfo)
{
	popGfxexCheckLogContext();
	popGfxexMemLogContext("CreateVertexDeclaration");

	GNMVertexDeclarationElements orbisVtxElements;
	for (ubiU32 i = 0; i < num_elements_; ++i)
	{
		const GNMVertexFormatDescriptionElement& vtxElem = input_elements_[i];
		orbisVtxElements.Add(GNMVertexDeclarationElem(vtxElem.m_Format,
			(VtxInputUsage)vtxElem.m_Usage,
			vtxElem.m_Offset,
			vtxElem.m_Stride,
			vtxElem.m_StreamIndex));
	}

	return popNew(GNMVertexDeclaration, "OrbisVertexDeclaration", this)(orbisVtxElements);


	//@@LRF TODO : to check whether if we still need to do these to match vertex layerout and shader input.



	//     PlatformGfxInputLayout* inputLayout = NULL;
	// 
	// 	// Create the input layout
	// 	ubiChar SHADER_INTRO[] = 
	// 		"struct VSIN {                      \n \
		// 		";
// 
// 	ubiChar SHADER_OUTRO[] = 
// 		"};                                 \n \
// 		struct VSOUT                        \n \
// 		{                                   \n \
// 		float4 pos : SV_POSITION;			\n \
// 		};                                  \n \
// 		VSOUT VSMain(VSIN input)            \n \
// 		{                                   \n \
// 		VSOUT output;						\n \
// 		output.pos = float4(0,0,0,0);		\n \
// 		return output;						\n \
// 		}                                   \n \
// 		";
// 
// 	// Generate a dummy vertex shader whose signature matches exactly the input layout
// 	ID3DBlob *shaderBlob, *errorMsgs;
// 	SimpleString shaderString;
// 
// 	// VS Intro
// #if defined(POP_DEBUG)
// 	if (debugInfo)
// 	{
// 		shaderString += debugInfo;
// 	}
// #endif
// 	shaderString += SHADER_INTRO;
// 
// 	// Adding input elements
// 	for (ubiU32 elem = 0; elem < num_elements_; elem++)
// 	{
// 		shaderString += GetVSInputSemanticTypeName(input_elements_[elem].Format);
// 		shaderString += StringFormat(" elem%i : %s%i;\n", elem, input_elements_[elem].SemanticName, input_elements_[elem].SemanticIndex);
// 	}
// 
// 	// VS Outro
// 	shaderString += SHADER_OUTRO;
// 
// 	ubiU32 complicationFlags = 0;
// #ifdef POP_DEBUG
// 	complicationFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
// #endif
// 	// Compile the dummy VS
// 	HRESULT hr = D3DCompile(shaderString.GetBuffer(),
// 		shaderString.GetLength(),
// 		NULL,
// 		NULL,
// 		NULL,
// 		"VSMain",
// 		"vs_5_0",
// 		complicationFlags,
// 		0,
// 		&shaderBlob,
// 		&errorMsgs);
// 
// 	if (SUCCEEDED(hr))
// 	{
// 		// Create the actual input layout
// 		GNM_CALL(m_device->CreateInputLayout(input_elements_, 
// 											num_elements_, 
// 											shaderBlob->GetBufferPointer(), 
// 											shaderBlob->GetBufferSize(), 
// 											&inputLayout));
// 	}
// 	else if (errorMsgs)
// 	{
// 		// Dump the shader code
// 		LogMessage(LogGraphics, shaderString.GetBuffer());
// 	}
// 
// 	if (errorMsgs)
// 	{
// 		// Dump the messages
// 		LogMessage(LogGraphics, (const ubiChar*)errorMsgs->GetBufferPointer());
// 		popAssert(0);
// 	}
// 
//     SAFE_RELEASE(shaderBlob);
//     SAFE_RELEASE(errorMsgs);
//     return inputLayout;
}
//----

//----------------------------------------------------------------------------

void GNMDevice::BeginScene()
{
	// TODO: wdj GNM seems no need this, need check later
	//     GNM_CALL(m_device->BeginScene());
}
//----

void GNMDevice::EndScene()
{
	// TODO: wdj GNM seems no need this, need check later
	//     GNM_CALL(m_device->EndScene());
}

//----------------------------------------------------------------------------

void GNMDevice::ResetStates()
{
	// reset render and sampler states to defaults
	InitStates();
}

void GNMDevice::FullResetStates()
{
#if defined(POP_MULTITHREAD_RENDERER)
	m_StateManager->FullResetStates();
	m_SamplerChanged = (0x01 << (max_samplers + 1)) - 1;
#endif
}
//----

void GNMDevice::SetVertexShaderConstantF(unsigned start_reg_, const float *f_, unsigned num_vec4f_)
{
#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
	ValidateConstantOperation(false, start_reg_, num_vec4f_);
#endif

	SetVSC(start_reg_, (const ubiVector4*)f_, num_vec4f_);

	m_StateManager->SetDirtyFlag(GNMStateManager::DIRTY_VS_CONTANT_BUFFER);
}
//----

void GNMDevice::SetVertexShaderConstantI(unsigned start_reg_, const int *i_, unsigned num_vec4i_)
{
	SetVSC(start_reg_, (const ubiVector4I*)i_, num_vec4i_);

	m_StateManager->SetDirtyFlag(GNMStateManager::DIRTY_VS_CONTANT_BUFFER);
}
//----

void GNMDevice::SetVertexShaderConstantB(unsigned start_reg_, const BOOL *b_, unsigned num_bools_)
{
	SetVSC(start_reg_, b_, num_bools_);

	m_StateManager->SetDirtyFlag(GNMStateManager::DIRTY_VS_CONTANT_BUFFER);
}
//----

void GNMDevice::SetPixelShaderConstantF(unsigned start_reg_, const float *f_, unsigned num_vec4f_)
{
#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE
	ValidateConstantOperation(true, start_reg_, num_vec4f_);
#endif
	SetPSC(start_reg_, (ubiVector4*)f_, num_vec4f_);

	m_StateManager->SetDirtyFlag(GNMStateManager::DIRTY_PS_CONTANT_BUFFER);
}
//----

void GNMDevice::GetPixelShaderConstantF(unsigned start_reg_, const float *f_, unsigned num_vec4f_)
{
	GetPSC(start_reg_, (ubiVector4*)f_, num_vec4f_);
}
//----

void GNMDevice::SetPixelShaderConstantI(unsigned start_reg_, const int *i_, unsigned num_vec4i_)
{
	SetPSC(start_reg_, (ubiVector4I*)i_, num_vec4i_);

	m_StateManager->SetDirtyFlag(GNMStateManager::DIRTY_PS_CONTANT_BUFFER);
}
//----

void GNMDevice::SetPixelShaderConstantB(unsigned start_reg_, const BOOL *b_, unsigned num_bools_)
{
	SetPSC(start_reg_, b_, num_bools_);

	m_StateManager->SetDirtyFlag(GNMStateManager::DIRTY_PS_CONTANT_BUFFER);
}
//----------------------------------------------------------------------------

popDeclareProfile(InitStates);

void GNMDevice::InitStates(ubiBool bInit)
{
	popPlatformProfile(InitStates);
	//TODO: After shader constant buffer and recording command buffer ready we should fix this function.
	// 
	//     m_ShaderManager = &GfxexGraphicDevice::GetInstance()->GetShaderManager();
	// 
	//     if(bInit)
	//     {
	//         POP_UPDATE_D3DSTATECACHE(m_PixelShaderConstantMap = NULL);
	//         POP_UPDATE_D3DSTATECACHE(m_VertexShaderConstantMap = NULL);
	//         POP_UPDATE_D3DSTATECACHE(m_ShaderManager = NULL);
	//         POP_UPDATE_D3DSTATECACHE(m_CurrentDepthStencilSurface = NULL);
	// 
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(m_CurrentRenderTarget, 0, sizeof(m_CurrentRenderTarget)));
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(&m_CurrentViewPort, 0, sizeof(m_CurrentViewPort)));
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(&m_CurrentScissorRect, 0, sizeof(m_CurrentScissorRect)));
	// 
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(m_CurrentPixelTexture, 0, sizeof(m_CurrentPixelTexture)));
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(m_CurrentVertexTexture, 0, sizeof(m_CurrentVertexTexture)));
	// 
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(m_CurrentVertexBuffer, 0, sizeof(m_CurrentVertexBuffer)));
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(m_CurrentVertexBufferOffset, 0, sizeof(m_CurrentVertexBufferOffset)));
	//         POP_UPDATE_D3DSTATECACHE(popMemSet(m_CurrentVertexBufferStride, 0, sizeof(m_CurrentVertexBufferStride)));
	//         POP_UPDATE_D3DSTATECACHE(m_CurrentIndexBuffer = NULL);
	//         POP_UPDATE_D3DSTATECACHE(m_CurrentVertexDeclaration = NULL);
	// 
	// #if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
	// 		if (m_ResetStateBlock == NULL)
	// 		{
	// 			m_device->BeginStateBlock();
	// 		}
	// #endif
	//     }
	// 
	//     // initialize all render states
	//     #define D3D_SETRS(state, value) if(bInit) \
		//                                     { \
//                                         GNMDevice_CALL(m_device->SetRenderState(state, raw_cast<DWORD>(value))); \
//                                         POP_UPDATE_D3DSTATECACHE(m_CurrentRenderState[D3DSTATE_TO_INDEX(state)] = (DWORD)value); \
//                                     } \
//                                     else \
//                                     { \
//                                         SetRenderState(state, raw_cast<DWORD>(value)); \
//                                     }
// 
//     D3D_SETRS(GFX_RS_ZENABLE, D3DZB_TRUE);
//     D3D_SETRS(GFX_RS_FILLMODE, D3DFILL_SOLID);
//     D3D_SETRS(GFX_RS_ZWRITEENABLE, TRUE);
//     D3D_SETRS(GFX_RS_ALPHATESTENABLE, FALSE);
//     D3D_SETRS(GFX_RS_SRCBLEND, GFX_BLEND_MODE_ONE);
//     D3D_SETRS(GFX_RS_DESTBLEND, GFX_BLEND_MODE_ZERO);
//     D3D_SETRS(GFX_RS_CULLMODE, GFX_CULL_MODE_CCW);
//     D3D_SETRS(GFX_RS_ZFUNC, GFX_CMP_LESSEQUAL);
//     D3D_SETRS(GFX_RS_ALPHAREF, 0);
//     D3D_SETRS(GFX_RS_ALPHAFUNC, GFX_CMP_ALWAYS);
//     D3D_SETRS(GFX_RS_ALPHABLENDENABLE, FALSE);
//     D3D_SETRS(GFX_RS_STENCILENABLE, FALSE);
//     D3D_SETRS(GFX_RS_STENCILFAIL, GFX_STENCIL_OP_KEEP);
//     D3D_SETRS(GFX_RS_STENCILZFAIL, GFX_STENCIL_OP_KEEP);
//     D3D_SETRS(GFX_RS_STENCILPASS, GFX_STENCIL_OP_KEEP);
//     D3D_SETRS(GFX_RS_STENCILFUNC, GFX_CMP_ALWAYS);
//     D3D_SETRS(GFX_RS_STENCILREF, 0);
//     D3D_SETRS(GFX_RS_STENCILMASK, 0xffffffff);
//     D3D_SETRS(GFX_RS_STENCILWRITEMASK, 0xffffffff);
//     D3D_SETRS(GFX_RS_WRAP0, 0);
//     D3D_SETRS(GFX_RS_WRAP1, 0);
//     D3D_SETRS(GFX_RS_WRAP2, 0);
//     D3D_SETRS(GFX_RS_WRAP3, 0);
//     D3D_SETRS(GFX_RS_WRAP4, 0);
//     D3D_SETRS(GFX_RS_WRAP5, 0);
//     D3D_SETRS(GFX_RS_WRAP6, 0);
//     D3D_SETRS(GFX_RS_WRAP7, 0);
//     D3D_SETRS(GFX_RS_CLIPPLANEENABLE, 0);
//     D3D_SETRS(GFX_RS_POINTSIZE, 64.0f);
//     D3D_SETRS(GFX_RS_POINTSIZE_MIN, 1.0f);
//     D3D_SETRS(GFX_RS_POINTSPRITEENABLE, FALSE);
//     D3D_SETRS(GFX_RS_MULTISAMPLEANTIALIAS, TRUE);
//     D3D_SETRS(GFX_RS_MULTISAMPLEMASK, 0xffffffff);
//     D3D_SETRS(GFX_RS_POINTSIZE_MAX, 64.0f);
//     D3D_SETRS(GFX_RS_COLORWRITEENABLE, 0xf);
//     D3D_SETRS(GFX_RS_BLENDOP, GFX_BLEND_OP_ADD);
//     D3D_SETRS(GFX_RS_SCISSORTESTENABLE, FALSE);
//     D3D_SETRS(GFX_RS_SLOPESCALEDEPTHBIAS, 0);
//     D3D_SETRS(GFX_RS_MINTESSELLATIONLEVEL, 1.0f);
//     D3D_SETRS(GFX_RS_MAXTESSELLATIONLEVEL, 1.0f);
//     D3D_SETRS(GFX_RS_TWOSIDEDSTENCILMODE, FALSE);
//     D3D_SETRS(GFX_RS_CCW_STENCILFAIL, GFX_STENCIL_OP_KEEP);
//     D3D_SETRS(GFX_RS_CCW_STENCILZFAIL, GFX_STENCIL_OP_KEEP);
//     D3D_SETRS(GFX_RS_CCW_STENCILPASS, GFX_STENCIL_OP_KEEP);
//     D3D_SETRS(GFX_RS_CCW_STENCILFUNC, GFX_CMP_ALWAYS);
//     D3D_SETRS(GFX_RS_COLORWRITEENABLE1, 0xf);
//     D3D_SETRS(GFX_RS_COLORWRITEENABLE2, 0xf);
//     D3D_SETRS(GFX_RS_COLORWRITEENABLE3, 0xf);
//     D3D_SETRS(GFX_RS_BLENDFACTOR, 0xffffffff);
//     D3D_SETRS(GFX_RS_DEPTHBIAS, 0);
//     D3D_SETRS(GFX_RS_WRAP8, 0);
//     D3D_SETRS(GFX_RS_WRAP9, 0);
//     D3D_SETRS(GFX_RS_WRAP10, 0);
//     D3D_SETRS(GFX_RS_WRAP11, 0);
//     D3D_SETRS(GFX_RS_WRAP12, 0);
//     D3D_SETRS(GFX_RS_WRAP13, 0);
//     D3D_SETRS(GFX_RS_WRAP14, 0);
//     D3D_SETRS(GFX_RS_WRAP15, 0);
//     D3D_SETRS(GFX_RS_SEPARATEALPHABLENDENABLE, FALSE);
//     D3D_SETRS(GFX_RS_SRCBLENDALPHA, GFX_BLEND_MODE_ONE);
//     D3D_SETRS(GFX_RS_DESTBLENDALPHA, GFX_BLEND_MODE_ZERO);
//     D3D_SETRS(GFX_RS_BLENDOPALPHA, GFX_BLEND_OP_ADD);
// 
// 	// initialize pc-specific render states
//     D3D_SETRS(GFX_RS_SRGBWRITEENABLE, 0);
//     D3D_SETRS(GFX_RS_DITHERENABLE, FALSE);
//     D3D_SETRS(GFX_RS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
//     D3D_SETRS(GFX_RS_POSITIONDEGREE, D3DDEGREE_CUBIC);
//     D3D_SETRS(GFX_RS_NORMALDEGREE, D3DDEGREE_LINEAR);
//     D3D_SETRS(GFX_RS_ADAPTIVETESS_X, 0.0f);
//     D3D_SETRS(GFX_RS_ADAPTIVETESS_Y, 0.0f);
//     D3D_SETRS(GFX_RS_ADAPTIVETESS_Z, 1.0f);
//     D3D_SETRS(GFX_RS_ADAPTIVETESS_W, 0.0f);
//     D3D_SETRS(GFX_RS_ENABLEADAPTIVETESSELLATION, FALSE);
//     D3D_SETRS(GFX_RS_ANTIALIASEDLINEENABLE, FALSE);
//     D3D_SETRS(GFX_RS_SHADEMODE, D3DSHADE_GOURAUD);
//     D3D_SETRS(GFX_RS_LASTPIXEL, TRUE);
//     D3D_SETRS(GFX_RS_FOGENABLE, FALSE);
//     D3D_SETRS(GFX_RS_SPECULARENABLE, FALSE);
//     D3D_SETRS(GFX_RS_FOGCOLOR, 0);
//     D3D_SETRS(GFX_RS_FOGTABLEMODE, D3DFOG_NONE);
//     D3D_SETRS(GFX_RS_FOGSTART, 0.0f);
//     D3D_SETRS(GFX_RS_FOGEND, 1.0f);
//     D3D_SETRS(GFX_RS_FOGDENSITY, 1.0f);
//     D3D_SETRS(GFX_RS_RANGEFOGENABLE, FALSE);
//     D3D_SETRS(GFX_RS_TEXTUREFACTOR, 0xffffffff);
//     D3D_SETRS(GFX_RS_CLIPPING, TRUE);
//     D3D_SETRS(GFX_RS_LIGHTING, TRUE);
//     D3D_SETRS(GFX_RS_AMBIENT, 0);
//     D3D_SETRS(GFX_RS_FOGVERTEXMODE, D3DFOG_NONE);
//     D3D_SETRS(GFX_RS_COLORVERTEX, TRUE);
//     D3D_SETRS(GFX_RS_LOCALVIEWER, TRUE);
//     D3D_SETRS(GFX_RS_NORMALIZENORMALS, FALSE);
//     D3D_SETRS(GFX_RS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
//     D3D_SETRS(GFX_RS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
//     D3D_SETRS(GFX_RS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
//     D3D_SETRS(GFX_RS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
//     D3D_SETRS(GFX_RS_VERTEXBLEND, D3DVBF_DISABLE);
//     D3D_SETRS(GFX_RS_POINTSCALEENABLE, FALSE);
//     D3D_SETRS(GFX_RS_POINTSCALE_A, 1.0f);
//     D3D_SETRS(GFX_RS_POINTSCALE_B, 0.0f);
//     D3D_SETRS(GFX_RS_POINTSCALE_C, 0.0f);
//     D3D_SETRS(GFX_RS_DEBUGMONITORTOKEN, D3DDMT_DISABLE);  // DX default = D3DDMT_ENABLE
//     D3D_SETRS(GFX_RS_INDEXEDVERTEXBLENDENABLE, FALSE);
//     D3D_SETRS(GFX_RS_TWEENFACTOR, 0.0f);
// 
//     //----
//     #undef D3D_SETRS
// 
//     // initialize all sampler states
//     #define D3D_SETSS(sampler, state, value) if(bInit) \
//                                              { \
//                                                  GNMDevice_CALL(m_device->SetSamplerState(sampler, state, raw_cast<DWORD>(value))); \
//                                                  POP_UPDATE_D3DSTATECACHE(m_CurrentSamplerState[sampler][D3DSTATE_TO_INDEX(state)] = (DWORD)value); \
//                                              } \
//                                              else \
//                                              { \
//                                                  SetSamplerState(sampler, state, raw_cast<DWORD>(value)); \
//                                              }
// 
//     for(unsigned si=0; si<max_samplers; ++si)
//     {
//         D3D_SETSS(si, GFX_SAMP_ADDRESSU, GFX_TEX_ADDRESS_WRAP);
//         D3D_SETSS(si, GFX_SAMP_ADDRESSV, GFX_TEX_ADDRESS_WRAP);
//         D3D_SETSS(si, GFX_SAMP_ADDRESSW, GFX_TEX_ADDRESS_WRAP);
//         D3D_SETSS(si, GFX_SAMP_BORDERCOLOR, 0);
//         D3D_SETSS(si, GFX_SAMP_MAGFILTER, m_EffectiveValueForMagLinearFilter);  // DX default = GFX_TEX_FILTER_POINT
//         D3D_SETSS(si, GFX_SAMP_MINFILTER, m_EffectiveValueForMinLinearFilter);  // DX default = GFX_TEX_FILTER_POINT
//         D3D_SETSS(si, GFX_SAMP_MIPFILTER, GFX_TEX_FILTER_LINEAR);  // DX default = GFX_TEX_FILTER_NONE
//         D3D_SETSS(si, GFX_SAMP_MIPMAPLODBIAS, m_DefaultLODBias);
//         D3D_SETSS(si, GFX_SAMP_MAXMIPLEVEL, 0);
//         D3D_SETSS(si, GFX_SAMP_MAXANISOTROPY, m_AnisotropyDegree);
//         // initialize pc-specific sampler states
//         D3D_SETSS(si, GFX_SAMP_SRGBTEXTURE, 0);
//         D3D_SETSS(si, GFX_SAMP_ELEMENTINDEX, 0);
//         D3D_SETSS(si, GFX_SAMP_DMAPOFFSET, 0);
//     }
//     //----
//     #undef D3D_SETSS
// 
//     // initialize all vertex sampler states
//     #define D3D_SETVSS(sampler, state, value) if(bInit) \
//                                               { \
//                                                   GNMDevice_CALL(m_device->SetSamplerState(D3DVERTEXTEXTURESAMPLER0+sampler, state, raw_cast<DWORD>(value))); \
//                                                   POP_UPDATE_D3DSTATECACHE(m_CurrentVSamplerState[sampler][D3DSTATE_TO_INDEX(state)] = value); \
//                                               } \
//                                               else \
//                                               { \
//                                                   SetVSamplerState(sampler, state, raw_cast<DWORD>(value)); \
//                                               } \
// 
//     for(unsigned vsi=0; vsi<max_vsamplers; ++vsi)
//     {
//         D3D_SETVSS(vsi, GFX_SAMP_ADDRESSU, GFX_TEX_ADDRESS_WRAP);
//         D3D_SETVSS(vsi, GFX_SAMP_ADDRESSV, GFX_TEX_ADDRESS_WRAP);
//         D3D_SETVSS(vsi, GFX_SAMP_ADDRESSW, GFX_TEX_ADDRESS_WRAP);
//         D3D_SETVSS(vsi, GFX_SAMP_BORDERCOLOR, 0);
//         D3D_SETVSS(vsi, GFX_SAMP_MAGFILTER, GFX_TEX_FILTER_LINEAR);  // DX default = GFX_TEX_FILTER_POINT
//         D3D_SETVSS(vsi, GFX_SAMP_MINFILTER, GFX_TEX_FILTER_LINEAR);  // DX default = GFX_TEX_FILTER_POINT
//         D3D_SETVSS(vsi, GFX_SAMP_MIPFILTER, GFX_TEX_FILTER_LINEAR);  // DX default = GFX_TEX_FILTER_NONE
//         D3D_SETVSS(vsi, GFX_SAMP_MIPMAPLODBIAS, 0);
//         D3D_SETVSS(vsi, GFX_SAMP_MAXMIPLEVEL, 0);
//         D3D_SETVSS(vsi, GFX_SAMP_MAXANISOTROPY, 1);
// 
// 		// initialize pc-specific sampler states
//         D3D_SETVSS(vsi, GFX_SAMP_SRGBTEXTURE, 0);
//         D3D_SETVSS(vsi, GFX_SAMP_ELEMENTINDEX, 0);
//         D3D_SETVSS(vsi, GFX_SAMP_DMAPOFFSET, 0);
//     }
//     //----
//     #undef D3D_SETVSS
// 
//     // initialize all texture stage states
//     #define D3D_SETTSS(stage, state, value) GNMDevice_CALL(m_device->SetTextureStageState(stage, state, raw_cast<DWORD>(value)))
// 
//     if(bInit)
//     {
//     for(unsigned tsi=0; tsi<4; ++tsi)
//     {
//         D3D_SETTSS(tsi, D3DTSS_COLOROP, tsi?D3DTOP_DISABLE:D3DTOP_MODULATE);
//         D3D_SETTSS(tsi, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//         D3D_SETTSS(tsi, D3DTSS_COLORARG2, D3DTA_CURRENT);
//         D3D_SETTSS(tsi, D3DTSS_ALPHAOP, tsi?D3DTOP_DISABLE:D3DTOP_SELECTARG1);
//         D3D_SETTSS(tsi, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
//         D3D_SETTSS(tsi, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
//         D3D_SETTSS(tsi, D3DTSS_BUMPENVMAT00, 0.0f);
//         D3D_SETTSS(tsi, D3DTSS_BUMPENVMAT01, 0.0f);
//         D3D_SETTSS(tsi, D3DTSS_BUMPENVMAT10, 0.0f);
//         D3D_SETTSS(tsi, D3DTSS_BUMPENVMAT11, 0.0f);
//         D3D_SETTSS(tsi, D3DTSS_TEXCOORDINDEX, tsi);
//         D3D_SETTSS(tsi, D3DTSS_BUMPENVLSCALE, 0);
//         D3D_SETTSS(tsi, D3DTSS_BUMPENVLOFFSET, 0);
//         D3D_SETTSS(tsi, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
//         D3D_SETTSS(tsi, D3DTSS_COLORARG0, D3DTA_CURRENT);
//         D3D_SETTSS(tsi, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
//         D3D_SETTSS(tsi, D3DTSS_RESULTARG, D3DTA_CURRENT);
//         D3D_SETTSS(tsi, D3DTSS_CONSTANT, 0);
//     }
//     }
//     //----
//     #undef D3D_SETTSS
// 
//     // init textures, shaders and shader constants
//     for(unsigned i = 0; i < max_samplers; ++i)
//     {
//         GNMDevice_CALL(m_device->SetTexture(i, 0));
//         POP_UPDATE_D3DSTATECACHE(m_CurrentPixelTexture[i] = NULL);
//     }
//     for(unsigned i = 0; i < max_vsamplers; ++i)
//     {
//         GNMDevice_CALL(m_device->SetTexture(D3DVERTEXTEXTURESAMPLER0+i, 0));
//         POP_UPDATE_D3DSTATECACHE(m_CurrentVertexTexture[i] = NULL);
//     }
//     GNMDevice_CALL(m_device->SetVertexShader(0));
//     GNMDevice_CALL(m_device->SetPixelShader(0));
// 
//     POP_UPDATE_D3DSTATECACHE(m_CurrentVertexShader =  NULL);
//     POP_UPDATE_D3DSTATECACHE(m_CurrentPixelShader = NULL);
// 
// #if defined(POP_PLATFORM_WIN32) && defined(POP_MULTITHREAD_RENDERER)
// 	GNMDevice_CALL(m_device->SetDepthStencilSurface(NULL));
// 
// 	if (bInit)
// 	{
// 		popMemCopy(&m_InitRenderState,&m_CurrentRenderState,sizeof(m_CurrentRenderState));
// 		popMemCopy(&m_InitVSamplerState,&m_CurrentVSamplerState,sizeof(m_CurrentVSamplerState));
// 		popMemCopy(&m_InitSamplerState,&m_CurrentSamplerState,sizeof(m_CurrentSamplerState));
// 
// 		if(m_ResetStateBlock == NULL)
// 		{
// 			m_device->EndStateBlock(&m_ResetStateBlock);
// 		}
// 	}
// #endif
// 
//     GNMDevice_CALL(m_device->SetVertexShaderConstantF(0, (float*)m_vsc_f, max_vsc_f));
//     GNMDevice_CALL(m_device->SetVertexShaderConstantI(0, (int*)m_vsc_i, max_vsc_i));
//     GNMDevice_CALL(m_device->SetVertexShaderConstantB(0, m_vsc_b, max_vsc_b));
//     GNMDevice_CALL(m_device->SetPixelShaderConstantF(0, (float*)m_psc_f, max_psc_f));
//     GNMDevice_CALL(m_device->SetPixelShaderConstantI(0, (int*)m_psc_i, max_psc_i));
//     GNMDevice_CALL(m_device->SetPixelShaderConstantB(0, m_psc_b, max_psc_b));
}

void GNMDevice::CreateResources()
{
#ifdef EMULATE_BEGINVERTICES_API
	// Begin*Vertices API emulation for GNM PC

	// create vertex buffer	
	m_DynamicVB = CreateVertexBuffer(DynamicVertexBufferSize, GFXUSAGE_DYNAMIC);

	// create index buffer
	m_DynamicIB = CreateIndexBuffer(DynamicIndexBufferSize, GFXUSAGE_DYNAMIC, GFXFMT_R16_UINT);
	popAssert(m_DynamicVB != NULL);
	popAssert(m_DynamicIB != NULL);

	m_DynamicVBOffset = 0;
	m_DynamicIBOffset = 0;
#endif // EMULATE_BEGINVERTICES_API

}

void GNMDevice::DeleteResources()
{
}

//----------------------------------------------------------------------------
// create all of the state related resources common to both immediate and deferred context devices
void GNMDevice::CreateStates()
{

}

//----------------------------------------------------------------------------
void GNMDevice::InitMSAASettings()
{
	for (ubiU32 i = 0; i < GFX_MULTISAMPLE_MAX; ++i)
	{
		MSAASetting* setting = &m_MSAASettings[i];
		sce::Gnm::AaSampleLocationControl aslc;
		uint32_t maxSampleDistance = 0;
		GFX_MULTISAMPLE_TYPE msaaType = (GFX_MULTISAMPLE_TYPE)i;

		sce::Gnmx::fillAaDefaultSampleLocations(&aslc, &maxSampleDistance, SamplesFromtMultiSampleType(msaaType));

		// -- @@4K Setup EQAA checkerboard
		if (msaaType == GFX_MULTISAMPLE_EQAA_CHECKERBOARD || msaaType == GFX_MULTISAMPLE_PRIM_ID_CHECKERBOARD)
		{
			// Enable 2xEQAA checkerboard.
			// Each pixel has two samples at locations (-4, 0) and (4, 0)
			// The sample ordering defines the checkerboard layout (alternated per frame)
			//
			//  ---------------------
			// |  s0  s1  |  s0  s1  |		kQuadPixelUpperLeft, kQuadPixelUpperRight
			// |----------+----------|
			// |  s1  s0  |  s1  s0  |		kQuadPixelLowerLeft, kQuadPixelLowerRight
			//  ---------------------

			ubiU32 sampleOffsetX = -4;

			aslc.setSampleLocationForPixel(Gnm::kQuadPixelUpperLeft, 0, sampleOffsetX, 0);
			aslc.setSampleLocationForPixel(Gnm::kQuadPixelUpperLeft, 1, -sampleOffsetX, 0);
			aslc.setSampleLocationForPixel(Gnm::kQuadPixelUpperRight, 0, sampleOffsetX, 0);
			aslc.setSampleLocationForPixel(Gnm::kQuadPixelUpperRight, 1, -sampleOffsetX, 0);

			aslc.setSampleLocationForPixel(Gnm::kQuadPixelLowerLeft, 0, -sampleOffsetX, 0);
			aslc.setSampleLocationForPixel(Gnm::kQuadPixelLowerLeft, 1, sampleOffsetX, 0);
			aslc.setSampleLocationForPixel(Gnm::kQuadPixelLowerRight, 0, -sampleOffsetX, 0);
			aslc.setSampleLocationForPixel(Gnm::kQuadPixelLowerRight, 1, sampleOffsetX, 0);
		}
		// --

		popMemCopy(&setting->m_SamplePositions.SampleLocs, &aslc.m_locations, sizeof(setting->m_SamplePositions.SampleLocs));
	}

	InitCRAASetting();
	m_ResetMSAASetting = false;
	m_MultiSampleType = GFX_MULTISAMPLE_NONE;
}

//----------------------------------------------------------------------------
void GNMDevice::InitCRAASetting()
{
	const ubiU32 x = 0;
	const ubiU32 y = 1;
	MSAASetting* craa8xSetting = &m_MSAASettings[GFX_MULTISAMPLE_CRAA_8x];

	// diamond sample locations copy from PS4 SDK, try other shape later
	popMemZero(craa8xSetting->m_SamplePositions.SampleLocs00, sizeof(craa8xSetting->m_SamplePositions.SampleLocs00));

	craa8xSetting->m_SamplePositions.SampleLocs00[0].x = -3;
	craa8xSetting->m_SamplePositions.SampleLocs00[0].y = 0;

	craa8xSetting->m_SamplePositions.SampleLocs00[1].x = 3;
	craa8xSetting->m_SamplePositions.SampleLocs00[1].y = 0;

	craa8xSetting->m_SamplePositions.SampleLocs00[2].x = 0;
	craa8xSetting->m_SamplePositions.SampleLocs00[2].y = 3;

	craa8xSetting->m_SamplePositions.SampleLocs00[3].x = 0;
	craa8xSetting->m_SamplePositions.SampleLocs00[3].y = -3;

	craa8xSetting->m_SamplePositions.SampleLocs00[4].x = -6;
	craa8xSetting->m_SamplePositions.SampleLocs00[4].y = 0;

	craa8xSetting->m_SamplePositions.SampleLocs00[5].x = 6;
	craa8xSetting->m_SamplePositions.SampleLocs00[5].y = 0;

	craa8xSetting->m_SamplePositions.SampleLocs00[6].x = 0;
	craa8xSetting->m_SamplePositions.SampleLocs00[6].y = 6;

	craa8xSetting->m_SamplePositions.SampleLocs00[7].x = 0;
	craa8xSetting->m_SamplePositions.SampleLocs00[7].y = -6;

	// same sample loc for 2x2 quad
	popMemCopy(craa8xSetting->m_SamplePositions.SampleLocs01, craa8xSetting->m_SamplePositions.SampleLocs00, sizeof(craa8xSetting->m_SamplePositions.SampleLocs00));
	popMemCopy(craa8xSetting->m_SamplePositions.SampleLocs10, craa8xSetting->m_SamplePositions.SampleLocs00, sizeof(craa8xSetting->m_SamplePositions.SampleLocs00));
	popMemCopy(craa8xSetting->m_SamplePositions.SampleLocs11, craa8xSetting->m_SamplePositions.SampleLocs00, sizeof(craa8xSetting->m_SamplePositions.SampleLocs00));

	// compute lut
	ubiU32 numSamples = 8;
	ubiU32 numPosibilities = 0x01 << numSamples;
	ubiVector4* CRAALut = popNewArray(ubiVector4, numPosibilities);
	popMemZero(CRAALut, sizeof(ubiVector4) * numPosibilities);

	for (ubiU32 index = 0; index < numPosibilities; ++index)
	{
		for (ubiU32 sample = 0; sample < numSamples; ++sample)
		{
			ubiU32  bit = 1 << sample;
			if (bit & index)
			{
				if (Gear::Abs((ubiS32)craa8xSetting->m_SamplePositions.SampleLocs00[sample].x) > Gear::Abs((ubiS32)craa8xSetting->m_SamplePositions.SampleLocs00[sample].y))
				{
					// horizontal edge; left or right
					if (craa8xSetting->m_SamplePositions.SampleLocs00[sample].x < 0)
						CRAALut[index](0) += 1.0f;
					else
						CRAALut[index](1) += 1.0f;
				}
				else
				{
					// vertical edge; up or down
					if (craa8xSetting->m_SamplePositions.SampleLocs00[sample].y > 0)
						CRAALut[index](2) += 1.0f;
					else
						CRAALut[index](3) += 1.0f;
				}
			}
		}
	}

	m_CRAA8xLutAddress = g_PhysMemAllocator->Alloc(sizeof(ubiVector4) * numPosibilities, sce::Gnm::kAlignmentOfBufferInBytes);
	popMemCopy(m_CRAA8xLutAddress, CRAALut, sizeof(ubiVector4) * numPosibilities);
	m_CRAA8xLutView = CreateBufferView(false, sce::Gnm::kDataFormatR32G32B32A32Float, numPosibilities);
	m_CRAA8xLutView->AssignBufferAddress(m_CRAA8xLutAddress);
}

Gnm::NumSamples Fragments2Samples(Gnm::NumFragments value) {
	switch (value) {
	case Gnm::kNumFragments1:
		return Gnm::kNumSamples1;
	case Gnm::kNumFragments2:
		return Gnm::kNumSamples2;
	case Gnm::kNumFragments4:
		return Gnm::kNumSamples4;
	case Gnm::kNumFragments8:
		return Gnm::kNumSamples8;
	default:
		return Gnm::kNumSamples1;
	}
}

//----------------------------------------------------------------------------
void GNMDevice::SetupMSAASetting(GFX_MULTISAMPLE_TYPE msaaType)
{
	if (msaaType == GFX_MULTISAMPLE_NONE)
	{
		m_ContextToFill->setScanModeControl(Gnm::kScanModeControlAaDisable, Gnm::kScanModeControlViewportScissorEnable);
		m_ContextToFill->setAaDefaultSampleLocations(Gnm::kNumSamples1);
	}
	else
	{
		sce::Gnm::NumFragments fragments = FragmentsFromMultiSampleType(m_MultiSampleType);
		sce::Gnm::NumSamples samples = SamplesFromtMultiSampleType(m_MultiSampleType);
		sce::Gnm::DepthEqaaControl eqaaCtl;
		eqaaCtl.init();

		// -- @@4K
		if (msaaType == GFX_MULTISAMPLE_EQAA_CHECKERBOARD || msaaType == GFX_MULTISAMPLE_PRIM_ID_CHECKERBOARD)
		{
			eqaaCtl.setPsSampleIterationCount(sce::Gnm::kNumSamples2);
		}
		else
		{
			eqaaCtl.setMaskExportNumSamples(samples);
			eqaaCtl.setAlphaToMaskSamples(Fragments2Samples(fragments));
			eqaaCtl.setMaxAnchorSamples(Fragments2Samples(fragments));
			eqaaCtl.setPsSampleIterationCount(sce::Gnm::kNumSamples1);
		}
		// --

		//eqaaCtl.setIncoherentEqaaReads(true);
		//eqaaCtl.setHighQualityTileIntersections(true);
		m_ContextToFill->setDepthEqaaControl(eqaaCtl);
		m_ContextToFill->setScanModeControl(sce::Gnm::kScanModeControlAaEnable, sce::Gnm::kScanModeControlViewportScissorEnable);

		MSAASetting* setting = &m_MSAASettings[msaaType];
		sce::Gnm::AaSampleLocationControl aslc;
		aslc.init(m_MSAASettings[msaaType].m_SamplePositions.SampleLocs, 0);

		// -- @@4K
		if (msaaType == GFX_MULTISAMPLE_EQAA_CHECKERBOARD || msaaType == GFX_MULTISAMPLE_PRIM_ID_CHECKERBOARD)
		{
			m_ContextToFill->setAaSampleCount(Gnm::kNumSamples2, 4);
		}
		else
		{
			aslc.updateCentroidPriority(sce::Gnm::kNumSamples8);
			aslc.computeMaxSampleDistance(sce::Gnm::kNumSamples8);
			m_ContextToFill->setAaSampleCount(sce::Gnm::kNumSamples8, aslc.computeMaxSampleDistance(sce::Gnm::kNumSamples8));
		}
		// --

		m_ContextToFill->setAaSampleLocationControl(&aslc);
	}
}

//----------------------------------------------------------------------------
void GNMDevice::SetVSC(unsigned start_reg_, const ubiVector4 *f_, unsigned num_regs_)
{
	m_StateManager->GetVSConstantsBuffer()->SetVectorsF(start_reg_, num_regs_, f_);
}

void GNMDevice::SetVSC(unsigned start_reg_, const ubiVector4I *i_, unsigned num_regs_)
{
	m_StateManager->GetVSConstantsBuffer()->SetVectorsI(start_reg_, num_regs_, i_);
}

void GNMDevice::SetVSC(unsigned start_reg_, const BOOL *b_, unsigned num_regs_)
{
	m_StateManager->GetVSConstantsBuffer()->SetBooleans(start_reg_, num_regs_, b_);
}

void GNMDevice::SetPSC(unsigned start_reg_, const ubiVector4 *f_, unsigned num_regs_)
{
	m_StateManager->GetPSConstantsBuffer()->SetVectorsF(start_reg_, num_regs_, f_);
}

void GNMDevice::GetPSC(unsigned start_reg_, ubiVector4 *f_, unsigned num_regs_)
{
	m_StateManager->GetPSConstantsBuffer()->GetVectorsF(start_reg_, num_regs_, f_);
}

void GNMDevice::SetPSC(unsigned start_reg_, const ubiVector4I *i_, unsigned num_regs_)
{
	m_StateManager->GetPSConstantsBuffer()->SetVectorsI(start_reg_, num_regs_, i_);
}

void GNMDevice::SetPSC(unsigned start_reg_, const BOOL *b_, unsigned num_regs_)
{
	m_StateManager->GetPSConstantsBuffer()->SetBooleans(start_reg_, num_regs_, b_);
}

void GNMDevice::SetDefaultLODBias(ubiFloat lodBias)
{
	m_StateManager->SetDefaultLODBias((ubiU32)lodBias);
}

void GNMDevice::SetAnisotropicFilteringOverride(ubiBool enabled, ubiU8 anisotropyDegree)
{
	m_StateManager->SetAnisotropicFilteringOverride(enabled, anisotropyDegree);
}

void GNMDevice::SetOptions(GfxexDeviceOptions& options)
{
	m_DeviceOptions = &options;
}

void GNMDevice::SetDefaults(ViewSurface* viewSurface)
{
	// set lod bias overrides
	ubiFloat lodBias = 0.0f;
	if (viewSurface->GetOptions().GetUseMipMapLODBias())
		lodBias = viewSurface->GetOptions().GetMipMapLODBias();
	SetDefaultLODBias(lodBias);

	// set anisotropic filtering overrides
	ubiBool anisotropicFilteringEnabled = false;
	ubiU8 anisotropyDegree = 1;
	if (viewSurface->GetOptions().GetUseAnisotropicFiltering())
	{
		anisotropicFilteringEnabled = true;
		anisotropyDegree = viewSurface->GetOptions().GetAnisotropyDegree();
	}

	SetAnisotropicFilteringOverride(anisotropicFilteringEnabled, anisotropyDegree);
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//@JSZ :sync from dx11
DefaultSamplerState g_DefaultSamplerStates[POP_NB_DEFAULTSAMPLERSTATES] = { GFX_SAMP_ADDRESSU, GFX_TEX_ADDRESS_WRAP,
GFX_SAMP_ADDRESSV, GFX_TEX_ADDRESS_WRAP,
GFX_SAMP_ADDRESSW, GFX_TEX_ADDRESS_WRAP,
GFX_SAMP_MINFILTER, GFX_TEX_FILTER_LINEAR,
GFX_SAMP_MAGFILTER, GFX_TEX_FILTER_LINEAR,
GFX_SAMP_MIPFILTER, GFX_TEX_FILTER_LINEAR };

void GNMDevice::RestoreDefaultSamplerStates()
{
	ubiU32 samplerChanged = m_SamplerChanged;
	m_SamplerChanged = 0;

	for (ubiU32 i = 0; i < max_samplers && samplerChanged; ++i)
	{
		if (samplerChanged & 1)
		{
			for (ubiU32 j = 0; j < POP_NB_DEFAULTSAMPLERSTATES; j++)
			{
				SetSamplerState(i, g_DefaultSamplerStates[j].m_SamplerState, g_DefaultSamplerStates[j].m_DefaultValue);
			}
		}

		samplerChanged >>= 1;
	}

	// Preserve hardcoded sampler states
	GFX_TEX_FILTER filterType = GFX_TEX_FILTER_POINT;
#ifdef POP_PLATFORM_WIN32
	if (GfxexGraphicDevice::GetInstance()->GetOptions().SupportsDepthTexture())
	{
		filterType = GFX_TEX_FILTER_LINEAR;
	}
#endif
	SetTextureFilter(SPL_BINDED_ShadowSampler, filterType, filterType, GFX_TEX_FILTER_NONE);
	SetTextureFilter(SPL_BINDED_DepthSampler, GFX_TEX_FILTER_POINT, GFX_TEX_FILTER_POINT, GFX_TEX_FILTER_NONE);

	SetTextureFilter(SPL_BINDED_WorldLightMapSampler, GFX_TEX_FILTER_LINEAR, GFX_TEX_FILTER_LINEAR, GFX_TEX_FILTER_LINEAR);
	SetTextureWrap(SPL_BINDED_WorldLightMapSampler, GFX_TEX_ADDRESS_CLAMP, GFX_TEX_ADDRESS_CLAMP, GFX_TEX_ADDRESS_CLAMP);

	SetTextureFilter(SPL_BINDED_ReflectionSampler, GFX_TEX_FILTER_LINEAR, GFX_TEX_FILTER_LINEAR, GFX_TEX_FILTER_NONE);
	SetTextureWrap(SPL_BINDED_ReflectionSampler, GFX_TEX_ADDRESS_CLAMP, GFX_TEX_ADDRESS_CLAMP, GFX_TEX_ADDRESS_CLAMP);
}

#ifdef POP_CHECK_FOR_GLOBAL_CONSTANTS_OVERRIDE

void GNMDevice::InitGlobalConstantsCheck()
{
	m_GlobalConstantMode = NotCheckingGlobalConstantsOverride;
	popMemZeroArray(m_VS_LockedConstants);
	popMemZeroArray(m_PS_LockedConstants);
}

void GNMDevice::ValidateConstantOperation(ubiBool pixelConstant, ubiU32 index, ubiU32 range)
{
	ubiBool* lockedConstants = (pixelConstant) ? m_PS_LockedConstants : m_VS_LockedConstants;

	if (m_GlobalConstantMode == CheckingGlobalConstantsOverride)
	{
		for (ubiU32 i = index; i < index + range; i++)
		{
			popAssert(!lockedConstants[i]);
		}
	}
	else if (m_GlobalConstantMode == SettingGlobalConstants)
	{
		for (ubiU32 i = index; i < index + range; i++)
		{
			lockedConstants[i] = true;
		}
	}
}

// Invalidate has been add for constant that needs to be set for multiple call, but not the whole frame
//  ie some material properties
void GNMDevice::InvalidateConstantOperation(ubiBool pixelConstant, ubiU32 index, ubiU32 range)
{
	ubiBool* lockedConstants = (pixelConstant) ? m_PS_LockedConstants : m_VS_LockedConstants;

	if (m_GlobalConstantMode == SettingGlobalConstants)
	{
		for (ubiU32 i = index; i < index + range; i++)
		{
			lockedConstants[i] = false;
		}
	}
}


void GNMDevice::BeginGlobalConstantsUpdate()
{
	m_ModeStack.Push(m_GlobalConstantMode);
	m_GlobalConstantMode = SettingGlobalConstants;
	m_StateManager->BeginGlobalConstantsUpdate();
}

void GNMDevice::EndGlobalConstantsUpdate()
{
	m_StateManager->EndGlobalConstantsUpdate();
	m_GlobalConstantMode = m_ModeStack.Pop();
}

void GNMDevice::BeginGlobalConstantsCheck()
{
	m_ModeStack.Push(m_GlobalConstantMode);
	m_GlobalConstantMode = CheckingGlobalConstantsOverride;
}

void GNMDevice::EndGlobalConstantsCheck()
{
	m_GlobalConstantMode = m_ModeStack.Pop();
}

#endif


void GNMDevice::InitContext()
{
	m_NumRingEntries = 64;
#ifdef POP_MULTITHREAD_RENDERER
	m_DcbMemorySize = GEAR_MB(16);
	m_CcbMemorySize = GEAR_MB(16);
#else
	m_DcbMemorySize = GEAR_MB(32);
	m_CcbMemorySize = GEAR_MB(32);
#endif

#ifndef USING_SEPARATE_CUE_HEAP
	const uint32_t cueHeapSize = Gnmx::ConstantUpdateEngine::computeHeapSize(m_NumRingEntries);
	// alloc cue heap memory on garlic
	for (uint32_t i = 0; i < MAX_CUE_HEAPS; i++)
		m_CueHeapMemory[i] = g_PhysMemAllocator->Alloc(cueHeapSize, 0x400);

	m_CueHeapIndex = 0;
#endif

	// alloc command buffers on onion
	for (auto i = 0; i < NumSwapchainBuffers; i++)
	{
		m_DcbMemory[i] = g_PhysMemAllocator->Alloc(m_DcbMemorySize, 0x400, PhysMemType_CPU_Cached);
		popAssert(m_DcbMemory[i] != NULL);
		m_CcbMemory[i] = g_PhysMemAllocator->Alloc(m_CcbMemorySize, 0x400, PhysMemType_CPU_Cached);
		popAssert(m_CcbMemory[i] != NULL);

		m_DcbBeginPtr[i] = (uint32_t *)m_DcbMemory[i];
		m_DcbEndPtr[i] = (uint32_t*)m_DcbBeginPtr[i] + (m_DcbMemorySize / 4);

		m_CcbBeginPtr[i] = (uint32_t *)m_CcbMemory[i];
		m_CcbEndPtr[i] = (uint32_t*)m_CcbBeginPtr[i] + (m_CcbMemorySize / 4);
	}

	m_CurrentDcbMemoryIndex = 0;
	m_CurrentCcbMemoryIndex = 0;
	m_DcbCmdPtr = m_DcbBeginPtr[0];
	m_CcbCmdPtr = m_CcbBeginPtr[0];


	m_ConextsCount = 0;
	m_NextContextHandle = 0;
	m_ContextToFill = nullptr;
	m_FirstContext = nullptr;
	ResetGnmContext();
}

#ifndef USING_SEPARATE_CUE_HEAP
void * GNMDevice::GetCueHeap(void)
{
	void* heapAddr = m_CueHeapMemory[m_CueHeapIndex];
	m_CueHeapIndex = (m_CueHeapIndex + 1) % MAX_CUE_HEAPS;
	return heapAddr;
}
#endif

GnmContext* GNMDevice::ResetGnmContext()
{
	popProfile(GnmResetGnmContext);

	if (m_IsDeferredDevice)
	{
		popAssert(m_PendingCommandLists.Size() == 0);
	}

	if (m_NextContextHandle < m_ConextsCount)
	{
		// re-use
		m_ContextToFill = GetContext(m_NextContextHandle);
		{
			//@@LRF if you need wait here means GPU is quite busy.
			m_ContextToFill->BlockUntilIdle();
		}
		popAssert(!m_ContextToFill->IsBusy());
	}
	else
	{
		// the number of chunk is not enough, create a new one.
		GnmContext *t_previous = m_ContextToFill;
		m_ContextToFill = CreateNewContext();
		if (t_previous != NULL)
		{
			t_previous->m_pNextContext = m_ContextToFill;
		}

		if (m_FirstContext == NULL)
		{
			m_FirstContext = m_ContextToFill;
		}
	}
	m_NextContextHandle++;

	m_ContextToFill->PrepareToFill();

	m_StateManager->SetAllDirtyFlags();
	m_NumDrawCalls = 0;

	return m_ContextToFill;
}


void GNMDevice::Flip(ubiBool waitUtilGPUIdle /*= false*/)
{
	if (waitUtilGPUIdle)
	{
		// stall the CPU until the GPU is finished with the frame we're entering into
		m_ContextToFill->BlockUntilIdle();
	}

	m_CurrentDcbMemoryIndex = (m_CurrentDcbMemoryIndex + 1) % NumSwapchainBuffers;
	m_CurrentCcbMemoryIndex = (m_CurrentCcbMemoryIndex + 1) % NumSwapchainBuffers;

	m_DcbCmdPtr = m_DcbBeginPtr[m_CurrentDcbMemoryIndex];
	m_CcbCmdPtr = m_CcbBeginPtr[m_CurrentCcbMemoryIndex];

	m_NextContextHandle = 0;
	m_ContextToFill = nullptr;
}

GnmContext * GNMDevice::CreateNewContext()
{
	GnmContext *pNewContext = popNew(GnmContext, "GnmContext", this);
	popAssert(pNewContext != nullptr);
	pNewContext->Initialize(this);
	pNewContext->SetHandle(m_ConextsCount);
	m_ConextsCount++;

#ifdef ENABLE_DUMP_CMDBUFFER_USAGE
	if (m_IsDeferredDevice)
	{
		g_numberOfChunk_deferred = m_ConextsCount > g_numberOfChunk_deferred ? m_ConextsCount : g_numberOfChunk_deferred;
	}
	else
	{
		g_numberOfChunk_main = m_ConextsCount;
	}
#endif

	return pNewContext;
}

void scimitar::GNMDevice::DestoryContext(GnmContext *a_context)
{
	if (a_context)
	{
		a_context->BlockUntilIdle();
		a_context->Shutdown();
		popDelete(a_context);
		m_ConextsCount--;
	}
}

GnmContext * GNMDevice::GetContext(ubiU32 a_contextHandle)
{
	popAssert(a_contextHandle >= 0 && a_contextHandle < m_ConextsCount);
	popAssert(m_FirstContext != nullptr);

	GnmContext *t_context = m_FirstContext;
	for (int i = 0; i < a_contextHandle; i++)
	{
		t_context = t_context->m_pNextContext;
		popAssert(t_context != nullptr);
	}

	popAssert(a_contextHandle == t_context->GetHandle());
	return t_context;
}

uint32_t * GNMDevice::GetCurrentDcbCmdPtr(ubiU32 a_allocationPlan)
{
#ifndef POP_OPTIMIZED
	uint32_t *t_pointer = (uint32_t*)m_DcbCmdPtr + (a_allocationPlan / 4);
	popAssert(t_pointer <= m_DcbEndPtr[m_CurrentDcbMemoryIndex]); //@@LRF The Dcb buffer is overload!!, how did you do that? try to increase m_DcbMemorySize to solve it.

#ifdef ENABLE_DUMP_CMDBUFFER_USAGE
	if (m_IsDeferredDevice)
	{
		g_totalDcbSize_deferred = m_DcbMemorySize;
		ubiU32 t_size = (t_pointer - m_DcbBeginPtr) * 4;
		g_maxDcbUsage_deferred = (t_size > g_maxDcbUsage_deferred ? t_size : g_maxDcbUsage_deferred);
	}
	else
	{
		g_totalDcbSize_main = m_DcbMemorySize;
		ubiU32 t_size = (t_pointer - m_DcbBeginPtr) * 4;
		g_maxDcbUsage_main = (t_size > g_maxDcbUsage_main ? t_size : g_maxDcbUsage_main);
	}
#endif
#endif

	return m_DcbCmdPtr;
}

uint32_t * GNMDevice::GetCurrentCcbCmdPtr(ubiU32 a_allocationPlan)
{
#ifndef POP_OPTIMIZED
	uint32_t *t_pointer = (uint32_t*)m_CcbCmdPtr + (a_allocationPlan / 4);
	popAssert(t_pointer <= m_CcbEndPtr[m_CurrentCcbMemoryIndex]); //@@LRF The Ccb buffer is overload!!, how did you do that? try to increase m_CcbMemorySize to solve it.

#ifdef ENABLE_DUMP_CMDBUFFER_USAGE
	if (m_IsDeferredDevice)
	{
		g_totalCcbSize_deferred = m_CcbMemorySize;
		ubiU32 t_size = (t_pointer - m_CcbBeginPtr) * 4;
		g_maxCcbUsage_deferred = (t_size > g_maxCcbUsage_deferred ? t_size : g_maxCcbUsage_deferred);
	}
	else
	{
		g_totalCcbSize_main = m_CcbMemorySize;
		ubiU32 t_size = (t_pointer - m_CcbBeginPtr) * 4;
		g_maxCcbUsage_main = (t_size > g_maxCcbUsage_main ? t_size : g_maxCcbUsage_main);
	}
#endif
#endif
	return m_CcbCmdPtr;
}

void GNMDevice::UpdateCmdPtr(uint32_t *a_dcbPtr, uint32_t *a_ccbPtr)
{
	popAssert(a_dcbPtr != nullptr);
	popAssert(a_ccbPtr != nullptr);
	popAssert(a_dcbPtr >= m_DcbBeginPtr[m_CurrentDcbMemoryIndex] && a_dcbPtr <= m_DcbEndPtr[m_CurrentDcbMemoryIndex]);
	popAssert(a_ccbPtr >= m_CcbBeginPtr[m_CurrentCcbMemoryIndex] && a_ccbPtr <= m_CcbEndPtr[m_CurrentCcbMemoryIndex]);
	m_DcbCmdPtr = a_dcbPtr;
	m_CcbCmdPtr = a_ccbPtr;
}

void GNMDevice::ReleaseSamplers()
{
	for (unsigned i = 0; i < GNMDevice::max_samplers; ++i)
	{
		UnsetTexture(i);
	}
}

void GNMDevice::SetPixelShader(scimitar::PixelShader* pixelShader)
{
	m_PixelShaderConstantMap = pixelShader->GetConstantMap();
	pixelShader->Set(this);
}

void GNMDevice::SetVertexShader(scimitar::VertexShader* vertexShader)
{
	m_VertexShaderConstantMap = vertexShader->GetConstantMap();
	vertexShader->Set(this);
}

void GNMDevice::SetPixelShader(PixelShaders pixelShader)
{
	popAssert(m_ShaderManager);
	SetPixelShader(m_ShaderManager->GetPixelShader(pixelShader));
}

void GNMDevice::SetVertexShader(VertexShaders vertexShader)
{
	popAssert(m_ShaderManager);
	SetVertexShader(m_ShaderManager->GetVertexShader(vertexShader));
}

void GNMDevice::SetVertexShaderInternal(PlatformGfxVertexShader* vs)
{
	SetVertexShader(vs->GetWrappedShader());
}

void GNMDevice::SetPixelShaderInternal(PlatformGfxPixelShader* ps)
{
	if (ps)
	{
		SetPixelShader(ps->GetWrappedShader());
	}
	else
	{
		SetPixelShader(Gear::RefCountedPtr<GNMWrappedResource::PixelShader>(NULL));
	}
}

void GNMDevice::CopyResource(PlatformGfxBaseTexture* dst, PlatformGfxBaseTexture* src)
{
#ifndef GNM_PORTING_TODO
	m_ContextToFill->CopyResource(dst, src);
#endif  // GNM_PORTING_TODO
}

void GNMDevice::CopySubresource(PlatformGfxBaseTexture* dst, ubiU32 dstSubresourceIndex, PlatformGfxBaseTexture* src, ubiU32 srcSubresourceIndex)
{
#ifndef GNM_PORTING_TODO
	m_ContextToFill->CopySubresourceRegion(dst, dstSubresourceIndex, 0, 0, 0, src, srcSubresourceIndex, NULL);
#endif  // GNM_PORTING_TODO
}

void GNMDevice::KickCommandBuffer(ubiBool a_waitComplete)
{
	popAssert(!m_IsDeferredDevice);

	ubiU32 t_flag = GnmContext::SUBMIT_FLAG_NONE;

	if (a_waitComplete)
	{
		t_flag = GnmContext::SUBMIT_AND_WAIT;
	}

	if (m_ContextToFill->KickCommandBuffer(t_flag))
	{
		ResetGnmContext();
	}
}

void GNMDevice::StretchRect(GfxexSurface* src, const RECT* srcRect, GfxexSurface* dest, const RECT* destRect, GFX_TEX_FILTER filter)
{
	popAssert(src);
	popAssert(dest);

	RECT	_srcRect, _destRect;

	if (srcRect)
	{
		popMemCopy(&_srcRect, srcRect, sizeof(RECT));
	}
	else
	{
		_srcRect.left = 0;
		_srcRect.right = src->GetWidth();
		_srcRect.top = 0;
		_srcRect.bottom = src->GetHeight();
	}

	if (destRect)
	{
		popMemCopy(&_destRect, destRect, sizeof(RECT));
	}
	else
	{
		_destRect.left = 0;
		_destRect.right = dest->GetWidth();
		_destRect.top = 0;
		_destRect.bottom = dest->GetHeight();
	}

	ubiFloat x = static_cast<ubiFloat>(_destRect.left);
	ubiFloat y = static_cast<ubiFloat>(_destRect.top);
	ubiFloat width = static_cast<ubiFloat>(_destRect.right - _destRect.left);
	ubiFloat height = static_cast<ubiFloat>(_destRect.bottom - _destRect.top);
	ubiFloat srcWidth = static_cast<ubiFloat>(_srcRect.right - _srcRect.left);
	ubiFloat srcHeight = static_cast<ubiFloat>(_srcRect.bottom - _srcRect.top);
	ubiFloat leftU = static_cast<ubiFloat>(_srcRect.left / srcWidth);
	ubiFloat rightU = static_cast<ubiFloat>(_srcRect.right / srcWidth);
	ubiFloat topV = static_cast<ubiFloat>(_srcRect.top / srcHeight);
	ubiFloat bottomV = static_cast<ubiFloat>(_srcRect.bottom / srcHeight);

	popAssert(x == 0.0f && y == 0.0f);

	//save old render state
	m_StateManager->PushRenderStates();

	//save the old render target and viewport
	GfxexSurface * oldrt = NULL;
	PlatformGfxViewport		oldvp, newvp;
	GetRenderTarget(0, &oldrt);
	//set dest as render target
	SetRenderTarget(0, dest);
	GetViewport(&oldvp);

	newvp.TopLeftX = x;
	newvp.TopLeftY = y;
	newvp.Width = width;
	newvp.Height = height;
	newvp.MinDepth = 0.0f;
	newvp.MaxDepth = 1.0f;
	SetViewport(newvp);

	//apply shaders 
	if (src->GetMultisampleType() == GFX_MULTISAMPLE_NONE || src->GetMultisampleType() == GFX_MULTISAMPLE_CRAA_8x)
	{
		SetVertexShader(VS_PassThrough);
		if (dest->IsFmt32Surface())
		{
			SetPixelShader(PS_PassThroughFmt32);
		}
		else
		{
			SetPixelShader(PS_PassThrough);
		}
	}
	else
	{
		SetVertexShader(VS_PassThroughMSAA);
		if (dest->IsFmt32Surface())
		{
			SetPixelShader(PS_PassThroughMSAAFmt32);
		}
		else
		{
			SetPixelShader(PS_PassThroughMSAA);
		}
	}

	//apply texture
	src->GetGfxTexture().UseTexture(0, this, filter != GFX_TEX_FILTER_POINT);
	//apply render state
	SetAlphaBlendEnabled(false);
	SetZTestEnabled(false);
	SetZWriteEnabled(false);
	SetCullingMode(GFX_CULL_MODE_NONE);

	scimitar::DrawScreenQuad(this, x / width, y / height, width / width, height / height, leftU, topV, rightU, bottomV);

	//restore the old render target and old viewport
	SetViewport(oldvp);
	SetRenderTarget(0, oldrt);

	m_StateManager->PopRenderStates();
}

// --------------------------------------------------------------------------
void GNMDevice::ApplyFilter2D(GfxexSurface* src, const RECT* srcRect, GfxexSurface* dest, const RECT* destRect, GFX_TEX_FILTER filter)
{
	popAssert(src);
	popAssert(dest);

	RECT	_srcRect, _destRect;

	if (srcRect)
	{
		popMemCopy(&_srcRect, srcRect, sizeof(RECT));
	}
	else
	{
		_srcRect.left = 0;
		_srcRect.right = src->GetWidth();
		_srcRect.top = 0;
		_srcRect.bottom = src->GetHeight();
	}

	if (destRect)
	{
		popMemCopy(&_destRect, destRect, sizeof(RECT));
	}
	else
	{
		_destRect.left = 0;
		_destRect.right = dest->GetWidth();
		_destRect.top = 0;
		_destRect.bottom = dest->GetHeight();
	}

	ubiFloat x = static_cast<ubiFloat>(_destRect.left);
	ubiFloat y = static_cast<ubiFloat>(_destRect.top);
	ubiFloat width = static_cast<ubiFloat>(_destRect.right - _destRect.left);
	ubiFloat height = static_cast<ubiFloat>(_destRect.bottom - _destRect.top);
	ubiFloat srcWidth = static_cast<ubiFloat>(_srcRect.right - _srcRect.left);
	ubiFloat srcHeight = static_cast<ubiFloat>(_srcRect.bottom - _srcRect.top);
	ubiFloat leftU = static_cast<ubiFloat>(_srcRect.left / srcWidth);
	ubiFloat rightU = static_cast<ubiFloat>(_srcRect.right / srcWidth);
	ubiFloat topV = static_cast<ubiFloat>(_srcRect.top / srcHeight);
	ubiFloat bottomV = static_cast<ubiFloat>(_srcRect.bottom / srcHeight);

	popAssert(x == 0.0f && y == 0.0f);

	//save old render state
	m_StateManager->PushRenderStates();

	//save the old render target and viewport
	GfxexSurface * oldrt = NULL;
	PlatformGfxViewport		oldvp, newvp;
	GetRenderTarget(0, &oldrt);
	//set dest as render target
	SetRenderTarget(0, dest);
	GetViewport(&oldvp);
	// reset depth stencil surface
	GfxexSurface* oldDSV = nullptr;
	GetDepthStencilSurface(&oldDSV);
	SetDepthStencilSurface(nullptr);

	newvp.TopLeftX = x;
	newvp.TopLeftY = y;
	newvp.Width = width;
	newvp.Height = height;
	newvp.MinDepth = 0.0f;
	newvp.MaxDepth = 1.0f;
	SetViewport(newvp);

	//apply texture
	src->GetGfxTexture().UseTexture(0, this, filter != GFX_TEX_FILTER_POINT);
	//apply render state
	SetAlphaBlendEnabled(false);
	SetZTestEnabled(false);
	SetZWriteEnabled(false);
	SetCullingMode(GFX_CULL_MODE_NONE);

	scimitar::DrawScreenQuad(this, x / width, y / height, width / width, height / height, leftU, topV, rightU, bottomV);

	//restore the old render target and old viewport
	SetViewport(oldvp);
	SetRenderTarget(0, oldrt);
	SetDepthStencilSurface(oldDSV);

	m_StateManager->PopRenderStates();
}

// --------------------------------------------------------------------------
void GNMDevice::ResolveMSAASurface(GfxexSurface* src, GfxexSurface* dest, GFX_MULTISAMPLE_TYPE msaaType)
{
#ifdef POP_USE_PROFILER
	ubiChar* msaaTypeNames[GFX_MULTISAMPLE_MAX] = {
		"ResolveMSAASurface_None",
		"ResolveMSAASurface_CRAA_8x",
		"ResolveMSAASurface_EQAA_2x",
		"ResolveMSAASurface_EQAA_4x",
	};
	PushMarker(msaaTypeNames[msaaType]);
#endif

	if (msaaType == GFX_MULTISAMPLE_CRAA_8x)
	{
		DecompressFmaskSurface(src);
		SetVertexShader(VS_CRAA_RESOLVE);
		SetPixelShader(PS_CRAA_RESOLVE);
		SetTexture(1, src->GetGfxTexture().GetFMaskTexture());
		SetBuffer(2, m_CRAA8xLutView.GetPtr());
	}
	else if (msaaType == GFX_MULTISAMPLE_EQAA_2x)
	{
		DecompressFmaskSurface(src);
		SetVertexShader(VS_EQAA_2X_RESOLVE);
		SetPixelShader(PS_EQAA_2X_RESOLVE);
		SetTexture(1, src->GetGfxTexture().GetFMaskTexture());
	}
	else if (msaaType == GFX_MULTISAMPLE_EQAA_4x)
	{
		DecompressFmaskSurface(src);
		SetVertexShader(VS_EQAA_4X_RESOLVE);
		SetPixelShader(PS_EQAA_4X_RESOLVE);
		SetTexture(1, src->GetGfxTexture().GetFMaskTexture());
	}
	ApplyFilter2D(src, nullptr, dest, nullptr, GFX_TEX_FILTER_POINT);
#ifdef POP_USE_PROFILER
	PopMarker();
#endif
}

// --------------------------------------------------------------------------
#include "system/memory.h"
void GNMDevice::ClearRenderTargetView(GfxexSurface* rtv, ubiFloat r, ubiFloat g, ubiFloat b, ubiFloat a)
{
	//popGpuProfile(ClearRenderTargetView, this)
	//@zyx clear rendertarget by draw a fullscreen quad
	ubiVector4Rval color(r, g, b, a);
	ubiVector4Rval tmp(0, 0, 0, 0);

	m_StateManager->PushRenderStates();

	//m_StateManager->SetRenderState(GFX_RS_DEPTH_CLEAR_ENABLE, false);
	//m_StateManager->SetRenderState(GFX_RS_STENCIL_CLEAR_ENABLE, false);
	//m_StateManager->SetDepthClearEnable(false, 0);	//@@LRF same with DX11, only clear color buffer here.
	//m_StateManager->SetStencilClearEnable(false, 0);

	//@am Fix for MRT overlap!
	GfxexSurface* mainRT;
	m_StateManager->GetRenderTarget(0, &mainRT);

	m_StateManager->SetRenderTarget(0, rtv);
	m_StateManager->SetRenderState(GFX_RS_COLORWRITEENABLE, 0xF);
	m_StateManager->SetViewport(PlatformGfxViewport(0, 0, rtv->GetRenderTargetView()->GetGNMObject()->getWidth(), rtv->GetRenderTargetView()->GetGNMObject()->getHeight(), 0.0f, 1.0f));

	// disable depth stencil test, z-write ...
	m_StateManager->SetRenderState(GFX_RS_ZENABLE, false);
	m_StateManager->SetRenderState(GFX_RS_ZFUNC, GFX_CMP_ALWAYS);
	m_StateManager->SetRenderState(GFX_RS_ZWRITEENABLE, false);
	m_StateManager->SetRenderState(GFX_RS_STENCILENABLE, false);

	if (Gnm::kDataFormatR32Float == rtv->GetRenderTargetView()->GetGNMObject()->getDataFormat())
	{
		m_StateManager->SetVertexShader(m_ShaderManager->GetVertexShader(VS_OrbisClearFmt32));
		m_StateManager->SetPixelShader(m_ShaderManager->GetPixelShader(PS_OrbisClearFmt32));
	}
	else
	{
		m_StateManager->SetVertexShader(m_ShaderManager->GetVertexShader(VS_OrbisClear));
		m_StateManager->SetPixelShader(m_ShaderManager->GetPixelShader(PS_OrbisClear));
	}

	m_StateManager->SetRenderState(GFX_RS_ALPHABLENDENABLE, false);

	m_StateManager->SetVertexDeclaration(NULL);

	GetVectorPS(0, tmp);
	// constant
	SetVectorPS(0, color);

	GFX_MULTISAMPLE_TYPE msType = m_MultiSampleType;
	m_MultiSampleType = rtv->GetMultisampleType();

	DrawPrimitive(GFXPT_FULLSCREEN_CLEAR, 0, 1);

	m_MultiSampleType = msType;

	m_StateManager->PopRenderStates();

	SetVectorPS(0, tmp);

	//@am Fix MRT overlap end
	m_StateManager->SetRenderTarget(0, mainRT);
}

void GNMDevice::ClearDepthStencilView(GfxexSurface* dsv, ubiBool clearDepth, ubiFloat depth, ubiBool clearStencil, ubiU8 stencil)
{
	//depth stencil view is not worded currently
	{
		//popGpuProfile(ClearDepthStencil, this);  //@@LRF Todo : Gfxex_GPUProfile

		// bm todo use separate DrawCommandBuffer?
		m_StateManager->PushRenderStates();

		m_StateManager->SetRenderState(GFX_RS_DEPTH_CLEAR_ENABLE, clearDepth);
		m_StateManager->SetRenderState(GFX_RS_DEPTH_CLEAR_VALUE, depth);
		m_StateManager->SetRenderState(GFX_RS_STENCIL_CLEAR_ENABLE, clearStencil);
		m_StateManager->SetRenderState(GFX_RS_STENCIL_CLEAR_VALUE, stencil);

		m_StateManager->SetRenderState(GFX_RS_ZENABLE, clearDepth);
		m_StateManager->SetRenderState(GFX_RS_ZFUNC, GFX_CMP_ALWAYS);
		m_StateManager->SetRenderState(GFX_RS_ZWRITEENABLE, clearDepth);
		m_StateManager->SetRenderState(GFX_RS_STENCILENABLE, clearStencil);
		m_StateManager->SetRenderState(GFX_RS_STENCILFUNC, GFX_CMP_ALWAYS);
		m_StateManager->SetRenderState(GFX_RS_STENCILPASS, GFX_STENCIL_OP_REPLACE);
		m_StateManager->SetRenderState(GFX_RS_STENCILFAIL, GFX_STENCIL_OP_REPLACE);
		m_StateManager->SetRenderState(GFX_RS_STENCILZFAIL, GFX_STENCIL_OP_REPLACE);
		m_StateManager->SetRenderState(GFX_RS_STENCILREF, 0xff);
		m_StateManager->SetRenderState(GFX_RS_STENCILMASK, 0xff);
		m_StateManager->SetRenderState(GFX_RS_STENCILWRITEMASK, 0xff);

		m_StateManager->SetRenderState(GFX_RS_COLORWRITEENABLE, 0);
		m_StateManager->SetDepthStencilTarget(dsv);
		m_StateManager->SetViewport(PlatformGfxViewport(0, 0, dsv->GetDepthStencilView()->GetGNMObject()->getWidth(), dsv->GetDepthStencilView()->GetGNMObject()->getHeight(), 0.0f, 1.0f));

		m_StateManager->SetPixelShader(NULL);
		m_StateManager->SetVertexShader(m_ShaderManager->GetVertexShader(VS_OrbisClear));

		m_StateManager->SetVertexDeclaration(NULL);

		DrawPrimitive(GFXPT_FULLSCREEN_CLEAR, 0, 1);

		m_StateManager->PopRenderStates();
	}

#ifndef GNM_PORTING_TODO 
#ifdef TRACK_GPU_PRIMITIVES
	m_ContextToFill->writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, (void*)ms_LastPrimitiveGPU, Gear::Atomic::Add(ms_LastPrimitiveCPU, 1), Gnm::kCacheActionNone);
#endif


#endif

#ifdef FORCE_KICK_COMMANDERBUFFER
	KickCommandBuffer();
#endif // FORCE_KICK_COMMANDERBUFFER


}

//---------------------------------------------------------------------------------------
void GNMDevice::InsertResourceBarrierForDepthRenderTarget(GfxexSurface* surface)
{
	GPUWaitOnFence(InsertFence());

	sce::Gnmx::ResourceBarrier barrier;
	barrier.init(surface->GetDepthStencilView()->GetGNMObject(), sce::Gnmx::ResourceBarrier::kUsageDepthSurface, sce::Gnmx::ResourceBarrier::kUsageRoTexture);
	barrier.enableDestinationCacheFlushAndInvalidate(true);
	GetGNMContext()->writeResourceBarrier(&barrier);

	GetGNMContext()->triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateDbMeta);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
ubiU64 GNMDevice::InsertFence()
{
	return GetGNMContext()->InsertFence();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
INLINE ubiBool GNMDevice::IsFencePending(ubiU64 fence)
{
	return GetGNMContext()->IsFencePending(fence);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::GPUWaitOnFence(ubiU64 fence)
{
	popAssert(fence != 0);
	volatile ubiU32* label = (volatile ubiU32*)fence;
	GetGNMContext()->waitOnAddress(const_cast<ubiU32*>(label), 0xffffffff, sce::Gnm::kWaitCompareFuncEqual, 0x1);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::FlushSurfaceCache(GfxexSurface* a_surface)
{
	//@@DF: use this for the moment, maybe use the same way as in ACBF later
	GetGNMContext()->FlushGPUCaches();
}

//---------------------------------------------------------------------------------------
void GNMDevice::InvalidateGPUCache()
{
	GetGNMContext()->FlushGPUCaches();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::ResummarizeHtile(GfxexSurface* src)
{
	GNMResummarizeHtile(m_ContextToFill, src->GetDepthStencilView()->GetGNMObject());
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::DecompressDepthSurface(GfxexSurface* src, GfxexSurface* dest, ubiBool resummarizeHtile)
{
	if (src == dest)
	{
		GNMDecompressDepthSurface(m_ContextToFill, src->GetDepthStencilView()->GetGNMObject(), resummarizeHtile);
	}
	else
	{
		GNMDecompressDepthSurfaceToCopy(m_ContextToFill, dest->GetDepthStencilView()->GetGNMObject(), src->GetDepthStencilView()->GetGNMObject(), resummarizeHtile);
	}

}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::DecompressFmaskSurface(GfxexSurface* src)
{
	GNMDecompressFmaskSurface(m_ContextToFill, src->GetRenderTargetView()->GetGNMObject());
	m_ContextToFill->FlushGPUCaches();
	m_ContextToFill->FlushGPUCaches();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::PushRenderStates()
{
	m_StateManager->PushRenderStates();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
void GNMDevice::PopRenderStates()
{
	m_StateManager->PopRenderStates();
}
//---------------------------------------------------------------------------------------

//----------------------------------------------------------------------------
void GNMDevice::SetViewport(ubiU32 x, ubiU32 y, ubiU32 width, ubiU32 height, ubiFloat minz, ubiFloat maxz)
{
	// Set viewport params
	PlatformGfxViewport vp;
	vp.TopLeftX = (ubiFloat)x;
	vp.TopLeftY = (ubiFloat)y;
	vp.Width = (ubiFloat)width;
	vp.Height = (ubiFloat)height;
	vp.MinDepth = (ubiFloat)minz;
	vp.MaxDepth = (ubiFloat)maxz;

	//apply the viewport
	SetViewport(vp);
}
//----------------------------------------------------------------------------


popEND_NAMESPACE

#endif // POP_PLATFORM_GNM
