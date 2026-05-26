struct PSIN
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D srcTex : register(t0);
SamplerState samplerLinearClamp : register(s0);

float Luma(float3 _c)
{
    return dot(_c, float3(0.299f, 0.587f, 0.114f));
}

float4 PSMain(PSIN _pin) : SV_Target
{
    uint w, h;
    srcTex.GetDimensions(w, h);
    float2 texel = 1.0f / float2(w, h);

    float3 c = srcTex.Sample(samplerLinearClamp, _pin.uv).rgb;
    float l = Luma(c);

    float lL = Luma(srcTex.Sample(samplerLinearClamp, _pin.uv + float2(-texel.x, 0.0f)).rgb);
    float lR = Luma(srcTex.Sample(samplerLinearClamp, _pin.uv + float2(texel.x, 0.0f)).rgb);
    float lU = Luma(srcTex.Sample(samplerLinearClamp, _pin.uv + float2(0.0f, -texel.y)).rgb);
    float lD = Luma(srcTex.Sample(samplerLinearClamp, _pin.uv + float2(0.0f, texel.y)).rgb);

    float edge = max(max(abs(l - lL), abs(l - lR)), max(abs(l - lU), abs(l - lD)));
    edge = saturate(edge * 4.0f);

    float3 outline = float3(0.0f, 0.0f, 0.0f);
    float3 outColor = lerp(c, outline, edge);

    return float4(outColor, 1.0f);
}