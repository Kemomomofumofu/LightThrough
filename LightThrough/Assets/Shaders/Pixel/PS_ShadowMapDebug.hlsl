Texture2DArray<float> shadowMap : register(t0);
SamplerComparisonState shadowSampler : register(s0);

struct PSIN
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 PSMain(PSIN pin) : SV_Target
{
    
    float layer = 0; // 表示したいシャドウレイヤー
    float depth = shadowMap.SampleCmpLevelZero(shadowSampler, float3(pin.uv, layer), 1.0f);
    return float4(depth, depth, depth, 1.0f);
}