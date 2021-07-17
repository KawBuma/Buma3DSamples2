#pragma once
#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <memory>

namespace buma
{

class DeviceResources;
class CommandQueue;

class StagingBufferPool;

class CopyContext
{
public:
    CopyContext(DeviceResources& _dr, buma3d::COMMAND_TYPE _type);
    ~CopyContext();

    buma3d::COMMAND_TYPE GetCommandType() const { return type; }

    void PipelineBarrier(const buma3d::CMD_PIPELINE_BARRIER& _barrier);

    void CopyDataToBuffer(buma3d::IBuffer* _dst_buffer, uint64_t _dst_offset, size_t _src_size, const void* _src_data);
    void CopyDataToTexture(buma3d::ITexture* _dst_texture, uint32_t _mip_slice, uint32_t _array_slice, uint64_t _src_row_pitch, uint64_t _src_texture_height, size_t _src_size, const void* _src_data);

    //void CopyBufferToData(BUFFER_ALLOCATION_PART* _dst_result_allocation, uint64_t _src_offset, uint64_t _src_size, buma3d::IBuffer* _src_buffer);              // End()呼び出し後にBUFFER_ALLOCATION_PART::map_data_partから取得します。
    //void CopyTextureToData(BUFFER_ALLOCATION_PART* _dst_result_allocation, buma3d::ITexture* _src_texture, uint32_t _mip_slice = 0, uint32_t _array_slice = 0); // End()呼び出し後にBUFFER_ALLOCATION_PART::map_data_partから取得します。

    void CopyBufferRegion(const buma3d::CMD_COPY_BUFFER_REGION& _args);
    void CopyTextureRegion(const buma3d::CMD_COPY_TEXTURE_REGION& _args);

    void CopyBufferToTexture(const buma3d::CMD_COPY_BUFFER_TO_TEXTURE& _args);
    void CopyTextureToBuffer(const buma3d::CMD_COPY_TEXTURE_TO_BUFFER& _args);

    buma3d::ICommandList* GetCommandList() const { return list.Get(); }

    void MakeVisible();
    bool HasCommand() const;
    void Reset();
    void Begin();
    const buma3d::SUBMIT_INFO& End();
    buma3d::IFence* GetCommandCompleteFence() const;

private:
    void Init();

private:
    DeviceResources&                                dr;
    buma3d::COMMAND_TYPE                            type;
    buma3d::util::Ptr<buma3d::ICommandAllocator>    allocator;
    buma3d::util::Ptr<buma3d::ICommandList>         list;

    std::unique_ptr<StagingBufferPool>              upload_buffer;
    std::unique_ptr<StagingBufferPool>              readback_buffer;

    buma3d::util::Ptr<buma3d::IFence>               fence;
    uint64_t                                        fence_val;
    buma3d::SUBMIT_INFO                             submit_info;
    bool                                            resetted;
    bool                                            has_command;

};


}// namespace buma
