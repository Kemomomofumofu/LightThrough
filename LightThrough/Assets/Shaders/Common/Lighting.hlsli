
struct LightPacked {
    float4 pos_type; // xyz = pos, w = type
    float4 dir_range; // xyz = dir, w = range
    float4 color;
    float4 spotAngles; // x = innerCos, y = outerCos, z,w 未使用
};

cbuffer LightBuffer : register(b1)
{
    int lightCount;          float3 _pad0;
    LightPacked lights[16];
};

// ディレクショナルライトの計算
float ComputeDirectional(LightPacked _light, float3 _normal)
{
    float L = normalize(-_light.dir_range.xyz);
    return saturate(dot(_normal, L));
}

// スポットライトの計算
float ComputeSpot(LightPacked _light, float3 _normal, float3 _worldPos)
{
    float3 L = _light.pos_type.xyz - _worldPos;
    float dist = length(L);
    if(dist > _light.dir_range.w) { return 0; }

    L /= dist;
    float dir = normalize(-_light.dir_range.xyz);
    float angle = dot(L, dir);
    if (angle < _light.spotAngles.y) { return 0; }
    float spot = saturate((angle - _light.spotAngles.y) / (_light.spotAngles.x - _light.spotAngles.y));
    
    return spot * saturate(dot(_normal, L)) * (1.0 - dist / _light.dir_range.w);
}

float ComputeLight(LightPacked _light, float3 _normal, float3 _worldPos)
{
    float type = _light.pos_type.w;
    if (type == 0)
    {
        return ComputeDirectional(_light, _normal);
    }
    else if (type == 1)
    {
        return ComputeSpot(_light, _normal, _worldPos);
    }
    
    return 0;

}