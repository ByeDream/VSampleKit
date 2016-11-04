#include "stdafx.h"

#include "Application.h"
#include "GraphicDevice.h"




//----------------------
#include <video_out.h>
#include <gnmx/shader_parser.h>
#include "std_cbuffer.h"

#include "dataformat_interpreter.h"

#include "tga_texture_loader.h"

// Set default heap size
size_t sceLibcHeapSize = UTIL_MB(512); // 512MB for Razor GPU
unsigned int sceLibcHeapExtendedAlloc = 1; // Enable
//----------------------

using namespace sce;



Framework::Application::Application()
: m_eopEventQueue("Framework Main Thread Queue")
{
	
}

Framework::Application::~Application()
{

}

bool Framework::Application::initialize(const char *name, int argc, const char* argv[])
{
	mName = name;
	processCommandLine(argc, argv);

	mGraphicDevice = new GraphicDevice(this);
	mGraphicDevice->init();
	

	Vector3 m_lightPositionX(1.5f, 4.0f, 9.0f);
	Vector3 m_lightTargetX(0.0f, 0.0f, 0.0f);
	Vector3 m_lightUpX(0.0f, 1.0f, 0.0f);
 	m_lightToWorldMatrix = inverse(Matrix4::lookAt((Point3)m_lightPositionX, (Point3)m_lightTargetX, m_lightUpX));
	float m_depthNear = 1.f;
	float m_depthFar = 100.f;
 	const float aspect = (float)m_targetWidth / (float)m_targetHeight;
 	m_projectionMatrix = Matrix4::frustum(-aspect, aspect, -1, 1, m_depthNear, m_depthFar);
	Vector3 m_lookAtPosition(0, 0, 2.5f);
	Vector3 m_lookAtTarget(0, 0, 0);
	Vector3 m_lookAtUp(0, 1, 0);
 	SetViewToWorldMatrix(inverse(Matrix4::lookAt((Point3)m_lookAtPosition, (Point3)m_lookAtTarget, m_lookAtUp)));

// 	DebugObjects::initializeWithAllocators(&m_allocators);
// 
// 	{
// 		s_embeddedShaders.initializeWithAllocators(&m_allocators);
// 		for (unsigned i = 0; i < s_embeddedShaders.m_embeddedVsShaders; ++i)
// 		{
// 			Gnmx::Toolkit::EmbeddedVsShader *embeddedVsShader = s_embeddedShaders.m_embeddedVsShader[i];
// 			m_allocators.allocate((void**)&embeddedVsShader->m_fetchShader, SCE_KERNEL_WC_GARLIC, Gnmx::computeVsFetchShaderSize(embeddedVsShader->m_shader), Gnm::kAlignmentOfBufferInBytes, Gnm::kResourceTypeFetchShaderBaseAddress, "Framework Fetch Shader %d", i);
// 			uint32_t shaderModifier;
// 			Gnmx::generateVsFetchShader(embeddedVsShader->m_fetchShader, &shaderModifier, embeddedVsShader->m_shader);
// 			embeddedVsShader->m_shader->applyFetchShaderModifier(shaderModifier);
// 		}
// 	}
 
 
// 		Gnm::SizeAlign screenSizeAlign = m_buffer[buffer].m_screen.calculateRequiredBufferSizeAlign(screenCharactersWide, screenCharactersHigh);
// 		void *screenPtr = nullptr;
// 		m_allocators.allocate(&screenPtr, SCE_KERNEL_WB_ONION, screenSizeAlign, Gnm::kResourceTypeBufferBaseAddress, "Framework Buffer %d Screen", buffer);
// 
// 		m_buffer[buffer].m_screen.initialize(screenPtr, screenCharactersWide, screenCharactersHigh);
// 		enum { kOverscan = 2 };
// 		DbgFont::Window fullScreenWindow;
// 		fullScreenWindow.initialize(&m_buffer[buffer].m_screen);
// 		m_buffer[buffer].m_window.initialize(&fullScreenWindow, kOverscan, kOverscan, screenCharactersWide - kOverscan * 2, screenCharactersHigh - kOverscan * 2);
// 
// 		m_buffer[buffer].m_timers.initialize(static_cast<uint8_t*>(timerPtr) + timerSizeAlign.m_size / m_config.m_buffers * buffer, kTimers);
// 
// 		m_buffer[buffer].m_debugObjects.Init(&m_onionAllocator, &m_sphereMesh, &m_coneMesh, &m_cylinderMesh);
	SCE_GNM_ASSERT_MSG(ret >= 0, "sceVideoOutRegisterBuffers() returned error code %d.", ret);

	enum { kCommandBufferSizeInBytes = 1 * 1024 * 1024 };
	//SCE_GNM_ASSERT(framework.m_config.m_buffers <= 3);
	for (unsigned buffer = 0; buffer < m_buffers; ++buffer)
	{
		Frame *frame = &m_frames[buffer];
		// createCommandBuffer
		const uint32_t kNumRingEntries = 64;
		const uint32_t cueHeapSize = Gnmx::ConstantUpdateEngine::computeHeapSize(kNumRingEntries);
		void *constantUpdateEngine;
		Gnm::ResourceHandle cueHandle;
		void *drawCommandBuffer;
		Gnm::ResourceHandle dcbHandle;
		void *constantCommandBuffer;
		Gnm::ResourceHandle ccbHandle;
		// 4byte align
		mAllocators->allocate(&constantUpdateEngine, SCE_KERNEL_WC_GARLIC, cueHeapSize, 4, Gnm::kResourceTypeGenericBuffer, &cueHandle, "Buffer %d Constant Update Engine", buffer);
		mAllocators->allocate(&drawCommandBuffer, SCE_KERNEL_WB_ONION, kCommandBufferSizeInBytes, 4, Gnm::kResourceTypeDrawCommandBufferBaseAddress, &dcbHandle, "Buffer %d Draw Command Buffer", buffer);
		mAllocators->allocate(&constantCommandBuffer, SCE_KERNEL_WB_ONION, kCommandBufferSizeInBytes, 4, Gnm::kResourceTypeConstantCommandBufferBaseAddress, &ccbHandle, "Buffer %d Constant Command Buffer", buffer);
		frame->m_commandBuffer.init(constantUpdateEngine, kNumRingEntries, drawCommandBuffer, kCommandBufferSizeInBytes, constantCommandBuffer, kCommandBufferSizeInBytes);

		//createCommandBuffer(&frame->m_commandBuffer, &framework, buffer);
		Gnm::ResourceHandle constantsHandle0;
		mAllocators->allocate((void**)&frame->m_constants[0], SCE_KERNEL_WB_ONION, sizeof(Constants), 4, Gnm::kResourceTypeConstantBufferBaseAddress, &constantsHandle0, "Buffer %d Command Buffer 0", buffer);
		frame->m_constantBuffer[0].initAsConstantBuffer(frame->m_constants[0], sizeof(Constants));
		frame->m_constantBuffer[0].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK
		
		Gnm::ResourceHandle constantsHandle1;
		mAllocators->allocate((void**)&frame->m_constants[1], SCE_KERNEL_WB_ONION, sizeof(Constants), 4, Gnm::kResourceTypeConstantBufferBaseAddress, &constantsHandle1, "Buffer %d Command Buffer 1", buffer);
		frame->m_constantBuffer[1].initAsConstantBuffer(frame->m_constants[1], sizeof(Constants));
		frame->m_constantBuffer[1].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK

	}

	vertexShader = LoadVsShader("/app0/shader_vv.sb", mAllocators);
	pixelShader = LoadPsShader("/app0/shader_p.sb", mAllocators);

	set_uint = LoadCsShader("/app0/cs_set_uint_c.sb", mAllocators);
	set_uint_fast = LoadCsShader("/app0/cs_set_uint_fast_c.sb", mAllocators);
	pix_clear_p = LoadPsShader("/app0/pix_clear_p.sb", mAllocators);

 	Framework::TgaError loadError = Framework::kTgaErrorNone;
 	loadError = Framework::loadTextureFromTga(&texturesEarth[0], "/app0/pab_ground_soil_001_c.tga", mAllocators);
 	SCE_GNM_ASSERT(loadError == Framework::kTgaErrorNone);
 	loadError = Framework::loadTextureFromTga(&texturesEarth[1], "/app0/pab_ground_soil_001_n.tga", mAllocators);
 	SCE_GNM_ASSERT(loadError == Framework::kTgaErrorNone);
	loadError = Framework::loadTextureFromTga(&texturesEarth[2], "/app0/pab_ground_soil_001_s.tga", mAllocators);
	SCE_GNM_ASSERT(loadError == Framework::kTgaErrorNone);
 
	texturesEarth[0].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it's OK to mark it as read-only.
	texturesEarth[1].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it's OK to mark it as read-only.
	texturesEarth[2].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it's OK to mark it as read-only.


	loadError = Framework::loadTextureFromTga(&texturesMetal[0], "/app0/pro_metal_checker_plate_001_c.tga", mAllocators);
	SCE_GNM_ASSERT(loadError == Framework::kTgaErrorNone);
	loadError = Framework::loadTextureFromTga(&texturesMetal[1], "/app0/pro_metal_checker_plate_001_n.tga", mAllocators);
	SCE_GNM_ASSERT(loadError == Framework::kTgaErrorNone);
	loadError = Framework::loadTextureFromTga(&texturesMetal[2], "/app0/pro_metal_checker_plate_001_s.tga", mAllocators);
	SCE_GNM_ASSERT(loadError == Framework::kTgaErrorNone);

	texturesMetal[0].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it's OK to mark it as read-only.
	texturesMetal[1].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it's OK to mark it as read-only.
	texturesMetal[2].setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // this texture is never bound as an RWTexture, so it's OK to mark it as read-only.

 	trilinearSampler.init();
 	trilinearSampler.setMipFilterMode(Gnm::kMipFilterModeLinear);
 	trilinearSampler.setXyFilterMode(Gnm::kFilterModeBilinear, Gnm::kFilterModeBilinear);
 
	m_mesh0 = new SimpleMesh;
//	BuildCubeMesh(mAllocators, "Cube", m_mesh, 1.5f);
//	BuildTorusMesh(mAllocators, "Torus", m_mesh, 1.0f, 0.3f, 64, 32, 4, 1);
//	BuildQuadMesh(mAllocators, "Quad", m_mesh, 2.0f);
	BuildSphereMesh(mAllocators, "Sphere", m_mesh0, 1.2f, 64, 64);
	m_mesh1 = new SimpleMesh;
	BuildSphereMesh(mAllocators, "Sphere", m_mesh1, 1.2f, 64, 64);
// 
// 	Frame *frame = m_frames + framework.getIndexOfBufferCpuIsWriting();
// 	Gnmx::GnmxGfxContext *gfxc = &frame->m_commandBuffer;
// 	framework.terminate(*gfxc);
	return true;
}

bool Framework::Application::frame()
{
	// 	while (!framework.m_shutdownRequested)
	// 	{
 	 		Buffer *bufferCpuIsWriting = m_buffer + m_indexOfBufferCpuIsWriting;
 	 		Frame *frame = m_frames + m_indexOfBufferCpuIsWriting;
 	 		Gnmx::GnmxGfxContext *gfxc = &frame->m_commandBuffer;
	 
	 		gfxc->reset();
			gfxc->initializeDefaultHardwareState();
	 		//framework.BeginFrame(*gfxc);
			// ExecuteKeyboardOptions();
			gfxc->waitUntilSafeForRendering(m_videoInfo.handle, m_videoInfo.flip_index_base + m_indexOfBufferCpuIsWriting);
#ifdef ENABLE_RAZOR_GPU_THREAD_TRACE
			// begin trace of this frame.
			if (m_traceFrame) {

				// populate parameters structure for Razor GPU Thread Trace.
				SceRazorGpuThreadTraceParams threadTraceParams;
				memset(&threadTraceParams, 0, sizeof(SceRazorGpuThreadTraceParams));
				threadTraceParams.sizeInBytes = sizeof(SceRazorGpuThreadTraceParams);

				// set up SQ counters for this trace.
				threadTraceParams.numCounters = 0;
				threadTraceParams.counterRate = SCE_RAZOR_GPU_THREAD_TRACE_COUNTER_RATE_HIGH;

				// set up instruction issue tracing, 
				// NOTE: change this to true and set the shaderEngine0ComputeUnitIndex below to enable instruction issue tracing.
				threadTraceParams.enableInstructionIssueTracing = false;

				// initialize thread trace.
				sceRazorGpuThreadTraceInit(&threadTraceParams);
				sceRazorGpuThreadTraceStart(&gfxc.m_dcb);
			}
#endif // #ifdef ENABLE_RAZOR_GPU_THREAD_TRACE
			
			//--------------------------------- 
	 		// Render the scene:
	 		Gnm::PrimitiveSetup primSetupReg;
	 		primSetupReg.init();
	 		primSetupReg.setCullFace(Gnm::kPrimitiveSetupCullFaceBack);
	 		primSetupReg.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCcw);
	 		gfxc->setPrimitiveSetup(primSetupReg);
	 
	 		// Clear
	 		clearRenderTarget(*gfxc, &bufferCpuIsWriting->m_renderTarget, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	 		clearDepthTarget(*gfxc, &bufferCpuIsWriting->m_depthTarget, 1.f);
	 		gfxc->setRenderTargetMask(0xF);
	 		gfxc->setActiveShaderStages(Gnm::kActiveShaderStagesVsPs);
	 		gfxc->setRenderTarget(0, &bufferCpuIsWriting->m_renderTarget);
	 		gfxc->setDepthRenderTarget(&bufferCpuIsWriting->m_depthTarget);
	 
			Gnm::DepthStencilControl dsc;
			dsc.init();
			dsc.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncLess);
			dsc.setDepthEnable(true);
			gfxc->setDepthStencilControl(dsc);
			gfxc->setupScreenViewport(0, 0, bufferCpuIsWriting->m_renderTarget.getWidth(), bufferCpuIsWriting->m_renderTarget.getHeight(), 0.5f, 0.5f);
	 
	 		gfxc->setVsShader(vertexShader->m_shader, 0, vertexShader->m_fetchShader, &vertexShader->m_offsetsTable);
	 		gfxc->setPsShader(pixelShader->m_shader, &pixelShader->m_offsetsTable);
	 
			m_mesh0->SetVertexBuffer(*gfxc, Gnm::kShaderStageVs);
	 

			static float radians = 0;
			radians += 0.001f;
	 		//const float radians = framework.GetSecondsElapsedApparent() * 0.5f;
	 		const Matrix4 r = Matrix4::rotationZYX(Vector3(radians, radians, 0.f));
			const Matrix4 t0 = Matrix4::translation(Vector3(-1.2f, 0.0f, 0.0f));
			const Matrix4 t1 = Matrix4::translation(Vector3(1.2f, 0.0f, 0.0f));
			const Matrix4 m0 = t0 * r;
			const Matrix4 m1 = t1 * r;
	 		Constants *constants = frame->m_constants[0];
			constants->m_modelView = transpose(m_worldToViewMatrix*m0);
	 		constants->m_modelViewProjection = transpose(m_viewProjectionMatrix*m0);
			// a sample point light
			Vector4 lightPosition(0.f, 0.f, 0.f, 1.f);
			Vector4 lightPosition_W = m_lightToWorldMatrix * lightPosition;
			Vector4 lightPosition_V = m_worldToViewMatrix * lightPosition_W;
	 		constants->m_lightPosition = lightPosition_V; // use view space in this case, whatever using view/world space, the inputs of illum computation should be in the same space.
	 		constants->m_lightColor = Vector4(0.5f, 0.5f, 0.5f, 0.5f); // combines the diffuse color and specular color for simplified modeling 
	 		constants->m_ambientColor = Vector4(0.0f, 0.0f, 0.0078f, 0.0078f); // global ambient, instead of computation with contribution component of each light, thus we only compute it once per pixel instead of per light.
			//float attenuation = saturate(1.0f / (lightAtten.x + lightAtten.y * d + lightAtten.z * d * d) - lightAtten.w);
			constants->m_lightAttenuation = Vector4(1, 0, 0, 0);
	
	 		gfxc->setConstantBuffers(Gnm::kShaderStageVs, 0, 1, &frame->m_constantBuffer[0]);
	 		gfxc->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &frame->m_constantBuffer[0]);
	// 
	 		gfxc->setTextures(Gnm::kShaderStagePs, 0, 3, texturesEarth);
	 		gfxc->setSamplers(Gnm::kShaderStagePs, 0, 1, &trilinearSampler);
	// 
	 		gfxc->setPrimitiveType(m_mesh0->m_primitiveType);
	 		gfxc->setIndexSize(m_mesh0->m_indexType);
	 		gfxc->drawIndex(m_mesh0->m_indexCount, m_mesh0->m_indexBuffer);
	// 		framework.EndFrame(*gfxc);


			constants = frame->m_constants[1];
			constants->m_modelView = transpose(m_worldToViewMatrix*m1);
			constants->m_modelViewProjection = transpose(m_viewProjectionMatrix*m1);
			// a sample point light
			constants->m_lightPosition = lightPosition_V; // use view space in this case, whatever using view/world space, the inputs of illum computation should be in the same space.
			constants->m_lightColor = Vector4(0.5f, 0.5f, 0.5f, 0.5f); // combines the diffuse color and specular color for simplified modeling 
			constants->m_ambientColor = Vector4(0.0f, 0.0f, 0.0078f, 0.0078f); // global ambient, instead of computation with contribution component of each light, thus we only compute it once per pixel instead of per light.
																			   //float attenuation = saturate(1.0f / (lightAtten.x + lightAtten.y * d + lightAtten.z * d * d) - lightAtten.w);
			constants->m_lightAttenuation = Vector4(1, 0, 0, 0);

			gfxc->setConstantBuffers(Gnm::kShaderStageVs, 0, 1, &frame->m_constantBuffer[1]);
			gfxc->setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &frame->m_constantBuffer[1]);

			gfxc->setTextures(Gnm::kShaderStagePs, 0, 3, texturesMetal);
			m_mesh1->SetVertexBuffer(*gfxc, Gnm::kShaderStageVs);
			gfxc->setPrimitiveType(m_mesh1->m_primitiveType);
			gfxc->setIndexSize(m_mesh1->m_indexType);
			gfxc->drawIndex(m_mesh1->m_indexCount, m_mesh1->m_indexBuffer);

#ifdef ENABLE_RAZOR_GPU_THREAD_TRACE
	// end trace of this frame.
			if (m_traceFrame)
				sceRazorGpuThreadTraceStop(&gfxc.m_dcb);

			// create a sync point
			// the GPU will write "1" to a label to signify that the trace has finished
			volatile uint64_t *label = (uint64_t*)gfxc.m_dcb.allocateFromCommandBuffer(8, Gnm::kEmbeddedDataAlignment8);
			*label = 0;
			gfxc.writeImmediateAtEndOfPipe(Gnm::kEopFlushCbDbCaches, (void*)label, 1, Gnm::kCacheActionNone);
#endif // #ifdef ENABLE_RAZOR_GPU_THREAD_TRACE


#ifdef ENABLE_RAZOR_GPU_THREAD_TRACE
			// wait for the trace to finish
			uint32_t wait = 0;
			while (*label != 1)
				++wait;

			// save the thread trace file for the frame and reset ready to perform another trace.
			if (m_traceFrame) {
				SceRazorGpuErrorCode errorCode = SCE_OK;
				errorCode = sceRazorGpuThreadTraceSave(RAZOR_GPU_THREAD_TRACE_FILENAME);
				if (errorCode != SCE_OK) {
					m_backBuffer->m_window.printf("Unable to save Razor GPU Thread Trace file. [%h]\n", errorCode);
				}
				else {
					m_backBuffer->m_window.printf("Saved Razor GPU Thread Trace file to %s.\n", RAZOR_GPU_THREAD_TRACE_FILENAME);
				}

				sceRazorGpuThreadTraceShutdown();
				m_traceFrame = false;
			}
#endif // #ifdef ENABLE_RAZOR_GPU_THREAD_TRACE
	// 	}
	return false;
}

bool Framework::Application::terminate()
{
// 	Frame *frame = frames + framework.getIndexOfBufferCpuIsWriting();
// 	Gnmx::GnmxGfxContext *gfxc = &frame->m_commandBuffer;
// 	framework.terminate(*gfxc);


	mGarlicStackAllocator->deinit();
	mOnionStackAllocator->deinit();

	SAFE_DELETE(mAllocators);
	SAFE_DELETE(mGarlicStackAllocator);
	SAFE_DELETE(mOnionStackAllocator);
	return true;
}

void Framework::Application::processCommandLine(int argc, const char* argv[])
{
	//TODO
	mConfig.mOnionMemoryInBytes			= UTIL_MB(256);
	mConfig.mGarlicMemoryInBytes		= UTIL_MB(512);
	mConfig.mTargetWidth				= 1920;
	mConfig.mTargetHeight				= 1080;

	mConfig.mNumberOfSwappedBuffers		= 3;
	mConfig.mAsynchronousRendering		= true;
}

Framework::EmbeddedVsShader * Framework::Application::LoadVsShader(const char* filename, Allocators *allocators)
{
	void* pointer;
	uint32_t bytes;
	AcquireFileContents(pointer, bytes, filename);
	EmbeddedVsShader *embeddedVsShader = new EmbeddedVsShader;
	embeddedVsShader->m_source = (uint32_t*)pointer;
	embeddedVsShader->m_name = filename;
	embeddedVsShader->initializeWithAllocators(allocators);

	ReleaseFileContents(pointer);

	allocators->allocate(&(embeddedVsShader->m_fetchShader), SCE_KERNEL_WC_GARLIC, embeddedVsShader->m_fetchShaderSize, Gnm::kAlignmentOfBufferInBytes, Gnm::kResourceTypeFetchShaderBaseAddress, &embeddedVsShader->mFetchShaderHandle, embeddedVsShader->m_name);
 	uint32_t shaderModifier;
 	Gnmx::generateVsFetchShader(embeddedVsShader->m_fetchShader, &shaderModifier, embeddedVsShader->m_shader, (Gnm::FetchShaderInstancingMode *)nullptr, 0); // Todo table
 	embeddedVsShader->m_shader->applyFetchShaderModifier(shaderModifier);
	return embeddedVsShader;
}

Framework::EmbeddedPsShader * Framework::Application::LoadPsShader(const char* filename, Allocators *allocators)
{
	void* pointer;
	uint32_t bytes;
	AcquireFileContents(pointer, bytes, filename);
	EmbeddedPsShader *embeddedPsShader = new EmbeddedPsShader;
	embeddedPsShader->m_source = (uint32_t*)pointer;
	embeddedPsShader->m_name = filename;

	embeddedPsShader->initializeWithAllocators(allocators);
	ReleaseFileContents(pointer);
	return embeddedPsShader;
}

Framework::EmbeddedCsShader * Framework::Application::LoadCsShader(const char* filename, Allocators *allocators)
{
	void* pointer;
	uint32_t bytes;
	AcquireFileContents(pointer, bytes, filename);
	EmbeddedCsShader *embeddedCsShader = new EmbeddedCsShader;
	embeddedCsShader->m_source = (uint32_t*)pointer;
	embeddedCsShader->m_name = filename;
	embeddedCsShader->initializeWithAllocators(allocators);
	ReleaseFileContents(pointer);
	return embeddedCsShader;
}

void Framework::Application::AcquireFileContents(void*& pointer, U32& bytes, const char* filename)
{
	//	SCE_GNM_ASSERT_MSG(access(filename, R_OK) == 0, "** Asset Not Found: %s\n", filename);
	FILE *fp = fopen(filename, "rb");
	SCE_GNM_ASSERT(fp);
	fseek(fp, 0, SEEK_END);
	bytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	pointer = malloc(bytes);
	fread(pointer, 1, bytes, fp);
	fclose(fp);
}

void Framework::Application::ReleaseFileContents(void *pointer)
{
	free(pointer);
}

void Framework::EmbeddedVsShader::initializeWithAllocators(Allocators *allocators)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary;
	void *shaderHeader;
	allocators->allocate(&shaderBinary, SCE_KERNEL_WC_GARLIC, shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, Gnm::kResourceTypeShaderBaseAddress, &mVsShaderHandle, m_name);
	allocators->allocate(&shaderHeader, SCE_KERNEL_WB_ONION, shaderInfo.m_vsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_vsShader, shaderInfo.m_vsShader->computeSize());

	m_shader = static_cast<Gnmx::VsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);

	Gnmx::generateInputOffsetsCache(&m_offsetsTable, Gnm::kShaderStageVs, m_shader);

	m_fetchShaderSize = Gnmx::computeVsFetchShaderSize(shaderInfo.m_vsShader);
}

void Framework::EmbeddedPsShader::initializeWithAllocators(Allocators *allocators)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary;
	void *shaderHeader;
	allocators->allocate(&shaderBinary, SCE_KERNEL_WC_GARLIC, shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, Gnm::kResourceTypeShaderBaseAddress, &mPsShaderHandle, m_name);
	allocators->allocate(&shaderHeader, SCE_KERNEL_WB_ONION, shaderInfo.m_psShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);


	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_psShader, shaderInfo.m_psShader->computeSize());

	m_shader = static_cast<Gnmx::PsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);

	Gnmx::generateInputOffsetsCache(&m_offsetsTable, Gnm::kShaderStagePs, m_shader);
}

sce::Gnm::SizeAlign Framework::Application::InitRenderTarget(sce::Gnm::RenderTarget *renderTarget, uint32_t width, uint32_t height, uint32_t numSlices, sce::Gnm::DataFormat rtFormat, sce::Gnm::TileMode tileModeHint, sce::Gnm::NumSamples numSamples, sce::Gnm::NumFragments numFragments, sce::Gnm::SizeAlign *cmaskSizeAlign, sce::Gnm::SizeAlign *fmaskSizeAlign)
{
	sce::Gnm::RenderTargetSpec spec;
	spec.init();
	spec.m_width = width;
	spec.m_height = height;
	spec.m_pitch = 0; // pitch
	spec.m_numSlices = numSlices;
	spec.m_colorFormat = rtFormat;
	spec.m_colorTileModeHint = tileModeHint;
	spec.m_minGpuMode = sce::Gnm::getGpuMode();
	spec.m_numSamples = numSamples;
	spec.m_numFragments = numFragments;
	spec.m_flags.enableCmaskFastClear = (cmaskSizeAlign != NULL) ? 1 : 0;
	spec.m_flags.enableFmaskCompression = (fmaskSizeAlign != NULL) ? 1 : 0;
	int32_t status = renderTarget->init(&spec);
	if (status != SCE_GNM_OK)
		return sce::Gnm::SizeAlign(0, 0);
	if (cmaskSizeAlign != NULL)
		*cmaskSizeAlign = renderTarget->getCmaskSizeAlign();
	if (fmaskSizeAlign != NULL)
		*fmaskSizeAlign = renderTarget->getFmaskSizeAlign();
	return renderTarget->getColorSizeAlign();
}

sce::Gnm::SizeAlign Framework::Application::InitDepthRenderTarget(sce::Gnm::DepthRenderTarget *depthRenderTarget, uint32_t width, uint32_t height, sce::Gnm::ZFormat zFormat, sce::Gnm::StencilFormat stencilFormat, sce::Gnm::TileMode depthTileModeHint, sce::Gnm::NumFragments numFragments, sce::Gnm::SizeAlign *stencilSizeAlign, sce::Gnm::SizeAlign *htileSizeAlign)
{
	sce::Gnm::DepthRenderTargetSpec spec;
	spec.init();
	spec.m_width = width;
	spec.m_height = height;
	spec.m_pitch = 0;  // pitch
	spec.m_numSlices = 1.0;  // numSlices
	spec.m_zFormat = zFormat;
	spec.m_stencilFormat = stencilFormat;
	spec.m_tileModeHint = depthTileModeHint;
	spec.m_minGpuMode = sce::Gnm::getGpuMode();
	spec.m_numFragments = numFragments;
	spec.m_flags.enableHtileAcceleration = (htileSizeAlign != NULL) ? 1 : 0;
	int32_t status = depthRenderTarget->init(&spec);
	if (status != SCE_GNM_OK)
		return sce::Gnm::SizeAlign(0, 0);
	if (stencilSizeAlign)
		*stencilSizeAlign = depthRenderTarget->getStencilSizeAlign();
	if (htileSizeAlign)
		*htileSizeAlign = depthRenderTarget->getHtileSizeAlign();
	return depthRenderTarget->getZSizeAlign();
}

void Framework::Application::clearRenderTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::RenderTarget* renderTarget, const Vector4& color)
{
	gfxc.pushMarker("clearRenderTarget");
 	uint32_t *source = static_cast<uint32_t*>(gfxc.allocateFromCommandBuffer(sizeof(uint32_t) * 4, Gnm::kEmbeddedDataAlignment4));
 	uint32_t dwords = 1;
 	dataFormatEncoder(source, &dwords, (Reg32*)&color, renderTarget->getDataFormat());
 	//clearRenderTarget(gfxc, renderTarget, source, dwords);

	// NOTE: this slice count is only valid if the array view hasn't changed since initialization!
	const uint32_t numSlices = renderTarget->getLastArraySliceIndex() - renderTarget->getBaseArraySliceIndex() + 1;
	clearMemoryToUints(gfxc, renderTarget->getBaseAddress(), renderTarget->getSliceSizeInBytes()*numSlices / sizeof(uint32_t), source, dwords);
	gfxc.popMarker();
}

void Framework::Application::clearDepthTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const float depthValue)
{
	gfxc.pushMarker("clearDepthTarget");

	Gnm::DbRenderControl dbRenderControl;
	dbRenderControl.init();
	dbRenderControl.setDepthClearEnable(true);
	gfxc.setDbRenderControl(dbRenderControl);

	Gnm::DepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncAlways);
	depthControl.setStencilFunction(Gnm::kCompareFuncNever);
	depthControl.setDepthEnable(true);
	gfxc.setDepthStencilControl(depthControl);

	gfxc.setDepthClearValue(depthValue);

// 	SCE_GNM_ASSERT_MSG(s_initialized, "Must call SurfaceUtil::initialize() before calling this function.");
 	SCE_GNM_ASSERT(pix_clear_p->m_shader);

	gfxc.setRenderTargetMask(0x0);

	gfxc.setPsShader(pix_clear_p->m_shader, &pix_clear_p->m_offsetsTable);

	Vector4Unaligned* constantBuffer = static_cast<Vector4Unaligned*>(gfxc.allocateFromCommandBuffer(sizeof(Vector4Unaligned), Gnm::kEmbeddedDataAlignment4));
	*constantBuffer = Vector4(0.f, 0.f, 0.f, 0.f);
	Gnm::Buffer buffer;
	buffer.initAsConstantBuffer(constantBuffer, sizeof(Vector4Unaligned));
	buffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK
	gfxc.setConstantBuffers(Gnm::kShaderStagePs, 0, 1, &buffer);

	const uint32_t width = depthTarget->getWidth();
	const uint32_t height = depthTarget->getHeight();
	gfxc.setupScreenViewport(0, 0, width, height, 0.5f, 0.5f);
	const uint32_t firstSlice = depthTarget->getBaseArraySliceIndex();
	const uint32_t lastSlice = depthTarget->getLastArraySliceIndex();
	Gnm::DepthRenderTarget dtCopy = *depthTarget;
	for (uint32_t iSlice = firstSlice; iSlice <= lastSlice; ++iSlice)
	{
		dtCopy.setArrayView(iSlice, iSlice);
		gfxc.setDepthRenderTarget(&dtCopy);
		Gnmx::renderFullScreenQuad(&gfxc);
	}

	// pop state
	gfxc.setRenderTargetMask(0xF);

	//Gnm::DbRenderControl dbRenderControl;
	dbRenderControl.init();
	gfxc.setDbRenderControl(dbRenderControl);

	gfxc.popMarker();
}

void Framework::Application::clearMemoryToUints(sce::Gnmx::GnmxGfxContext &gfxc, void *destination, uint32_t destUints, uint32_t *source, uint32_t srcUints)
{
	const bool srcUintsIsPowerOfTwo = (srcUints & (srcUints - 1)) == 0;

	gfxc.setCsShader(srcUintsIsPowerOfTwo ? set_uint_fast->m_shader : set_uint->m_shader,
		srcUintsIsPowerOfTwo ? &set_uint_fast->m_offsetsTable : &set_uint->m_offsetsTable);

	Gnm::Buffer destinationBuffer;
	destinationBuffer.initAsDataBuffer(destination, Gnm::kDataFormatR32Uint, destUints);
	destinationBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);
	gfxc.setRwBuffers(Gnm::kShaderStageCs, 0, 1, &destinationBuffer);

	Gnm::Buffer sourceBuffer;
	sourceBuffer.initAsDataBuffer(source, Gnm::kDataFormatR32Uint, srcUints);
	sourceBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO);
	gfxc.setBuffers(Gnm::kShaderStageCs, 0, 1, &sourceBuffer);

	struct Constants
	{
		uint32_t m_destUints;
		uint32_t m_srcUints;
	};
	Constants *constants = (Constants*)gfxc.allocateFromCommandBuffer(sizeof(Constants), Gnm::kEmbeddedDataAlignment4);
	constants->m_destUints = destUints;
	constants->m_srcUints = srcUints - (srcUintsIsPowerOfTwo ? 1 : 0);
	Gnm::Buffer constantBuffer;
	constantBuffer.initAsConstantBuffer(constants, sizeof(*constants));
	gfxc.setConstantBuffers(Gnm::kShaderStageCs, 0, 1, &constantBuffer);

	gfxc.dispatch((destUints + Gnm::kThreadsPerWavefront - 1) / Gnm::kThreadsPerWavefront, 1, 1);

	synchronizeComputeToGraphics(&gfxc.m_dcb);
}



void Framework::Application::AppendEndFramePackets(sce::Gnmx::GnmxGfxContext& gfxc)
{
	const unsigned indexOfBufferCpuIsWriting = m_indexOfBufferCpuIsWriting;
	m_buffer[indexOfBufferCpuIsWriting].m_expectedLabel = m_frameCount;
	gfxc.writeImmediateAtEndOfPipeWithInterrupt(Gnm::kEopFlushCbDbCaches, const_cast<uint64_t*>(m_buffer[indexOfBufferCpuIsWriting].m_label), m_buffer[indexOfBufferCpuIsWriting].m_expectedLabel, Gnm::kCacheActionNone);
}

void Framework::Application::SubmitCommandBufferOnly(sce::Gnmx::GnmxGfxContext& gfxc)
{
	/*
	if (0 == m_frameCount)
	{
		if (m_config.m_dumpPackets)
		{
			// Dump the PM4 packet for the first frame:
			char fileName[kMaxPath];
			strncpy(fileName, m_szExePath, kMaxPath);
			strncat(fileName, "dlog.pm4", kMaxPath - strlen(fileName) - 1);
			FILE *dlog = fopen(fileName, "w");
			if (dlog)
			{
				if (gfxc.m_submissionCount > 0) {
					Gnm::Pm4Dump::SubmitRange aSubmit[Gnmx::BaseGfxContext::kMaxNumStoredSubmissions + 1];
					for (unsigned iSubmit = 0; iSubmit < gfxc.m_submissionCount; ++iSubmit) {
						aSubmit[iSubmit].m_startDwordOffset = gfxc.m_submissionRanges[iSubmit].m_dcbStartDwordOffset;
						aSubmit[iSubmit].m_sizeInDwords = gfxc.m_submissionRanges[iSubmit].m_dcbSizeInDwords;
					}
					aSubmit[gfxc.m_submissionCount].m_startDwordOffset = gfxc.m_currentDcbSubmissionStart - gfxc.m_dcb.m_beginptr;
					aSubmit[gfxc.m_submissionCount].m_sizeInDwords = gfxc.m_dcb.m_cmdptr - gfxc.m_currentDcbSubmissionStart;
					Gnm::Pm4Dump::dumpPm4PacketStream(dlog, gfxc.m_dcb.m_beginptr, gfxc.m_submissionCount + 1, aSubmit);
				}
				else
					Gnm::Pm4Dump::dumpPm4PacketStream(dlog, &gfxc.m_dcb);
				fclose(dlog);
			}

			if (gfxc.m_acb.m_cmdptr != gfxc.m_acb.m_beginptr)
			{
				strncpy(fileName, m_szExePath, kMaxPath);
				strncat(fileName, "alog.pm4", kMaxPath - strlen(fileName) - 1);
				FILE *alog = fopen(fileName, "w");
				if (alog)
				{
					if (gfxc.m_submissionCount > 0) {
						Gnm::Pm4Dump::SubmitRange aSubmit[Gnmx::BaseGfxContext::kMaxNumStoredSubmissions + 1];
						for (unsigned iSubmit = 0; iSubmit < gfxc.m_submissionCount; ++iSubmit) {
							aSubmit[iSubmit].m_startDwordOffset = gfxc.m_submissionRanges[iSubmit].m_acbStartDwordOffset;
							aSubmit[iSubmit].m_sizeInDwords = gfxc.m_submissionRanges[iSubmit].m_acbSizeInDwords;
						}
						aSubmit[gfxc.m_submissionCount].m_startDwordOffset = gfxc.m_currentAcbSubmissionStart - gfxc.m_acb.m_beginptr;
						aSubmit[gfxc.m_submissionCount].m_sizeInDwords = gfxc.m_acb.m_cmdptr - gfxc.m_currentAcbSubmissionStart;
						Gnm::Pm4Dump::dumpPm4PacketStream(alog, gfxc.m_acb.m_beginptr, gfxc.m_submissionCount + 1, aSubmit);
					}
					else
						Gnm::Pm4Dump::dumpPm4PacketStream(alog, &gfxc.m_acb);
					fclose(alog);
				}
			}

			strncpy(fileName, m_szExePath, kMaxPath);
			strncat(fileName, "clog.pm4", kMaxPath - strlen(fileName) - 1);
			FILE *clog = fopen(fileName, "w");
			if (clog)
			{
				if (gfxc.m_submissionCount > 0) {
					Gnm::Pm4Dump::SubmitRange aSubmit[Gnmx::BaseGfxContext::kMaxNumStoredSubmissions + 1];
					for (unsigned iSubmit = 0; iSubmit < gfxc.m_submissionCount; ++iSubmit) {
						aSubmit[iSubmit].m_startDwordOffset = gfxc.m_submissionRanges[iSubmit].m_ccbStartDwordOffset;
						aSubmit[iSubmit].m_sizeInDwords = gfxc.m_submissionRanges[iSubmit].m_ccbSizeInDwords;
					}
					aSubmit[gfxc.m_submissionCount].m_startDwordOffset = gfxc.m_currentCcbSubmissionStart - gfxc.m_ccb.m_beginptr;
					aSubmit[gfxc.m_submissionCount].m_sizeInDwords = gfxc.m_ccb.m_cmdptr - gfxc.m_currentCcbSubmissionStart;
					Gnm::Pm4Dump::dumpPm4PacketStream(clog, gfxc.m_ccb.m_beginptr, gfxc.m_submissionCount + 1, aSubmit);
				}
				else
					Gnm::Pm4Dump::dumpPm4PacketStream(clog, &gfxc.m_ccb);
				fclose(clog);
			}
		}
	}
	else
	{
		m_secondsElapsedActualPerFrame = m_secondsElapsedActual / m_frameCount;
		m_secondsElapsedApparentPerFrame = m_paused ? 0 : m_secondsElapsedActualPerFrame;
	}
	*/
	//const unsigned indexOfBufferCpuIsWriting = getIndexOfBufferCpuIsWriting();

	//
	// Submit the command buffer:
	//
	int32_t state;
	//if (kGpuEop == m_config.m_whoQueuesFlips)
	state = gfxc.submitAndFlip(m_videoInfo.handle, m_videoInfo.flip_index_base + m_indexOfBufferCpuIsWriting, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0, (void *)m_buffer[m_indexOfBufferCpuIsWriting].m_labelForPrepareFlip, m_buffer[m_indexOfBufferCpuIsWriting].m_expectedLabel);
// 	else
// 		state = gfxc.submit();

	SCE_GNM_ASSERT_MSG(state == sce::Gnm::kSubmissionSuccess, "Command buffer validation error.");
	Gnm::submitDone();
}

void Framework::Application::SubmitCommandBuffer(sce::Gnmx::GnmxGfxContext& gfxc)
{
	AppendEndFramePackets(gfxc);
	SubmitCommandBufferOnly(gfxc);
}

void Framework::Application::WaitForBufferAndFlip()
{
	++m_frameCount;
// 	if (m_frameCount >= m_config.m_frames)
// 		RequestTermination();
// 	if (m_secondsElapsedActual >= m_config.m_seconds)
// 		RequestTermination();


	bool m_asynchronous = true;
	if (!m_asynchronous)
	{
		StallUntilGPUIsNotUsingBuffer(&m_eopEventQueue, m_indexOfBufferCpuIsWriting);
// 		if (kCpuMainThread == m_config.m_whoQueuesFlips)
// 		{
// 			const Result ret = sceVideoOutSubmitFlip(m_videoInfo.handle, m_videoInfo.flip_index_base + getIndexOfBufferCpuIsWriting(), m_config.m_flipMode, 0);
// 			SCE_GNM_ASSERT(ret >= 0);
// 		}
		advanceCpuToNextBuffer();
		StallUntilGPUIsNotUsingBuffer(&m_eopEventQueue, m_indexOfBufferCpuIsWriting);
	}
	else
	{
		advanceCpuToNextBuffer();
		StallUntilGPUIsNotUsingBuffer(&m_eopEventQueue, m_indexOfBufferCpuIsWriting);
// 		if ((kCpuMainThread == m_config.m_whoQueuesFlips) && (m_frameCount >= m_config.m_buffers))
// 		{
// 			const Result ret = sceVideoOutSubmitFlip(m_videoInfo.handle, m_videoInfo.flip_index_base + getIndexOfBufferCpuIsWriting(), m_config.m_flipMode, 0);
// 			SCE_GNM_ASSERT(ret >= 0);
// 		}
	}
}

void Framework::Application::StallUntilGpuIsIdle()
{

}

void Framework::Application::StallUntilGPUIsNotUsingBuffer(EopEventQueue *eopEventQueue, uint32_t bufferIndex)
{

}

void Framework::Application::advanceCpuToNextBuffer()
{
	m_indexOfLastBufferCpuWrote = m_indexOfBufferCpuIsWriting;
	m_indexOfBufferCpuIsWriting = (m_indexOfBufferCpuIsWriting + 1) % 3;

	m_backBufferIndex = m_indexOfBufferCpuIsWriting;
	m_backBuffer = m_buffer + m_backBufferIndex;
}

void Framework::Application::synchronizeComputeToGraphics(sce::Gnmx::GnmxDrawCommandBuffer *dcb)
{
	volatile uint64_t* label = (volatile uint64_t*)dcb->allocateFromCommandBuffer(sizeof(uint64_t), Gnm::kEmbeddedDataAlignment8); // allocate memory from the command buffer
	*label = 0x0; // set the memory to have the val 0
	dcb->writeAtEndOfShader(Gnm::kEosCsDone, const_cast<uint64_t*>(label), 0x1); // tell the CP to write a 1 into the memory only when all compute shaders have finished
	dcb->waitOnAddress(const_cast<uint64_t*>(label), 0xffffffff, Gnm::kWaitCompareFuncEqual, 0x1); // tell the CP to wait until the memory has the val 1
	dcb->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackAndInvalidateL1andL2, 0, Gnm::kStallCommandBufferParserDisable); // tell the CP to flush the L1$ and L2$
}

void Framework::Application::SetViewToWorldMatrix(const Matrix4& m)
{
	m_viewToWorldMatrix = m;
	UpdateMatrices();
}

void Framework::Application::UpdateMatrices()
{
	m_worldToViewMatrix = inverse(m_viewToWorldMatrix);
	m_worldToLightMatrix = inverse(m_lightToWorldMatrix);
	m_viewProjectionMatrix = m_projectionMatrix * m_worldToViewMatrix;
}

Framework::Application::EopEventQueue::EopEventQueue(const char *name)
{
	Result ret = 0;
	ret = sceKernelCreateEqueue(&m_equeue, name);
	SCE_GNM_ASSERT_MSG(ret >= 0, "sceKernelCreateEqueue returned error code %d.", ret);
	m_name = name;
	ret = sce::Gnm::addEqEvent(m_equeue, sce::Gnm::kEqEventGfxEop, NULL);
	SCE_GNM_ASSERT_MSG(ret == 0, "sce::Gnm::addEqEvent returned error code %d.", ret);
}

Framework::Application::EopEventQueue::~EopEventQueue()
{
	sceKernelDeleteEqueue(m_equeue);
}

void Framework::Application::EopEventQueue::waitForEvent()
{
	SceKernelEvent ev;
	int num;
	Result error = sceKernelWaitEqueue(m_equeue, &ev, 1, &num, nullptr);
	SCE_GNM_ASSERT(error == SCE_OK);
}

void Framework::EmbeddedCsShader::initializeWithAllocators(Allocators *allocators)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary;
	void *shaderHeader;
	allocators->allocate(&shaderBinary, SCE_KERNEL_WC_GARLIC, shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes, Gnm::kResourceTypeShaderBaseAddress, &mCsShaderHandle, m_name);
	allocators->allocate(&shaderHeader, SCE_KERNEL_WB_ONION, shaderInfo.m_csShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);


	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_csShader, shaderInfo.m_csShader->computeSize());

	m_shader = static_cast<Gnmx::CsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);

	Gnmx::generateInputOffsetsCache(&m_offsetsTable, Gnm::kShaderStageCs, m_shader);
}
