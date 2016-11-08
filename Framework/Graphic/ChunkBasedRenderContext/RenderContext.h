#pragma once

#include<vector>

namespace Framework
{
	class GraphicDevice;
	class RenderSurface;
	struct CommandList;

	class VertexShaderView;
	class PixelShaderView;
	class ComputeShaderView;

	class RenderContextChunk;

	class RenderContext
	{
		friend class RenderContextChunk;
		friend class ImmediateRenderContextChunk;

	public:
		RenderContext(GraphicDevice *device);
		virtual ~RenderContext();

		virtual void roll();
		virtual void reset();
		virtual void submitAndFlip(bool asynchronous);
		virtual void appendLabelAtEOPWithInterrupt(void *dstGpuAddr, U64 value);	// writes value and and triggers an interrupt
		virtual void appendLabelAtEOP(void *dstGpuAddr, U64 value);					// writes value only

		inline void								setVertexShader(const VertexShaderView *shader) {}
		inline void								setPixelShader(const PixelShaderView *shader) {}
		inline void								setComputeShader(const ComputeShaderView *shader) {}

		void setTextureSurface(U32 soltID, const RenderSurface *surface);
		void setRenderTargetSurface(U32 soltID, const RenderSurface *surface);
		void setDepthStencilTargetSurface(const RenderSurface *surface);

		void setViewport(U32 x, U32 y, U32 width, U32 height, Float32 minz = 0.0f, Float32 maxz = 1.0f);

		inline void updateCmdPtr(uint32_t *a_dcbPtr, uint32_t *a_ccbPtr)
		{
// 			popAssert(a_dcbPtr != nullptr);
// 			popAssert(a_ccbPtr != nullptr);
// 			popAssert(a_dcbPtr >= m_DcbBeginPtr[m_CurrentDcbMemoryIndex] && a_dcbPtr <= m_DcbEndPtr[m_CurrentDcbMemoryIndex]);
// 			popAssert(a_ccbPtr >= m_CcbBeginPtr[m_CurrentCcbMemoryIndex] && a_ccbPtr <= m_CcbEndPtr[m_CurrentCcbMemoryIndex]);
// 			m_DcbCmdPtr = a_dcbPtr;
// 			m_CcbCmdPtr = a_ccbPtr;
		}

		U32 *getCurrentDcbCmdPtr(U32 allocationPlan);
		U32 *getCurrentCcbCmdPtr(U32 allocationPlan);

		// Chunk management
		RenderContextChunk *				getCurrentChunk();
		const RenderContextChunk *			getCurrentChunk() const;
	protected:
		GraphicDevice *						mDevice{ nullptr };
		bool								mIsDeferred{ false };
		std::vector<CommandList *>			mPendingCommandLists;
	};
}
