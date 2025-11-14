cbuffer LightMatrixBuffer : register(b0)
{
    matrix lightViewProj;
};

struct VSIN
{
    float3 pos : POSITIONT;
};

struct VSOUT
{
    float4 pos : SV_POSITION;
};

VSOUT VSMain(VSIN _vin)
{
    VSOUT vout;
    vout.pos = mul(float4(_vin.pos, 1.0f), lightViewProj);    
    return vout;
}