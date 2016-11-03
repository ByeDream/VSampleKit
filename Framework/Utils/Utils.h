#pragma once

#include "DesignPattern/Factory.h"
#include "DesignPattern/Singleton.h"

#include "Utils/IO/FileIO.h"
#include "Utils/IO/tga_reader.h"

#include "Utils/Geommath/geommath.h"
#include "Utils/simple_mesh.h"

namespace Framework
{
	template<typename Src, typename Dest>
	inline Dest *typeCast(Src *type)
	{
		Dest *_type = dynamic_cast<Dest *>(type);
		SCE_GNM_ASSERT_MSG(_type != nullptr, "Invalid type casting");
		return _type;
	}

	template<typename T>
	inline T max(T a, T b)
	{
		return (a > b) ? a : b;
	}

	template<typename T>
	inline T min(T a, T b)
	{
		return (a < b) ? a : b;
	}
}