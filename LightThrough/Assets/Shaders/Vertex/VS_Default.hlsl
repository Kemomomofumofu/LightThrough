// 一番シンプルなシェーダ
cbuffer cbperFrame : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
};

cbuffer cbPerObject : register(b1)
{
    row_major float4x4 worldMatrix;
}

struct VSIN
{
    float3 pos : POSITION0;
    float4 col : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VSOUT
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
};

VSOUT VSMain(VSIN vin)
{
    VSOUT vout;
    
    // 座標変換
    float4 p = float4(vin.pos, 1.0f);
    p = mul(p, worldMatrix);
    p = mul(p, viewMatrix);
    p = mul(p, projectionMatrix);
    vout.pos = p;
    
    vout.col = vin.col;
    return vout;
}