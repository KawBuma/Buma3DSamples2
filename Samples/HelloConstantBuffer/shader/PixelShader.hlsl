#include "./ShaderHeader.hlsli"

[shader("pixel")]
float4 main(PS_IN _in) : SV_TARGET
{
    return _in.col;
}
