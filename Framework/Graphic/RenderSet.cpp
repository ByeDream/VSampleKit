#include "stdafx.h"

#include "RenderSet.h"
#include "ChunkBasedRenderContext/RenderContext.h"
#include "GPUResource/Texture.h"
#include "GPUResource/GPUResourceManager.h"

Framework::RenderSet::RenderSet()
{

}

Framework::RenderSet::~RenderSet()
{

}

void Framework::RenderSet::init(RenderSurface *depth, RenderSurface *color0, RenderSurface *color1 /*= nullptr*/, RenderSurface *color2 /*= nullptr*/, RenderSurface *color3 /*= nullptr*/)
{
	setDepthSurface(depth);
	setColorSurface(0, color0);
	setColorSurface(1, color1);
	setColorSurface(2, color2);
	setColorSurface(3, color3);
}

void Framework::RenderSet::setColorSurface(U32 slotID, RenderSurface *surface)
{
	SCE_GNM_ASSERT(slotID >= 0 && slotID < MAX_NUM_COLOR_SURFACE);
	if (mColorSurfaces[slotID] == nullptr && surface != nullptr)
	{
		mNumColorSurfaces++;
	}
	else if(mColorSurfaces[slotID] != nullptr && surface == nullptr)
	{
		//TODO free previse surface.
		mNumColorSurfaces--;
	}
	mColorSurfaces[slotID] = surface;
	mColorHandle[slotID] = (surface != nullptr) ? surface->getHandle() : RenderSurface::RENDER_SURFACE_HANDLE_INVALID;
}

void Framework::RenderSet::setColorSurface(U32 slotID, RenderSurface::Handle surfaceHandle)
{
	RenderSurface *_surface = GPUResourceManager::getInstance()->getSurface(surfaceHandle);
	SCE_GNM_ASSERT(_surface != nullptr);
	setColorSurface(slotID, _surface);
}

void Framework::RenderSet::setDepthSurface(RenderSurface *surface)
{
	mDepthSurface = surface;
	mDepthHandle = (surface != nullptr) ? surface->getHandle() : RenderSurface::RENDER_SURFACE_HANDLE_INVALID;
}

void Framework::RenderSet::setDepthSurface(RenderSurface::Handle surfaceHandle)
{
	RenderSurface *_surface = GPUResourceManager::getInstance()->getSurface(surfaceHandle);
	SCE_GNM_ASSERT(_surface != nullptr);
	setDepthSurface(_surface);
}

void Framework::RenderSet::bindToPipeline(RenderContext *context) const
{
	// Ge The reference surface to use to set back the Viewport
	RenderSurface* _referenceSurface = nullptr;

	// render target
	for (auto i = 0; i < MAX_NUM_COLOR_SURFACE; i++)
	{
		if (mColorSurfaces[i] != nullptr)
		{
			mColorSurfaces[i]->bindAsRenderTarget(context, i);
			_referenceSurface = (_referenceSurface == nullptr) ? mColorSurfaces[i] : _referenceSurface;
		}
	}

	// depth target
	if (mDepthSurface != nullptr)
	{
		mDepthSurface->bindAsDepthStencilTarget(context);
		_referenceSurface = (_referenceSurface == nullptr) ? mDepthSurface : _referenceSurface;
	}

	// Need to set the Viewport as it has been reset when the render target is set
	context->setViewport(0, 0, _referenceSurface->getTexture()->getDescription().mWidth, _referenceSurface->getTexture()->getDescription().mHeight);
}

// TODO
// Get the Viewport after set the target
// const PlatformGfxViewport& vp = t_gfxDevice->GetViewport();
// 
// Get the Correct Reference  Surface
// GfxexSurface* referenceSurface = GetReferenceSurface(t_graphicDevice);
// 
// adjust the viewport scale offset
// ubiVector4  viewportScaleOffset(0.5f * vp.Width / referenceSurface->GetWidth(),
// 	-0.5f * vp.Height / referenceSurface->GetHeight(),
// 	((0.5f * vp.Width) + vp.TopLeftX) / referenceSurface->GetWidth(),
// 	((0.5f * vp.Height) + vp.TopLeftY) / referenceSurface->GetHeight());
// 
// viewportScaleOffset(2) += (0.5f / referenceSurface->GetWidth());
// viewportScaleOffset(3) += (0.5f / referenceSurface->GetHeight());
// 
// a_context->SetViewportScaleOffset(viewportScaleOffset);
// 
// ubiVector2 uvHalfOffsetForSurface(0.5f / referenceSurface->GetWidth(), 0.5f / referenceSurface->GetHeight());
// 
// 
// ubiVector4 vposToUVScaleOffset(1.0f / (ubiFloat)referenceSurface->m_Width,
//                                1.0f / (ubiFloat)referenceSurface->m_Height,
//                                uvHalfOffsetForSurface(0) + ((ubiFloat)vp.X / (ubiFloat)referenceSurface->m_Width),
//                                uvHalfOffsetForSurface(1) + ((ubiFloat)vp.Y / (ubiFloat)referenceSurface->m_Height));
// 
// View* currentView = NULL;
// if ((t_viewSurface != NULL) && (t_viewSurface->GetViews().Size() > 0))
// {
// 	currentView = t_viewSurface->GetViews()[0];
// }
// 
// Rect effectiveViewport(0, 0, 0, 0);
// ubiVector4 vposToUVScaleOffset = ubiVector4::GetZero();
// if (currentView)
// {
// 	currentView->UpdateInternalValues();
// 	effectiveViewport = currentView->GetEffectiveViewport();
// 
// 	vposToUVScaleOffset = ubiVector4(1.0f / (ubiFloat)referenceSurface->GetWidth(),
// 		1.0f / (ubiFloat)referenceSurface->GetHeight(),
// 		(ubiFloat)referenceSurface->GetWidth() / effectiveViewport.GetWidth(),
// 		(ubiFloat)referenceSurface->GetHeight() / effectiveViewport.GetHeight());
// }
// 
// a_context->SetUVHalfOffsetForSurface(uvHalfOffsetForSurface);
// a_context->SetVPosToUVScaleOffset(vposToUVScaleOffset);