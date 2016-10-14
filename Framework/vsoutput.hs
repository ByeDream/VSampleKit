/* SCE CONFIDENTIAL
PlayStation(R)4 Programmer Tool Runtime Library Release 03.508.201
* Copyright (C) 2013 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef __VSOUTPUT_H__
#define __VSOUTPUT_H__

struct VS_OUTPUT
{
    float4 Position     : S_POSITION;
	float2 TextureUV    : TEXCOORD0;
	float3 vNorm		: TEXCOORD1;
	float4 vTang		: TEXCOORD3;
	float3 vPosInView	: TEXCOORD5;
	float4 Color		: COLOR;
};



#endif