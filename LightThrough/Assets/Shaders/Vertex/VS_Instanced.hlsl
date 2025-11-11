
struct VSVertex
{
    float3 pos : POSITION0;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
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
    float3 worldPos : WORLDPOS;
};

cbuffer cbperFrame : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
};


VSOUT VSMain(VSVertex _vin, VSInstance _inst)
{
    VSOUT vout;
    
    row_major float4x4 worldMatrix = float4x4(_inst.row0, _inst.row1, _inst.row2, _inst.row3);
    
    // ワールド座標
    float4 wp = mul(float4(_vin.pos, 1.0f), worldMatrix);
    vout.worldPos = wp.xyz;
    
    // クリップ座標
    float4 p = mul(wp, viewMatrix);
    p = mul(p, projectionMatrix);
    vout.pos = p;
    
    // 法線(ワールドスペース)
    float3 nWS = normalize(mul(_vin.normal, (float3x3) worldMatrix));
    vout.normalWS = nWS;
    
    // 色
    vout.color = _vin.color * _inst.color;

    return vout;
}