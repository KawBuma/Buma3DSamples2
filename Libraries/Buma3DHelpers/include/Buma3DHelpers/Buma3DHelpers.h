#pragma once

#include <Buma3D/Buma3D.h>

#include <Utils/Definitions.h>

#include <algorithm>

#define BMR_ASSERT(x) BUMA_ASSERT(buma::util::IsSucceeded(x))

namespace buma::util
{

inline buma3d::UINT3 CalcMipExtents(uint32_t _mip_slice, const buma3d::EXTENT3D& _extent_mip0)
{
    return buma3d::UINT3{  (std::max)(_extent_mip0.width  >> _mip_slice, 1ui32)
                         , (std::max)(_extent_mip0.height >> _mip_slice, 1ui32)
                         , (std::max)(_extent_mip0.depth  >> _mip_slice, 1ui32) };
}

inline bool IsSucceeded(buma3d::BMRESULT _bmr)
{
    return _bmr < buma3d::BMRESULT_FAILED;
}

inline bool IsFailed(buma3d::BMRESULT _bmr)
{
    return _bmr >= buma3d::BMRESULT_FAILED;
}


} // namespace buma::util
