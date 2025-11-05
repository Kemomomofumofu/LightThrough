
struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

cbuffer cbLight : register(b1)
{
    float3 lightDirWS; float _pad0;
    float3 lightColor; float _pad1;
    float3 ambientColor; float _pad2;
}




float4 PSMain(PSIN pin) : SV_Target
{
    float3 N = normalize(pin.normal);
    float3 L = normalize(-lightDirWS);
    
    float NdotL = saturate(dot(N, L));
    
    float3 albedo = pin.color.rgb;
    float3 diffuse = albedo * lightColor * NdotL;
    float3 ambient = albedo * ambientColor;
    
    float3 color = diffuse + ambient;
    
    return float4(color, pin.color.a);
}