#pragma once

namespace Framework
{
#	define	SAFE_DELETE(p)			if(p != nullptr) { delete(p); p = nullptr; }
#	define	SAFE_DELETE_ARRAY(p)	if(p != nullptr) { delete[](p); p = nullptr; }

	typedef __int8_t				S8;         //< Minimum value for a signed 8 bits integer
	typedef __uint8_t				U8;         //< 8 bits unsigned integer
	typedef __int16_t				S16;        //< 16 bits signed integer
	typedef __uint16_t				U16;        //< 16 bits unsigned integer
	typedef __int32_t				S32;        //< 32 bits signed integer
	typedef __uint32_t				U32;        //< 32 bits unsigned integer
	typedef __int64_t				S64;        //< 64 bits signed integer
	typedef __uint64_t				U64;        //< 64 bits unsigned integer
	typedef float					Float32;    //< 32 bits float
	typedef double					Float64;    //< 64 bits double float
	typedef unsigned long long		Word;       //< Machine word (the size of the data bus)
	typedef long long				SWord;      //< Machine signed word (the size of the data bus)
	typedef U64						Address;    //< Address unsigned word (the size of the ADRESS bus)
	typedef wchar_t					WChar;      //< Wide-character type
	typedef U64						TID;        //< Thread ID
	typedef S32						Result;		//< Functions return

#	define	MAX_VALUE_8				0xFF
#	define	MAX_VALUE_16			0xFFFF
#	define	MAX_VALUE_32			0xFFFFFFFF
#	define	MAX_VALUE_64			0xFFFFFFFFFFFFFFFF 


	typedef struct __Rect
	{
		S64	left;
		S64 top;
		S64 right;
		S64 bottom;
	} Rect, *PRect;

	enum AntiAliasingType
	{
		AA_NONE = 0,
		AA_EQAA_2X,
		AA_EQAA_4X,

		AA_TYPE_COUNT
	};

	enum TextureType
	{
		TEX_1D = 0,
		TEX_2D,
		TEX_3D,
		TEX_CUBE,

		TEX_TYPE_COUNT
	};

	enum ShaderType
	{
		SHADER_VERTEX = 0,
		SHADER_PIXEL,
		SHADER_COMPUTE,
		SHADER_HULL,
		SHADER_DOMAIN,
		SHADER_GEOMETRY,

		SHADER_TYPE_COUNT
	};

	enum SampleStateType
	{
		// Order must match bitfield with Framework::SamplingState

		// per texture settings
		SAMP_STATE_ADDRESSU = 0,
		SAMP_STATE_ADDRESSV,
		SAMP_STATE_ADDRESSW,
		SAMP_STATE_BORDERCOLOR,
		SAMP_STATE_MAGFILTER,
		SAMP_STATE_MINFILTER,
		SAMP_STATE_MIPFILTER,
		SAMP_STATE_SRGBTEXTURE,

		// global settings
		SAMP_STATE_ANISOFILTER,
		SAMP_STATE_MAXANISOTROPY,
		SAMP_STATE_MIPMAPLODBIAS,

		SAMP_STATE_TYPE_COUNT,
	};
	typedef U32 SampleStateValue;

// 	enum RESOURCE_BIND_FLAG
// 	{
// 		BIND_VERTEX_BUFFER = 0x1L,
// 		BIND_INDEX_BUFFER = 0x2L,
// 		BIND_CONSTANT_BUFFER = 0x4L,
// 		BIND_SHADER_RESOURCE = 0x8L,
// 		BIND_STREAM_OUTPUT = 0x10L,
// 		BIND_RENDER_TARGET = 0x20L,
// 		BIND_DEPTH_STENCIL = 0x40L,
// 		BIND_UNORDERED_ACCESS = 0x80L,
// 		BIND_DECODER = 0x200L,
// 		BIND_VIDEO_ENCODER = 0x400L,
// 	};
}

