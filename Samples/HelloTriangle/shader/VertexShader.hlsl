#include "./ShaderHeader.hlsli"

[shader("vertex")]
PS_IN main(VS_IN _in)
{
    PS_IN result = (PS_IN) 0;
    result.pos = _in.pos;
    result.col = _in.col;

    #ifdef VK_INV_Y
    result.pos.y *= -1.f;
    #endif

    return result;
}
