#include "stdafx.h"

#include "RenderContext.h"
#include "GPUResourceViews.h"
#include "RenderSurface.h"
#include "RenderableTexture.h"

Framework::RenderContext::RenderContext()
{

}

Framework::RenderContext::~RenderContext()
{

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
