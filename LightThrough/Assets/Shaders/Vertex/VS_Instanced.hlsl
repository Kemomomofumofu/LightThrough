cbuffer cbperFrame : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
};

struct VSVertex
{
    float3 pos : POSITION0;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VSInstance
{
    float4 row0 : INSTANCE_ROW0;
    float4 row1 : INSTANCE_ROW1;
    float4 row2 : INSTANCE_ROW2;
    float4 row3 : INSTANCE_ROW3;
    float4 color : INSTANCE_COLOR;
};

struct VSOUT
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float2 uv : TEXCOORD0;
};

VSOUT VSMain(VSVertex vin, VSInstance inst)
{
    VSOUT vout;
    
    row_major float4x4 worldMatrix = float4x4(inst.row0, inst.row1, inst.row2, inst.row3);
    
    float4 p = float4(vin.pos, 1.0f);
    p = mul(p, worldMatrix);
    p = mul(p, viewMatrix);
    p = mul(p, projectionMatrix);
    vout.pos = p;
    
    float3 nWS = normalize(mul(vin.normal, (float3x3) worldMatrix));
    vout.normalWS = nWS;
    vout.uv = vin.uv;
    vout.color = vin.color * inst.color;
    
    return vout;
}