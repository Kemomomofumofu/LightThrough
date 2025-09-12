// 一番シンプルなシェーダ
cbuffer cbperFrame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 worldMatrix;
}

struct VSIN
{
    float3 pos : POSITION0;
    float4 col : COLOR0;
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
    float4 worldPos = mul(worldMatrix, float4(vin.pos, 1.0f));
    float4 viewPos = mul(viewMatrix, worldPos);
    vout.pos = mul(projectionMatrix, viewPos);
    
    vout.col = vin.col;
    return vout;
}

float4 PSMain(VSOUT pin) : SV_Target
{
    return float4(pin.col);
}