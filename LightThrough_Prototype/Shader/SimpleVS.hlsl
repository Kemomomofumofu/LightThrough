#include "common.hlsl"

PS_IN vs_main(VS_IN input)
{
    PS_IN output;
    
    // à íuÇ∆êFÇÃÇ›
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    output.pos = mul(input.pos, wvp);
    output.col = input.col;
      
	return output;
}