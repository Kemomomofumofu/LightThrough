#include "../Common/Lighting.hlsli"


struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float3 worldPos : WORLDPOS;
    float4 posLight : TEXCOORD0;
};


Texture2D shadowMap : register(t0);
SamplerComparisonState shadowSampler : register(s0);


float4 PSMain(PSIN _pin) : SV_Target
{
    float3 N = normalize(_pin.normalWS);
    
    // ライト空間クリップ -> NDC
    float3 shadowCoord = _pin.posLight.xyz / _pin.posLight.w;
    float2 uv = float2(shadowCoord.x * 0.5f + 0.5f, -shadowCoord.y * 0.5f + 0.5f);
    float depth = saturate(shadowCoord.z);
    
    // 微小バイアスでアクネ軽減
    float shadow = shadowMap.SampleCmpLevelZero(shadowSampler, uv, depth - 0.001f);
    
    // ライト計算
    float3 accum = 0;
    [loop]
    for (int i = 0; i < lightCount; ++i)
    {
        float li = ComputeLight(lights[i], N, _pin.worldPos);
        accum += li * lights[i].color.rgb;
    }
        
    return float4(accum * shadow, 1.0);
    
}