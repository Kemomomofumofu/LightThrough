#include "../Common/Lighting.hlsli"


struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float3 worldPos : WORLDPOS;
};

float4 PSMain(PSIN _pin) : SV_Target
{
    float3 N = normalize(_pin.normalWS);
    float3 accum = 0;
    
    // ライト計算
    [loop]
    for (int i = 0; i < lightCount; ++i)
    {
        float li = ComputeLight(lights[i], N, _pin.worldPos);
        accum += li * lights[i].color.rgb;
    }
    
    float3 albedo = _pin.color.rgb;
    float3 diffuse = albedo * accum;
    
    // 二値化
    return float4(accum, 1.0);
    
}