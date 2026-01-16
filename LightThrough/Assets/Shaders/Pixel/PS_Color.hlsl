// 色をそのまま出力するピクセルシェーダ

struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float3 worldPos : WORLDPOS;
    float4 posLight : TEXCOORD0;
};

float4 PSMain(PSIN _pin) : SV_Target
{
    return float4(_pin.color.rgb, _pin.color.a);
}