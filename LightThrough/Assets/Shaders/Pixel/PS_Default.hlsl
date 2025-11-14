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
    float3 accum = 0;
    
    // ライト計算
    [loop]
    for (int i = 0; i < lightCount; ++i)
    {
        float li = ComputeLight(lights[i], N, _pin.worldPos);
        accum += li * lights[i].color.rgb;
    }
    
    // ライト空間での位置を正規化
    float3 shadowCoord = _pin.posLight.xyz / _pin.posLight.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;
    
    float shadow = shadowMap.SampleCmpLevelZero(shadowSampler, shadowCoord.xy, shadowCoord.z);
    
    // ライティング
    float3 lightDir = normalize(float3(0.3f, -1.0f, 0.2f));
    float diff = max(dot(_pin.normalWS, -lightDir), 0.0);
    
    float shade = lerp(0.3, 1.0, shadow);
    accum *= diff * shade;
    
    return float4(accum, 1.0);
    
}