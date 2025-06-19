#include "common.hlsl"

float4 ps_main(PS_IN input) : SV_TARGET
{
	return input.col;
}