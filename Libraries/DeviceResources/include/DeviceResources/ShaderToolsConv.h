#pragma once

#include <DeviceResources/DeviceResources.h>

/* このヘッダを使用するプロジェクトは ShaderTools ライブラリがリンクされている事を想定しています。 */
//#include <ShaderTools/ShaderLoader.h>

namespace buma
{
namespace shader
{

inline COMPILE_TARGET ConvertApiType(INTERNAL_API_TYPE _api_type)
{
    switch (_api_type)
    {
    case buma::INTERNAL_API_TYPE_D3D12:  return COMPILE_TARGET_D3D12;
    case buma::INTERNAL_API_TYPE_VULKAN: return COMPILE_TARGET_VULKAN;
    default:
        return COMPILE_TARGET(-1);
    }
}

inline SHADER_STAGE ConvertShaderStage(buma3d::SHADER_STAGE_FLAG _stage)
{
    switch (_stage)
    {
    case buma3d::SHADER_STAGE_FLAG_VERTEX       : return shader::SHADER_STAGE_VERTEX;
    case buma3d::SHADER_STAGE_FLAG_HULL         : return shader::SHADER_STAGE_HULL;
    case buma3d::SHADER_STAGE_FLAG_DOMAIN       : return shader::SHADER_STAGE_DOMAIN;
    case buma3d::SHADER_STAGE_FLAG_GEOMETRY     : return shader::SHADER_STAGE_GEOMETRY;
    case buma3d::SHADER_STAGE_FLAG_PIXEL        : return shader::SHADER_STAGE_PIXEL;
    case buma3d::SHADER_STAGE_FLAG_COMPUTE      : return shader::SHADER_STAGE_COMPUTE;

    // TODO: shader::SHADER_STAGE_TASK/MESH
    //case buma3d::SHADER_STAGE_FLAG_TASK         : return shader::SHADER_STAGE_TASK;
    //case buma3d::SHADER_STAGE_FLAG_MESH         : return shader::SHADER_STAGE_MESH;

    case buma3d::SHADER_STAGE_FLAG_RAYGEN       : return shader::SHADER_STAGE_LIBRARY;
    case buma3d::SHADER_STAGE_FLAG_ANY_HIT      : return shader::SHADER_STAGE_LIBRARY;
    case buma3d::SHADER_STAGE_FLAG_CLOSEST_HIT  : return shader::SHADER_STAGE_LIBRARY;
    case buma3d::SHADER_STAGE_FLAG_MISS         : return shader::SHADER_STAGE_LIBRARY;
    case buma3d::SHADER_STAGE_FLAG_INTERSECTION : return shader::SHADER_STAGE_LIBRARY;
    case buma3d::SHADER_STAGE_FLAG_CALLABLE     : return shader::SHADER_STAGE_LIBRARY;

    case buma3d::SHADER_STAGE_FLAG_UNKNOWN:
    default:
        BUMA_ASSERT(false && "Invalid shader stage");
        return shader::SHADER_STAGE(-1);
    }
}


} // namespace shader
} // namespace buma
