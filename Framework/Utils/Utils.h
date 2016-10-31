#pragma once

#include "Utils/IO/FileIO.h"
#include "Utils/IO/tga_reader.h"

#include "Utils/Geommath/geommath.h"
#include "Utils/simple_mesh.h"

namespace Framework
{
	template<typename Source, typename Dest>
	Dest *typeCast(Source *type)
	{
		Dest *_type = dynamic_cast<Dest *>(type);
		SCE_GNM_ASSERT_MSG(_type != nullptr, "Invalid type casting");
		return _type;
	}
}