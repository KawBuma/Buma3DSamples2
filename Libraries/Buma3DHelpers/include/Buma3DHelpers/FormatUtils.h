#pragma once

#include <Buma3D/Buma3D.h>

namespace buma
{
namespace util
{

size_t GetFormatSize(buma3d::RESOURCE_FORMAT _format);
size_t GetDepthOrStencilFormatSize(buma3d::RESOURCE_FORMAT _format, bool _is_stencil);

void GetFormatBlockSize(buma3d::RESOURCE_FORMAT _format, uint32_t* _dst_block_w, uint32_t* _dst_block_h);

size_t CalcTexelsPerBlock(buma3d::RESOURCE_FORMAT _format);

bool IsTypelessFormat(buma3d::RESOURCE_FORMAT _format);
buma3d::RESOURCE_FORMAT GetTypelessFormat(buma3d::RESOURCE_FORMAT _format);

bool IsDepthStencilFormat(buma3d::RESOURCE_FORMAT _format);
bool IsDepthOnlyFormat(buma3d::RESOURCE_FORMAT _format);

bool IsIntegerFormat(buma3d::RESOURCE_FORMAT _format);
bool IsSintFormat(buma3d::RESOURCE_FORMAT _format);
bool IsUintFormat(buma3d::RESOURCE_FORMAT _format);
bool IsSrgbFormat(buma3d::RESOURCE_FORMAT _format);

}// namespace util
}// namespace buma
