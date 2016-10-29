#pragma once

#include "Texture.h"

namespace Framework
{
	class BaseTargetView;
	class Allocators;
	class RenderableTextureColor;
	class RenderableTextureDepthStencil;

	class RenderableTexture : public Texture
	{
		using super = Texture;

	public:
		RenderableTexture() {}
		virtual ~RenderableTexture() {}

		virtual void						init(const Texture::Description &desc, Allocators *allocators);
		virtual void						deinit(Allocators *allocators);
		inline BaseTargetView *				getTargetView() const { return mTargetView; }

		static RenderableTexture *			CreateRenderableTextureColor(const Texture::Description &desc, Allocators *allocators, bool isSwapChain = false);
		static RenderableTexture *			CreateRenderableTextureDepthStencil(const Texture::Description &desc, Allocators *allocators, bool isUsingHTile = true, bool isUsingStencil = true);
	
	protected:
		virtual void						createTargetView() = 0;
		virtual void						allocMemory(Allocators *allocators) = 0;

	protected:
		BaseTargetView *					mTargetView{ nullptr };
	};


	class RenderableTextureColor : public RenderableTexture
	{
		using super = RenderableTexture;

	public:
		RenderableTextureColor(bool isSwapChain, bool isUsingCMask, bool isUsingFMask);
		virtual ~RenderableTextureColor();

		virtual void						deinit(Allocators *allocators);

		inline bool							isSwapChain() const { return mIsSwapChain; }
		inline bool							isUsingCMask() const { return mIsUsingCMask; }
		inline bool							isUsingFMask() const { return mIsUsingFMask; }

	protected:
		virtual void						createTargetView();
		virtual void						allocMemory(Allocators *allocators);

	protected:
		bool								mIsSwapChain{ false };
		bool								mIsUsingCMask{ false };
		bool								mIsUsingFMask{ false };
		void *								mCMaskGpuMemAddr{ nullptr };
		void *								mFMaskGpuMemAddr{ nullptr };
		sce::Gnm::ResourceHandle			mCMaskHandle{ sce::Gnm::kInvalidResourceHandle };
		sce::Gnm::ResourceHandle			mFMaskHandle{ sce::Gnm::kInvalidResourceHandle };
	};

	class RenderableTextureDepthStencil : public RenderableTexture
	{
		using super = RenderableTexture;

	public:
		RenderableTextureDepthStencil(bool isUsingHTile, bool isUsingStencil);
		virtual ~RenderableTextureDepthStencil();

		virtual void						deinit(Allocators *allocators);

		inline bool							isUsingHTile() const { return mIsUsingHTile; }
		inline bool							isUsingStencil() const { return mIsUsingStencil; }

	protected:
		virtual void						createTargetView();
		virtual void						allocMemory(Allocators *allocators);

	protected:
		bool								mIsUsingHTile{ true };
		bool								mIsUsingStencil{ true };
		void *								mHTileGpuMemAddr{ nullptr };
		void *								mStencilGpuMemAddr{ nullptr };
		sce::Gnm::ResourceHandle			mHTileHandle{ sce::Gnm::kInvalidResourceHandle };
		sce::Gnm::ResourceHandle			mStencilHandle{ sce::Gnm::kInvalidResourceHandle };
	};
}