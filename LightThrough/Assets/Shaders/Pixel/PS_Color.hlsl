// 色をそのまま出力するピクセルシェーダ

struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0; // 未使用
    float3 worldPos : WORLDPOS; // 未使用
};

float4 PSMain(PSIN _pin) : SV_Target
{
    return float4(_pin.color.rgb, _pin.color.a);
}