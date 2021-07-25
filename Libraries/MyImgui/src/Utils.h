#pragma once

#include <Buma3D/Util/Buma3DPtr.h>

#include <Buma3DHelpers/B3DDescHelpers.h>
#include <Buma3DHelpers/FormatUtils.h>

#include <DeviceResources/DeviceResources.h>

#include <utility>
#include <cassert>

#define BMR_RET_IF_FAILED(x) if (x >= buma3d::BMRESULT_FAILED) { assert(false && #x); return false; }
#define RET_IF_FAILED(x) if (!(x)) { assert(false && #x); return false; }


namespace buma
{

class DeviceResources;
class CommandQueue;
class Texture;

namespace gui
{

struct RENDER_RESOURCE
{
    struct tex_deleter {
        constexpr tex_deleter() noexcept = default;
        tex_deleter(DeviceResources* _dr) noexcept : dr{ _dr } {}
        tex_deleter(const tex_deleter& _d) noexcept { dr = _d.dr; }
        void operator()(Texture * _tex) const noexcept { dr->DestroyTexture(_tex); }
        DeviceResources* dr = nullptr;
    };
    using MYIMGUI_CREATE_FLAGS = uint32_t;
    MYIMGUI_CREATE_FLAGS                            flags;
    DeviceResources*                                dr;
    buma3d::util::Ptr<buma3d::IDevice>              device;
    CommandQueue*                                   queue;
    util::PipelineBarrierDesc                       barriers;
    buma3d::util::Ptr<buma3d::ISamplerView>         sampler;
    std::unique_ptr<Texture, tex_deleter>           font_texture;
    buma3d::util::Ptr<buma3d::IShaderResourceView>  font_srv;
    buma3d::util::Ptr<buma3d::IShaderModule>        vs;
    buma3d::util::Ptr<buma3d::IShaderModule>        ps;
    buma3d::util::Ptr<buma3d::IDescriptorSetLayout> sampler_layout;
    buma3d::util::Ptr<buma3d::IDescriptorSetLayout> texture_layout;
    buma3d::util::Ptr<buma3d::IPipelineLayout>      pipeline_layout;
    buma3d::util::Ptr<buma3d::IPipelineState>       pipeline_state_load;
    buma3d::util::Ptr<buma3d::IRenderPass>          render_pass_load;
    buma3d::util::Ptr<buma3d::IRenderPass>          render_pass_clear;
};


inline void GetSurfaceInfo(
      size_t                    _width
    , size_t                    _height
    , buma3d::RESOURCE_FORMAT   _fmt
    , size_t*                   _out_num_bytes
    , size_t*                   _out_row_bytes
    , size_t*                   _out_num_rows)
{
    size_t num_bytes = 0;
    size_t row_bytes = 0;
    size_t num_rows = 0;

    bool is_bc = false;
    bool is_packed = false;
    bool is_planar = false;
    size_t bpe = 0;
    switch (_fmt)
    {
    case buma3d::RESOURCE_FORMAT_BC1_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC1_UNORM:
    case buma3d::RESOURCE_FORMAT_BC1_UNORM_SRGB:
    case buma3d::RESOURCE_FORMAT_BC4_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC4_UNORM:
    case buma3d::RESOURCE_FORMAT_BC4_SNORM:
        is_bc = true;
        bpe = 8;
        break;

    case buma3d::RESOURCE_FORMAT_BC2_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC2_UNORM:
    case buma3d::RESOURCE_FORMAT_BC2_UNORM_SRGB:
    case buma3d::RESOURCE_FORMAT_BC3_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC3_UNORM:
    case buma3d::RESOURCE_FORMAT_BC3_UNORM_SRGB:
    case buma3d::RESOURCE_FORMAT_BC5_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC5_UNORM:
    case buma3d::RESOURCE_FORMAT_BC5_SNORM:
    case buma3d::RESOURCE_FORMAT_BC6H_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC6H_UF16:
    case buma3d::RESOURCE_FORMAT_BC6H_SF16:
    case buma3d::RESOURCE_FORMAT_BC7_TYPELESS:
    case buma3d::RESOURCE_FORMAT_BC7_UNORM:
    case buma3d::RESOURCE_FORMAT_BC7_UNORM_SRGB:
        is_bc = true;
        bpe = 16;
        break;

    default:
        break;
    }

    if (is_bc)
    {
        size_t num_blocks_wide = 0;
        if (_width > 0)
        {
            num_blocks_wide = std::max<size_t>(1, (_width + 3) / 4);
        }
        size_t num_blocks_high = 0;
        if (_height > 0)
        {
            num_blocks_high = std::max<size_t>(1, (_height + 3) / 4);
        }
        row_bytes = num_blocks_wide * bpe;
        num_rows = num_blocks_high;
        num_bytes = row_bytes * num_blocks_high;
    }
    else if (is_packed)
    {
        row_bytes = ((_width + 1) >> 1) * bpe;
        num_rows = _height;
        num_bytes = row_bytes * _height;
    }
    else if (is_planar)
    {
        row_bytes = ((_width + 1) >> 1) * bpe;
        num_bytes = (row_bytes * _height) + ((row_bytes * _height + 1) >> 1);
        num_rows = _height + ((_height + 1) >> 1);
    }
    else
    {
        auto size = util::GetFormatSize(_fmt);
        size_t bpp = (size / util::CalcTexelsPerBlock(_fmt)) * 8;
        row_bytes = (_width * bpp + 7) / 8; // round up to nearest byte
        num_rows = _height;
        num_bytes = row_bytes * _height;
    }

    if (_out_num_bytes)
        *_out_num_bytes = num_bytes;

    if (_out_row_bytes)
        *_out_row_bytes = row_bytes;

    if (_out_num_rows)
        *_out_num_rows = num_rows;
}


}// namespace gui
}// namespace buma
