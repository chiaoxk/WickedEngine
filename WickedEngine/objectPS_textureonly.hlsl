#include "objectHF.hlsli"



float4 main(PixelInputType_Simple input) : SV_TARGET
{
	OBJECT_PS_DITHER

	OBJECT_PS_MAKE_SIMPLE

	OBJECT_PS_SAMPLETEXTURES_SIMPLE

	OBJECT_PS_DEGAMMA

	OBJECT_PS_EMISSIVE

	OBJECT_PS_OUT_FORWARD
}

