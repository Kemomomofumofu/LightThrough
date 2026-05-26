struct PSIN
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D srcTex : register(t0);
SamplerState samplerLinearClamp : register(s0);

float4 PSMain(PSIN _pin) : SV_Target
{
    uint w, h;
    srcTex.GetDimensions(w, h);
    float2 texel = 1.0f / float2(w, h);

    float2 uv = _pin.uv;

    float scan = 0.9f + 0.1f * sin(uv.y * h * 3.14159f);

    float shift = 1.5f * texel.x;
    float r = srcTex.Sample(samplerLinearClamp, uv + float2(shift, 0.0f)).r;
    float g = srcTex.Sample(samplerLinearClamp, uv).g;
    float b = srcTex.Sample(samplerLinearClamp, uv + float2(-shift, 0.0f)).b;

    float3 color = float3(r, g, b) * scan;

    float noise = frac(sin(dot(uv * float2(w, h), float2(12.9898f, 78.233f))) * 43758.5453f);
    color = lerp(color, color + noise * 0.05f, 0.5f);

    return float4(saturate(color), 1.0f);
}