// 一番シンプルなシェーダ


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
    vout.pos = float4(vin.pos, 1);
    vout.col = vin.col;
    return vout;
}

float4 PSMain(VSOUT pin) : SV_Target
{
    return float4(pin.col);
}