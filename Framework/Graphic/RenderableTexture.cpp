#include "stdafx.h"

#include "RenderableTexture.h"

Framework::RenderableTexture::RenderableTexture()
{

}

Framework::RenderableTexture::~RenderableTexture()
{

}

Framework::RenderableTexture *Framework::RenderableTexture::CreateRenderableTexture(const Description &desc)
{
	RenderableTexture *_texture = new RenderableTexture;
	_texture->init(desc);
	return _texture;
}

void Framework::RenderableTexture::init(const Description &desc)
{

}
