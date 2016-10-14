#pragma once

#define SAFE_DELETE(p) if(p != nullptr) { delete(p); p = nullptr; }
#define SAFE_DELETE_ARRAY(p) if(p != nullptr) { delete[](p); p = nullptr; }

namespace Framework
{
	typedef __int8_t              S8;         //< Minimum value for a signed 8 bits integer
	typedef __uint8_t             U8;         //< 8 bits unsigned integer
	typedef __int16_t             S16;        //< 16 bits signed integer
	typedef __uint16_t            U16;        //< 16 bits unsigned integer
	typedef __int32_t             S32;        //< 32 bits signed integer
	typedef __uint32_t            U32;        //< 32 bits unsigned integer
	typedef __int64_t             S64;        //< 64 bits signed integer
	typedef __uint64_t            U64;        //< 64 bits unsigned integer
	typedef float                 Float32;    //< 32 bits float
	typedef double                Float64;    //< 64 bits double float
	typedef unsigned long long    Word;       //< Machine word (the size of the data bus)
	typedef long long             SWord;      //< Machine signed word (the size of the data bus)
	typedef U64                   Address;    //< Address unsigned word (the size of the ADRESS bus)
	typedef wchar_t               WChar;      //< Wide-character type.
	typedef U64                   TID;        //< Thread ID
}
