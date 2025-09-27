
struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};


float4 PSMain(PSIN pin) : SV_Target
{
    return pin.color;

}