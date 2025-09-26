cbuffer cbperFrame : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VSIN
{
    float3 pos : POSITION0;
    float4 col : COLOR0;
    
    float4 iworld0 : INSTANCEWORLD0;
    float4 iworld1 : INSTANCEWORLD1;
    float4 iworld2 : INSTANCEWORLD2;
    float4 iworld3 : INSTANCEWORLD3;
};

struct VSOUT
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
};

VSOUT VSMain(VSIN vin)
{
    VSOUT vout;
    
    float4x4 worldMatrix = float4x4(vin.iworld0, vin.iworld1, vin.iworld2, vin.iworld3);
    
    float4 worldPos = mul(worldMatrix, float4(vin.pos, 1.0f));
    float4 viewPos = mul(viewMatrix, worldPos);
    vout.pos = mul(projectionMatrix, viewPos);
    
    vout.col = vin.col;
    return vout;
}