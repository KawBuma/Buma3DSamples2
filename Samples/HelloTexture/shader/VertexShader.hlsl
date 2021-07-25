#include "./ShaderHeader.hlsli"

struct CB_MODEL
{
    float4x4 model;
};
ConstantBuffer<CB_MODEL> cb_model : register(b0, space0);

struct CB_SCENE
{
    float4x4 view_proj;
};
ConstantBuffer<CB_SCENE> cb_scene : register(b1, space0);

[shader("vertex")]
PS_IN main(VS_IN _in)
{
    PS_IN result = (PS_IN)0;
    result.pos = mul(cb_scene.view_proj, mul(cb_model.model, _in.pos));
    result.uv  = _in.uv;

    return result;
}
