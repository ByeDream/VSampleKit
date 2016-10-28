#pragma once

#include "ConfigData.h"

struct Constants;


namespace Framework
{
	
	
	//------------------------------------

	struct EmbeddedVsShader
	{
		const uint32_t *m_source;
		const char *m_name;
		sce::Gnmx::VsShader *m_shader;
		void *m_fetchShader;
		sce::Gnmx::InputOffsetsCache m_offsetsTable;

		uint32_t m_fetchShaderSize;

		sce::Gnm::ResourceHandle mVsShaderHandle;
		sce::Gnm::ResourceHandle mFetchShaderHandle;

		void initializeWithAllocators(Allocators *allocators);
	};

	struct EmbeddedPsShader
	{
		const uint32_t *m_source;
		const char *m_name;
		sce::Gnmx::PsShader *m_shader;
		sce::Gnmx::InputOffsetsCache m_offsetsTable;

		sce::Gnm::ResourceHandle mPsShaderHandle;

		void initializeWithAllocators(Allocators *allocators);
	};

	struct EmbeddedCsShader
	{
		const uint32_t *m_source;
		const char *m_name;
		sce::Gnmx::CsShader *m_shader;
		sce::Gnmx::InputOffsetsCache m_offsetsTable;

		sce::Gnm::ResourceHandle mCsShaderHandle;

		void initializeWithAllocators(Allocators *allocators);
	};

	struct Buffer
	{
		sce::Gnm::RenderTarget       m_renderTarget;
		sce::Gnm::DepthRenderTarget  m_depthTarget;
		uint8_t						 m_reserved0[4];
		//sce::DbgFont::Screen         m_screen;
		//sce::DbgFont::Window         m_window;
		volatile uint64_t           *m_label;
		volatile uint64_t           *m_labelForPrepareFlip;
		uint32_t                     m_expectedLabel;
		uint32_t                     m_reserved;
		//sce::Gnmx::Toolkit::Timers   m_timers;
		//DebugObjects                 m_debugObjects;
	};

	class Frame
	{
	public:
		sce::Gnmx::GnmxGfxContext m_commandBuffer;
		Constants *m_constants[2];
		sce::Gnm::Buffer m_constantBuffer[2];
	};
	//------------------------------------
	class GraphicDevice;
	struct ConfigData;

	class Application
	{
	public:
		Application();
		~Application();

		bool initialize(const char *name, int argc, const char* argv[]);
		bool frame();
		bool terminate();

		inline const char *getTitleName() const { return mName; }
		inline const ConfigData *getConfig() const { return &mConfig; }

	private:
		void processCommandLine(int argc, const char* argv[]);

	private:
		const char *						mName{ nullptr };
		ConfigData							mConfig;

		GraphicDevice *						mGraphicDevice{ nullptr };

		Allocators *mAllocators{ nullptr };
		StackAllocator *mGarlicStackAllocator{ nullptr };
		StackAllocator *mOnionStackAllocator{ nullptr };

		

		/////////////////////////////////////

		struct VideoInfo
		{
			int32_t handle;
			uint32_t flip_index_base;
			uint32_t buffer_num;
		};

		EmbeddedVsShader *LoadVsShader(const char* filename, Allocators *allocators);
		EmbeddedPsShader *LoadPsShader(const char* filename, Allocators *allocators);
		EmbeddedCsShader *LoadCsShader(const char* filename, Allocators *allocators);
		void AcquireFileContents(void *&pointer, U32 &bytes, const char *filename);
		void ReleaseFileContents(void *pointer);

		volatile uint64_t *m_label;
		volatile uint64_t *m_labelForPrepareFlip;

		Buffer* m_buffer;
		Buffer* m_backBuffer;
		uint32_t m_backBufferIndex;
		uint32_t m_indexOfLastBufferCpuWrote;
		uint32_t m_indexOfBufferCpuIsWriting;

		sce::Gnm::SizeAlign InitRenderTarget(sce::Gnm::RenderTarget *renderTarget, uint32_t width, uint32_t height, uint32_t numSlices, sce::Gnm::DataFormat rtFormat, sce::Gnm::TileMode tileModeHint, sce::Gnm::NumSamples numSamples, sce::Gnm::NumFragments numFragments, sce::Gnm::SizeAlign *cmaskSizeAlign, sce::Gnm::SizeAlign *fmaskSizeAlign);
		sce::Gnm::SizeAlign InitDepthRenderTarget(sce::Gnm::DepthRenderTarget *depthRenderTarget, uint32_t width, uint32_t height, sce::Gnm::ZFormat zFormat, sce::Gnm::StencilFormat stencilFormat, sce::Gnm::TileMode depthTileModeHint, sce::Gnm::NumFragments numFragments, sce::Gnm::SizeAlign *stencilSizeAlign, sce::Gnm::SizeAlign *htileSizeAlign);
	
		Frame m_frames[3];


		/** @brief Uses a compute shader to clear a render target to a solid color.
		* @param gfxc The graphics context to use
		* @param renderTarget The render target to clear
		* @param color The color to clear the render target to
		*/
		void clearRenderTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::RenderTarget* renderTarget, const Vector4& color);
		void clearDepthTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const float depthValue);
		void clearMemoryToUints(sce::Gnmx::GnmxGfxContext &gfxc, void *destination, uint32_t destUints, uint32_t *source, uint32_t srcUints);

		void AppendEndFramePackets(sce::Gnmx::GnmxGfxContext& gfxc);
		void SubmitCommandBufferOnly(sce::Gnmx::GnmxGfxContext& gfxc);
		void SubmitCommandBuffer(sce::Gnmx::GnmxGfxContext& gfxc);
		void WaitForBufferAndFlip();
		void StallUntilGpuIsIdle();
		class EopEventQueue
		{
			SceKernelEqueue m_equeue;
			const char *m_name;
		public:
			EopEventQueue(const char *name);
			~EopEventQueue();

			SCE_GNM_API_DEPRECATED("These APIs are no longer supported.")
				bool queueIsEmpty() const { return true; }
			SCE_GNM_API_DEPRECATED("These APIs are no longer supported.")
				void setMaximumPendingEvents(uint32_t maximumPendingEvents) { SCE_GNM_UNUSED(maximumPendingEvents); }
			SCE_GNM_API_DEPRECATED("These APIs are no longer supported.")
				void addPendingEvent() { }

			void waitForEvent();
		};

		void StallUntilGPUIsNotUsingBuffer(EopEventQueue *eopEventQueue, uint32_t bufferIndex);
		void advanceCpuToNextBuffer();
		void synchronizeComputeToGraphics(sce::Gnmx::GnmxDrawCommandBuffer *dcb);

		uint32_t          m_frameCount{ 0 };

		EopEventQueue m_eopEventQueue;

		// TODO shader manager
		EmbeddedVsShader *vertexShader;
		EmbeddedPsShader *pixelShader;
		EmbeddedCsShader *set_uint;
		EmbeddedCsShader *set_uint_fast;
		EmbeddedPsShader *pix_clear_p;

		SimpleMesh *m_mesh0;
		SimpleMesh *m_mesh1;

		Matrix4 m_worldToLightMatrix;
		Matrix4 m_lightToWorldMatrix;
		Matrix4 m_projectionMatrix;
		Matrix4 m_worldToViewMatrix;
		Matrix4 m_viewToWorldMatrix;
		Matrix4 m_viewProjectionMatrix;
		void SetViewToWorldMatrix(const Matrix4&);
		void UpdateMatrices();
	
		sce::Gnm::Texture texturesEarth[3];
		sce::Gnm::Texture texturesMetal[3];
		sce::Gnm::Sampler trilinearSampler;
	};
}
