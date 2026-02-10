// スプライト用ピクセルシェーダ

struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float3 worldPos : WORLDPOS;
    float4 posLight : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

Texture2D spriteTex : register(t0);
SamplerState samplerLinearClamp : register(s0);

float4 PSMain(PSIN _pin) : SV_Target
{    
    float4 texColor = spriteTex.Sample(samplerLinearClamp, _pin.uv);
    return texColor * _pin.color;
}