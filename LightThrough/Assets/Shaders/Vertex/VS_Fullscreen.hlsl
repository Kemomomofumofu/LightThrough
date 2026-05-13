struct VSOUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOUT VSMain(uint _vid : SV_VertexID)
{
    VSOUT vout;
    
    // 画面全体を覆う三角形の頂点座標を定義
    float2 pos = (_vid == 0) ? float2(-1.0f, -1.0f) :
                 (_vid == 1) ? float2( 3.0f, -1.0f) :
                               float2(-1.0f,  3.0f);
    vout.pos = float4(pos, 0.0f, 1.0f);
    vout.uv = pos * 0.5f + 0.5f; 
    
    return vout;
}