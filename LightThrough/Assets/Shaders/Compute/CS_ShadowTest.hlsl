cbuffer CSParams : register(b0)
{
    row_major float4x4 lightViewProj;
    uint numPoints;
    uint shadowWidth;
    uint shadowHeight;
    uint sliceIndex;
    float depthBias;
    uint _pad0;
};

StructuredBuffer<float3> points : register(t0); // ポイントリスト
Texture2DArray<float> shadowMap : register(t1); // シャドウマップ

RWStructuredBuffer<uint> outFlags : register(u0); // 出力 0: lit, 1: shadow

[numthreads(64, 1, 1)]
void CSMain(uint3 _tid : SV_DispatchThreadID)
{
    uint idx = _tid.x;
    if (idx >= numPoints)
        return;

    float3 P = points[idx];
    // クリップ座標に変換
    float4 clip = mul(float4(P, 1.0f), lightViewProj);
    
    // wが0に近い場合
    if (abs(clip.w) < 1e-6f)
    {
        outFlags[idx] = 1u; // shadow
        return;
    }
    
    float3 ndc = clip.xyz / clip.w; // -1..1
    float2 uv = ndc.xy * 0.5f + 0.5f; // 0..1
    float zLight = ndc.z * 0.5f + 0.5f; // 0..1
    
    // 範囲外
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
    {
        outFlags[idx] = 1u; // shadow
        return;
    }
    
    // テクセル座標
    int tx = (int) (uv.x * (shadowWidth - 1));
    int ty = (int) (uv.y * (shadowHeight - 1));

    // シャドウマップから深度を取得
    float stored = shadowMap.Load(int4(tx, ty, (int)sliceIndex, 0)).x;
    
    // シャドウ判定
    uint isShadow = (zLight > stored + depthBias) ? 1u : 0u;
    
    outFlags[idx] = isShadow;
}