#pragma once

#include "DesignPattern/Factory.h"
#include "DesignPattern/Singleton.h"

#include "Utils/IO/FileIO.h"
#include "Utils/IO/tga_reader.h"

#include "Utils/Geommath/geommath.h"
#include "Utils/simple_mesh.h"

#define UTIL_KB(Nb)                     ((Nb)*1024U)            ///< Defines for KiloBytes prefix (2 ^ 10)
#define UTIL_MB(Nb)                     UTIL_KB((Nb)*1024U)     ///< Defines for MegaBytes prefix (2 ^ 20)
#define UTIL_GB(Nb)                     UTIL_MB((Nb)*1024ULL)   ///< Defines for GigaBytes prefix (2 ^ 30)
#define UTIL_TB(Nb)                     UTIL_GB((Nb)*1024ULL)   ///< Defines for TeraBytes prefix (2 ^ 40)
#define UTIL_PB(Nb)                     UTIL_TB((Nb)*1024ULL)   ///< Defines for PetaBytes prefix (2 ^ 50)

namespace Framework
{
	template<typename Src, typename Dest>
	inline Dest *typeCast(Src *type)
	{
		Dest *_type = dynamic_cast<Dest *>(type);
		SCE_GNM_ASSERT_MSG(_type != nullptr, "Invalid type casting");
		return _type;
	}

	template<typename Src, typename Dest>
	inline const Dest *typeCast(const Src *type)
	{
		const Dest *_type = dynamic_cast<const Dest *>(type);
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

	struct Bitset
	{
		U64 mBits{ 0 };

		Bitset() {}
		Bitset(U64 v) : mBits(v) {}

		inline bool get(U64 bit) const { return ((mBits & bit) == bit); }
		inline void set(U64 bit) { mBits |= bit; }
		inline void unset(U64 bit) { mBits &= ~bit; }
		inline void fullset() { mBits = MAX_VALUE_64; }
		inline void clear() { mBits = 0; }
	};
}