
struct VS_IN
{
    float4 pos  : POSITION;
    float2 uv   : TEXCOORD;
};

struct PS_IN
{
    float4 pos  : SV_Position;
    float2 uv   : TEXCOORD;
};
