#include "../Common/Lighting.hlsli"


struct PSIN
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float3 normalWS : NORMAL0;
    float3 worldPos : WORLDPOS;
};


Texture2DArray<float> shadowMap : register(t0);
SamplerComparisonState shadowSampler : register(s0);

// シャドウの計算
float CalcShadowFactor(float3 _worldPos, int _shadowIndex, matrix _lightVP)
{
    // シャドウマップ未使用
    if (_shadowIndex < 0)
    {
        return 0.0f;
    }
    
    float4 lightPos = mul(float4(_worldPos, 1), _lightVP);
    float3 uvw = lightPos.xyz / lightPos.w;
    
    // UV変換
    uvw = uvw * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f);

    // 影外チェック
    if (uvw.x < 0 || uvw.x > 1 || uvw.y < 0 || uvw.y > 1)
    {
        return 0.0; // 光側
    }
    
    // PCFサンプリング
    float2 texel = 1.0f / float2(2048.0f, 2048.0f);
    float result = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 offset = float2(x, y) * texel;
            result += shadowMap.SampleCmpLevelZero(
            shadowSampler,
            float3(uvw.xy + offset, _shadowIndex),
            uvw.z
            );
        }
    }

    
    return result / 9.0;
}


float4 PSMain(PSIN _pin) : SV_Target
{
    float3 N = normalize(_pin.normalWS);
    
    float3 color = float3(0.0f, 0.0f, 0.0f);
    float totalLight = 0.0f;
    [unroll]
    for (int i = 0; i < lightCount; ++i)
    {
        // ライティング計算
        float li = ComputeLight(lights[i], N, _pin.worldPos);
        
        // シャドウ計算
        int shadowIndex = (int) lights[i].spotAngles_shadowIndex.z;
        float shadowFactor = CalcShadowFactor(_pin.worldPos, shadowIndex, lightViewProjs[i]);
        
        float effectiveLight = li * shadowFactor;
       
        // 色加算
        color += effectiveLight * lights[i].color.rgb;
        totalLight += effectiveLight;
    }
    
    // 光が当たっていない場合は破棄
    if (totalLight <= 0.0f)
    {
        discard;
    }
    
    //
    float threshold = 0.3f;
    float edgeWidth = 0.2f;
    
    float alpha = smoothstep(threshold - edgeWidth, threshold, totalLight);
    alpha *= _pin.color.a;

    return float4(saturate(color), alpha);
}