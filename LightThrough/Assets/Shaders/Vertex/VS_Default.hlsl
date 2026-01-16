// 一番シンプルなシェーダ
cbuffer cbperFrame : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
};

cbuffer cbPerObject : register(b1)
{
    row_major float4x4 worldMatrix;
    float4 objectColor;
};

cbuffer cbLightMatrix : register(b2)
{
    row_major float4x4 lightViewProj;
};

struct VSIN
{
    float3 pos : POSITION0;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
};

struct VSOUT
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float3 worldPos : WORLDPOS;
    float4 posLight : TEXCOORD0;
};


VSOUT VSMain(VSIN _vin)
{
    VSOUT vout;
    
    // ワールド座標
    float4 wp = mul(float4(_vin.pos, 1.0f), worldMatrix);
    vout.worldPos = wp.xyz;
    // クリップ座標
    float4 p = mul(wp, viewMatrix);
    p = mul(p, projectionMatrix);
    vout.pos = p;
    // ライト空間座標
    vout.posLight = mul(wp, lightViewProj);
    // 法線(ワールドスペース)
    vout.normalWS = normalize(mul(_vin.normal, (float3x3) worldMatrix));
    // 色
    vout.color = objectColor;
    
    vout.posLight = mul(wp, lightViewProj);
    
    return vout;
}