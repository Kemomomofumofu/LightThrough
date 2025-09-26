
struct PSIN
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
};


float4 PSMain(PSIN pin) : SV_Target
{
    return float4(pin.col);
}