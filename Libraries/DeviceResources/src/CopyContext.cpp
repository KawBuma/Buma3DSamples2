#include <DeviceResources/CopyContext.h>
#include <DeviceResources/DeviceResources.h>
#include "./StagingBufferPool.h"

#include <Utils/Utils.h>

#include <Buma3DHelpers/Buma3DHelpers.h>
#include <Buma3DHelpers/B3DInit.h>

namespace buma
{

CopyContext::CopyContext(DeviceResources& _dr, buma3d::COMMAND_TYPE _type)
    : dr              { _dr }
    , type            { _type }
    , allocator       {}
    , list            {}
    , upload_buffer   {}
    , readback_buffer {}
    , fence           {}
    , fence_val       {}
    , submit_info     {}
    , resetted        {}
    , has_command     {}
{
    Init();
}

CopyContext::~CopyContext()
{
}

void CopyContext::Init()
{
    auto&& d = dr.GetDevice();

    auto bmr = d->CreateFence(buma3d::init::TimelineFenceDesc(), &fence);
    BMR_ASSERT(bmr);

    bmr = d->CreateCommandAllocator(buma3d::init::CommandAllocatorDesc(type, buma3d::COMMAND_LIST_LEVEL_PRIMARY, buma3d::COMMAND_ALLOCATOR_FLAG_TRANSIENT), &allocator);
    BMR_ASSERT(bmr);

    bmr = d->AllocateCommandList(buma3d::init::CommandListDesc(allocator.Get(), buma3d::B3D_DEFAULT_NODE_MASK), &list);
    BMR_ASSERT(bmr);

    fence->SetName("CopyContext::fence");
    allocator->SetName("CopyContext::allocator");
    list->SetName("CopyContext::list");

    upload_buffer   = std::make_unique<StagingBufferPool>(dr, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE, buma3d::init::BUF_COPYABLE_FLAGS, util::Mib(16));
    readback_buffer = std::make_unique<StagingBufferPool>(dr, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE, buma3d::init::BUF_COPYABLE_FLAGS, util::Mib(16));

    submit_info.num_command_lists_to_execute = 1;
    submit_info.command_lists_to_execute     = list.GetAddressOf();
    submit_info.signal_fence.num_fences      = 1;
    submit_info.signal_fence.fences          = fence.GetAddressOf();
    submit_info.signal_fence.fence_values    = &fence_val;
}

void CopyContext::Reset()
{
    // 以前の実行を待機します。 
    auto bmr = fence->Wait(fence_val, UINT32_MAX);
    BMR_ASSERT(bmr);

    bmr = allocator->Reset(buma3d::COMMAND_ALLOCATOR_RESET_FLAG_NONE);
    BMR_ASSERT(bmr);

    upload_buffer->ResetPages();
    readback_buffer->ResetPages();

    resetted = true;
    has_command = false;

    bmr = list->BeginRecord({ buma3d::COMMAND_LIST_BEGIN_FLAG_ONE_TIME_SUBMIT });
    BMR_ASSERT(bmr);
}

void CopyContext::Begin()
{
    if (!resetted)
        Reset();
}

const buma3d::SUBMIT_INFO& CopyContext::End()
{
    BUMA_ASSERT(resetted);

    auto bmr = list->EndRecord();
    BMR_ASSERT(bmr);

    // コマンド送信前に、ホストの書き込みを確定させます。
    upload_buffer->MakeVisible();
    resetted = false;
    has_command = false;

    fence_val++;
    return submit_info;
}

buma3d::IFence* CopyContext::GetCommandCompleteFence() const
{
    return fence.Get();
}

void CopyContext::MakeVisible()
{
    readback_buffer->MakeVisible();
}

bool CopyContext::HasCommand() const
{
    return has_command;
}

void CopyContext::PipelineBarrier(const buma3d::CMD_PIPELINE_BARRIER& _barrier)
{
    list->PipelineBarrier(_barrier);
    has_command = true;
}

void CopyContext::CopyDataToBuffer(buma3d::IBuffer* _dst_buffer, uint64_t _dst_offset, size_t _src_size, const void* _src_data)
{
    auto al = upload_buffer->AllocateBufferPart(_src_size, 16);
    memcpy(al.map_data_part, _src_data, _src_size);

    buma3d::BUFFER_COPY_REGION copy_region{};
    copy_region.src_offset      = al.data_offset;
    copy_region.dst_offset      = _dst_offset;
    copy_region.size_in_bytes   = _src_size;

    buma3d::CMD_COPY_BUFFER_REGION copy{};
    copy.dst_buffer  = _dst_buffer;
    copy.num_regions = 1;
    copy.regions     = &copy_region;
    copy.src_buffer  = al.parent_resouce;

    list->CopyBufferRegion(copy);
    has_command = true;
}

void CopyContext::CopyDataToTexture(buma3d::ITexture* _dst_texture, uint32_t _mip_slice, uint32_t _array_slice, uint64_t _src_row_pitch, uint64_t _src_texture_height, size_t _src_size, const void* _src_data)
{
    auto al = upload_buffer->AllocateBufferPart(_src_size, dr.GetDeviceAdapterLimits().buffer_copy_offset_alignment);
    memcpy(al.map_data_part, _src_data, _src_size);

    buma3d::BUFFER_TEXTURE_COPY_REGION copy_region{};
    copy_region.buffer_layout.offset            = al.data_offset;
    copy_region.buffer_layout.row_pitch         = _src_row_pitch;
    copy_region.buffer_layout.texture_height    = (uint32_t)_src_texture_height;

    copy_region.texture_subresource.offset.aspect      = buma3d::TEXTURE_ASPECT_FLAG_COLOR;
    copy_region.texture_subresource.offset.mip_slice   = _mip_slice;
    copy_region.texture_subresource.offset.array_slice = _array_slice;
    copy_region.texture_subresource.array_count        = 1;

    copy_region.texture_offset = nullptr;
    copy_region.texture_extent = nullptr;

    buma3d::CMD_COPY_BUFFER_TO_TEXTURE copy{};
    copy.src_buffer  = al.parent_resouce;
    copy.dst_texture = _dst_texture;
    copy.num_regions = 1;
    copy.regions     = &copy_region;

    list->CopyBufferToTexture(copy);
    has_command = true;
}

//void CopyContext::CopyBufferToData(BUFFER_ALLOCATION_PART* _dst_result_allocation, uint64_t _src_offset, uint64_t _src_size, buma3d::IBuffer* _src_buffer)
//{
//    *_dst_result_allocation = readback_buffer->AllocateBufferPart(_src_size, dr.GetDeviceAdapterLimits().buffer_copy_offset_alignment);
//
//    buma3d::BUFFER_COPY_REGION copy_region{};
//    copy_region.src_offset      = _src_offset;
//    copy_region.dst_offset      = _dst_result_allocation->data_offset;
//    copy_region.size_in_bytes   = _src_size;
//
//    buma3d::CMD_COPY_BUFFER_REGION copy{};
//    copy.dst_buffer  = _dst_result_allocation->parent_resouce;
//    copy.num_regions = 1;
//    copy.regions     = &copy_region;
//    copy.src_buffer  = _src_buffer;
//
//    list->CopyBufferRegion(copy);
//}
//void CopyContext::CopyTextureToData(BUFFER_ALLOCATION_PART* _dst_result_allocation, buma3d::ITexture* _src_texture, uint32_t _mip_slice, uint32_t _array_slice)
//{
//    auto&& tex_desc = _src_texture->GetDesc().texture;
//    uint32_t bw{}, bh{};
//    util::GetFormatBlockSize(tex_desc.format_desc.format, &bw, &bh);
//    auto format_size = util::GetFormatSize(tex_desc.format_desc.format);
//
//    auto&& l = dr.GetDeviceAdapterLimits();
//    auto extent    = util::CalcMipExtents(_mip_slice, tex_desc.extent);
//    auto row_pitch = util::AlignUp(format_size * (extent.x / bw), l.buffer_copy_row_pitch_alignment);
//
//    auto buffer_size = util::AlignUp(row_pitch * extent.y, l.buffer_copy_row_pitch_alignment);
//    *_dst_result_allocation = upload_buffer->AllocateBufferPart(buffer_size, l.buffer_copy_offset_alignment);
//
//    buma3d::BUFFER_TEXTURE_COPY_REGION copy_region{};
//    copy_region.buffer_layout.offset            = _dst_result_allocation->data_offset;
//    copy_region.buffer_layout.row_pitch         = row_pitch;
//    copy_region.buffer_layout.texture_height    = extent.y;
//
//    copy_region.texture_subresource.offset.aspect      = buma3d::TEXTURE_ASPECT_FLAG_COLOR;
//    copy_region.texture_subresource.offset.mip_slice   = _mip_slice;
//    copy_region.texture_subresource.offset.array_slice = _array_slice;
//    copy_region.texture_subresource.array_count        = 1;
//
//    copy_region.texture_offset = nullptr;
//    copy_region.texture_extent = nullptr;
//
//    buma3d::CMD_COPY_TEXTURE_TO_BUFFER copy{};
//    copy.src_texture = _src_texture;
//    copy.dst_buffer  = _dst_result_allocation->parent_resouce;
//    copy.num_regions = 1;
//    copy.regions     = &copy_region;
//
//    list->CopyTextureToBuffer(copy);
//}

void CopyContext::CopyBufferRegion(const buma3d::CMD_COPY_BUFFER_REGION& _args)
{
    list->CopyBufferRegion(_args);
    has_command = true;
}
void CopyContext::CopyTextureRegion(const buma3d::CMD_COPY_TEXTURE_REGION& _args)
{
    list->CopyTextureRegion(_args);
    has_command = true;
}

void CopyContext::CopyBufferToTexture(const buma3d::CMD_COPY_BUFFER_TO_TEXTURE& _args)
{
    list->CopyBufferToTexture(_args);
    has_command = true;
}
void CopyContext::CopyTextureToBuffer(const buma3d::CMD_COPY_TEXTURE_TO_BUFFER& _args)
{
    list->CopyTextureToBuffer(_args);
    has_command = true;
}


}// namespace buma
