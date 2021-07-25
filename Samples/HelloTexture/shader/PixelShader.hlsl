#include "./ShaderHeader.hlsli"

Texture2D<float4>   color_texture : register(t0, space1);
SamplerState        color_sampler : register(s1, space1);

[shader("pixel")]
float4 main(PS_IN _in) : SV_TARGET
{
    return float4(color_texture.Sample(color_sampler, _in.uv).rgb, (_in.uv.x + _in.uv.y) * .5f);
    //return color_texture.Sample(color_sampler, _in.uv);
}
