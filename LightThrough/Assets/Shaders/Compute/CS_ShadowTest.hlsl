cbuffer CSParams : register(b0)
{
    row_major float4x4 lightViewProj;
    
    uint numPoints;
    uint shadowWidth;
    uint shadowHeight;
    uint sliceIndex;
    
    float3 lightPos;
    float _pad3;
    
    float3 lightDir;
    float _pad4;
    float cosOuterAngle;
    float cosInnerAngle;
    float lightRange;
    float _pad5;
    };

StructuredBuffer<float3> points : register(t0);
Texture2DArray<float> shadowMap : register(t1);
SamplerComparisonState shadowSampler : register(s0);

// outFlags: 0 = lit, 1 = shadow, 2 = outsideXY, 3 = wZero
RWStructuredBuffer<uint> outFlags : register(u0);

[numthreads(64, 1, 1)]
void CSMain(uint3 _tid : SV_DispatchThreadID)
{
    uint idx = _tid.x;
    if (idx >= numPoints)
        return;
    
    // shadowで初期化
    outFlags[idx] = 1u;

    float3 P = points[idx];
    
    // ライト位置からポイントへのベクトルと距離
    float3 toPoint = P - lightPos;
    float dist = length(toPoint);
    
    // 距離チェック
    if (dist > lightRange)
    {
        outFlags[idx] = 2u; // 範囲外
        return;
    }
    
    // 角度チェック
    float3 toPointDir = toPoint / dist;
    float cosAngle = dot(toPointDir, lightDir);
    if (cosAngle < cosOuterAngle)
    {
        outFlags[idx] = 2u; // 円錐外
        return;
    }
    
    // クリップ座標に変換
    float4 clip = mul(float4(P, 1.0f), lightViewProj);

    // wが0に近い場合
    if (abs(clip.w) < 1e-6f)
    {
        outFlags[idx] = 3u;
        return;
    }

    float3 ndc = clip.xyz / clip.w;
    
    // UV座標変換 (PS_Default.hlslと同じロジック)
    float3 uvw = ndc * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f);
    
    // 範囲外
    if (uvw.x < 0.0f || uvw.x > 1.0f || uvw.y < 0.0f || uvw.y > 1.0f)
    {
        outFlags[idx] = 2u;
        return;
    }

    // Z範囲外（ニアプレーンより手前、ファープレーンより奥）
    if (uvw.z < 0.0f || uvw.z > 1.0f)
    {
        outFlags[idx] = 2u; // 範囲外扱い
        return;
    }
    
    // SampleCmpLevelZero: uvw.z <= stored なら 1.0 (光の中)
    float shadowFactor = shadowMap.SampleCmpLevelZero(
        shadowSampler,
        float3(uvw.xy, (float) sliceIndex),
        uvw.z
    );
    
    // shadowFactor: 1.0 = 光の中, 0.0 = 影
    // 0.5より大きければ光の中と判定（PCFを考慮）
    outFlags[idx] = (shadowFactor > 0.5f) ? 0u : 1u;
}