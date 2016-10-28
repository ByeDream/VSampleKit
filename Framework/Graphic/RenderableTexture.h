#pragma once

#include "Texture.h"

namespace Framework
{
	class RenderableTexture : public Texture
	{
		using super = Texture;

	public:
		RenderableTexture();
		virtual ~RenderableTexture();

		struct Description
		{
			U32 mWidth{ 0 };
			U32 mHeight{ 0 };
			sce::Gnm::DataFormat mFormat{ sce::Gnm::kDataFormatInvalid };
			AntiAliasingType mAAType;
		};

		static RenderableTexture *CreateRenderableTexture(const Description &desc);

		void init(const Description &desc);
	};
}