
struct LightPacked
{
    float4 pos_type; // xyz = pos, w = type
    float4 dir_range; // xyz = dir, w = range
    float4 color;
    float4 spotAngles; // x = innerCos, y = outerCos, z,w 未使用
};

cbuffer LightBuffer : register(b1)
{
    int lightCount;
    float3 _pad0;
    LightPacked lights[16];
};


float Lambert(float3 _n, float3 _l)
{
    return saturate(dot(_n, _l));
}



// ディレクショナルライトの計算
float ComputeDirectional(LightPacked _light, float3 _normal)
{
    float L = normalize(-_light.dir_range.xyz);
    return Lambert(_normal, L);

}

// スポットライトの計算
float ComputeSpot(LightPacked _light, float3 _normal, float3 _worldPos)
{
    float3 L = _light.pos_type.xyz - _worldPos;
    
    // 距離と範囲判定(早期リターン)
    float range = _light.dir_range.w;
    float dist2 = dot(L, L);
    float range2 = range * range;
    if (dist2 > range2)
    {
        return 0.0;
    }
    
    // 距離減衰
    float dist = sqrt(dist2);
    L /= max(dist, 1e-4);
    
    // 裏側判定(早期リターン)
    float ndotl = Lambert(_normal, L);
    if (ndotl <= 0)
    {
        return 0.0;
    }
    
    // スポット角度減衰
    float3 dir = normalize(-_light.dir_range.xyz);
    float angle = dot(L, dir);
    float inner = _light.spotAngles.x;
    float outer = _light.spotAngles.y;
    if (angle < outer)
    {
        return 0.0;
    }
    float spot = saturate((angle - outer) / max((inner - outer), 1e-4));
    
    float atten = 1.0 - saturate(dist / max(range, 1e-4));
    
    return ndotl * spot * atten;
}

float ComputeLight(LightPacked _light, float3 _normal, float3 _worldPos)
{
    int type = (int) _light.pos_type.w;
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