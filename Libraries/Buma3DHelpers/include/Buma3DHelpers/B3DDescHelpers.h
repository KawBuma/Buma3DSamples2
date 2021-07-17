#pragma once
#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <Buma3DHelpers/B3DInit.h>

#include <Utils/Utils.h>
#include <Utils/Definitions.h>

#include <cassert>
#include <set>
#include <vector>
#include <memory>
#include <array>
#include <functional>

namespace buma
{
namespace util
{

class Mapper
{
public:
    template<typename T>
    class MappedData
    {
    public:
        MappedData(T* _data) :data{ _data } {}
        ~MappedData() { data = nullptr; }

        T* GetData()
        {
            return data;
        }

        void Write(uint32_t _index, const T& _src)
        {
            if (data)
                data[_index] = _src;
        }
        bool WriteRaw(size_t _offset, const void* _src, size_t _size)
        {
            if (data)
                memcpy((uint8_t*)data + _offset, _src, _size);
        }

    private:
        T* data;

    };
    
public:
    Mapper(buma3d::util::Ptr<buma3d::IResourceHeap> _heap)
        : heap  { _heap }
        , range {}
        , data  {}
    {
        heap->Map();
        heap->GetMappedData(&range, &data);
    }

    Mapper(buma3d::util::Ptr<buma3d::IResourceHeap> _heap, const buma3d::MAPPED_RANGE& _range)
        : heap  { _heap }
        , range { _range }
        , data  {}
    {
        heap->Map(&range);
        heap->GetMappedData(&range, &data);
    }

    ~Mapper()
    {
        heap->Unmap();
        heap.Reset();
        range = {};
        data  = nullptr;
    }

    template<typename T>
    MappedData<T> As()
    {
        return MappedData<T>(reinterpret_cast<T*>(data));
    }

private:
    buma3d::util::Ptr<buma3d::IResourceHeap>    heap;
    buma3d::MAPPED_RANGE                        range;
    void*                                       data;

};

struct FENCE_VALUES
{
    uint64_t value;
    FENCE_VALUES& operator++()    { ++value; return *this; } 
    FENCE_VALUES  operator++(int) { auto tmp = *this; value++; return tmp; }
    uint64_t wait  () const { return value; }
    uint64_t signal() const { return value + 1; }
};

class FenceSubmitDesc
{
public:
    FenceSubmitDesc()
        : signal_desc  {}
        , wait_desc    {}
        , num_fences   {}
        , fences       {}
        , fence_values {}
    {}
    FenceSubmitDesc(const FenceSubmitDesc& _c)
        : signal_desc  { _c.signal_desc }
        , wait_desc    { _c.wait_desc }
        , num_fences   { _c.num_fences }
        , fences       { _c.fences }
        , fence_values { _c.fence_values }
    {
        Finalize();
    }
    FenceSubmitDesc& operator=(const FenceSubmitDesc& _c)
    {
        signal_desc  = _c.signal_desc;
        wait_desc    = _c.wait_desc;
        num_fences   = _c.num_fences;
        fences       = _c.fences;
        fence_values = _c.fence_values;
        return Finalize();
    }
    ~FenceSubmitDesc()
    {}

    FenceSubmitDesc& Reset()
    {
        num_fences = 0;

        signal_desc = {};
        wait_desc = {};

        return *this;
    }
    FenceSubmitDesc& AddFence(buma3d::IFence* _fence, uint64_t _fence_value)
    {
        Resize(num_fences + 1);
        fences      .data()[num_fences] = _fence;
        fence_values.data()[num_fences] = _fence_value;
        num_fences++;
        return *this;
    }
    FenceSubmitDesc& SetSignalFenceToGpu(buma3d::IFence* _fence)
    {
        signal_desc.signal_fence_to_cpu = _fence;
        return *this;
    }
    FenceSubmitDesc& Finalize()
    {
        signal_desc .signal_fence   .num_fences     = num_fences;
        signal_desc .signal_fence   .fences         = fences.data();
        signal_desc .signal_fence   .fence_values   = fence_values.data();
        wait_desc   .wait_fence     .num_fences     = num_fences;
        wait_desc   .wait_fence     .fences         = fences.data();
        wait_desc   .wait_fence     .fence_values   = fence_values.data();
        return *this;
    }

    const buma3d::SUBMIT_SIGNAL_DESC& GetAsSignal() const 
    {
        return signal_desc;
    }
    const buma3d::SUBMIT_WAIT_DESC& GetAsWait() const
    {
        return wait_desc;
    }
    const buma3d::FENCE_SUBMISSION& GetAsFenceSubmission() const
    {
        return GetAsWait().wait_fence;
    }

private:
    void Resize(uint32_t _num_fences)
    {
        if (_num_fences > (uint32_t)fences.size())
        {
            fences.resize(_num_fences);
            fence_values.resize(_num_fences);
        }
    }

private:
    buma3d::SUBMIT_SIGNAL_DESC      signal_desc;
    buma3d::SUBMIT_WAIT_DESC        wait_desc;
    uint32_t                        num_fences;
    std::vector<buma3d::IFence*>    fences;
    std::vector<uint64_t>           fence_values;

};

class SubmitInfo
{
public:
    SubmitInfo()
        : submit_info               {}
        , wait_fence                {}
        , signal_fence              {}
        , command_lists_to_execute  {}
    {}
    SubmitInfo(const SubmitInfo& _c)
        : submit_info               { _c.submit_info }
        , wait_fence                { _c.wait_fence }
        , signal_fence              { _c.signal_fence }
        , command_lists_to_execute  { _c.command_lists_to_execute }
    {
        Finalize();
    }
    SubmitInfo& operator=(const SubmitInfo& _c)
    {
        submit_info               = _c.submit_info;
        wait_fence                = _c.wait_fence;
        signal_fence              = _c.signal_fence;
        command_lists_to_execute  = _c.command_lists_to_execute;
        return Finalize();
    }
    ~SubmitInfo()
    {}

    SubmitInfo& Reset()
    {
        wait_fence.Reset();
        signal_fence.Reset();
        submit_info = {};
        return *this;
    }
    SubmitInfo& AddWaitFence(buma3d::IFence* _fence, uint64_t _fence_value = 0)
    {
        wait_fence.AddFence(_fence, _fence_value);
        return *this;
    }
    SubmitInfo& AddSignalFence(buma3d::IFence* _fence, uint64_t _fence_value = 0)
    {
        signal_fence.AddFence(_fence, _fence_value);
        return *this;
    }
    SubmitInfo& AddCommandList(buma3d::ICommandList* _command_list_to_execute)
    {
        Resize(submit_info.num_command_lists_to_execute + 1);
        command_lists_to_execute.data()[submit_info.num_command_lists_to_execute++] = _command_list_to_execute;
        return *this;
    }
    SubmitInfo& Finalize()
    {
        submit_info.wait_fence               = wait_fence.Finalize().GetAsFenceSubmission();
        submit_info.command_lists_to_execute = command_lists_to_execute.data();
        submit_info.signal_fence             = signal_fence.Finalize().GetAsFenceSubmission();
        return *this;
    }

    const buma3d::SUBMIT_INFO& Get() const 
    {
        return submit_info;
    }

private:
    void Resize(uint32_t _num_command_lists_to_execute)
    {
        if (_num_command_lists_to_execute > (uint32_t)command_lists_to_execute.size())
            command_lists_to_execute.resize(_num_command_lists_to_execute);
    }

private:
    buma3d::SUBMIT_INFO                 submit_info;
    FenceSubmitDesc                     wait_fence;
    FenceSubmitDesc                     signal_fence;
    std::vector<buma3d::ICommandList*>  command_lists_to_execute;

};

class SubmitDesc
{
public:
    SubmitDesc()
        : desc      {}
        , infos     {}
        , b3d_infos {}
    {}
    SubmitDesc(const SubmitDesc& _c)
        : desc      { _c.desc }
        , infos     { _c.infos }
        , b3d_infos { _c.b3d_infos }
    {
        Finalize();
    }
    SubmitDesc& operator=(const SubmitDesc& _c)
    {
        desc      = _c.desc;
        infos     = _c.infos;
        b3d_infos = _c.b3d_infos;
        return Finalize();
    }
    ~SubmitDesc()
    {}

    SubmitDesc& Reset()
    {
        for (auto& i : infos)
            i.Reset();
        desc = {};
        return *this;
    }
    SubmitDesc& SetSignalFenceToCpu(buma3d::IFence* _signal_fence_to_cpu)
    {
        desc.signal_fence_to_cpu = _signal_fence_to_cpu;
        return *this;
    }
    SubmitDesc& AddSubmitInfo(SubmitInfo&& _info)
    {
        Resize(desc.num_submit_infos + 1);
        infos.data()[desc.num_submit_infos++] = std::move(_info);
        return *this;
    }
    SubmitDesc& AddSubmitInfo(const SubmitInfo& _info)
    {
        Resize(desc.num_submit_infos + 1);
        infos.data()[desc.num_submit_infos++] = _info;
        return *this;
    }
    SubmitInfo& AddNewSubmitInfo()
    {
        Resize(desc.num_submit_infos + 1);
        return infos.data()[desc.num_submit_infos++];
    }
    SubmitDesc& Append(const SubmitDesc& _other_desc)
    {
        // _other_desc.desc.signal_fence_to_cpu は無視されます 
        Resize(desc.num_submit_infos + _other_desc.desc.num_submit_infos);
        for (auto& i : _other_desc.infos)
            AddSubmitInfo(i);
        return *this;
    }
    SubmitDesc& Finalize()
    {
        auto id = infos.data();
        auto bid = b3d_infos.data();
        for (uint32_t i = 0; i < desc.num_submit_infos; i++)
            bid[i] = id[i].Get();

        desc.submit_infos = bid;
        return *this;
    }

    const buma3d::SUBMIT_DESC& Get() const 
    {
        return desc;
    }

private:
    void Resize(uint32_t _num_infos)
    {
        if (_num_infos > (uint32_t)infos.size())
        {
            infos.resize(_num_infos);
            b3d_infos.resize(_num_infos);
        }
    }

private:
    buma3d::SUBMIT_DESC                 desc;
    std::vector<SubmitInfo>             infos;
    std::vector<buma3d::SUBMIT_INFO>    b3d_infos;

};

#pragma region pipeline barrier

class PipelineBarrierDesc;

class TextureBarrierRange
{
public:
    TextureBarrierRange(PipelineBarrierDesc* _parent)
        : parent        { _parent }
        , barrier_range {}
        , subres_ranges {}
    {}
    TextureBarrierRange(PipelineBarrierDesc* _parent, const buma3d::TEXTURE_BARRIER_RANGE* _range)
        : parent        { _parent }
        , barrier_range { *_range }
        , subres_ranges { _range->num_subresource_ranges }
    {
        memcpy(subres_ranges.data(), _range->subresource_ranges, sizeof(buma3d::SUBRESOURCE_RANGE) * _range->num_subresource_ranges);
        if (parent)
            Finalize();
    }
    TextureBarrierRange(const TextureBarrierRange& _c)
        : parent        { _c.parent }
        , barrier_range { _c.barrier_range }
        , subres_ranges { _c.subres_ranges }
    {
        if (parent)
            Finalize();
    }
    TextureBarrierRange& operator=(const TextureBarrierRange& _c)
    {
        parent        = _c.parent;
        barrier_range = _c.barrier_range;
        subres_ranges = _c.subres_ranges;
        if (parent)
            Finalize();
        return *this;
    }
    ~TextureBarrierRange()
    {}

    void SetParent(PipelineBarrierDesc* _parent)
    {
        parent = _parent;
    }

    TextureBarrierRange& Reset()
    {
        barrier_range.texture                   = nullptr;
        barrier_range.num_subresource_ranges    = 0;
        return *this;
    }
    TextureBarrierRange& AddSubresRange(buma3d::TEXTURE_ASPECT_FLAGS _aspect, uint32_t _mip_slice, uint32_t _array_slice, uint32_t _array_size = 1, uint32_t _mip_levels = 1)
    {
        Resize(barrier_range.num_subresource_ranges + 1);
        auto&& range = subres_ranges.data()[barrier_range.num_subresource_ranges++];
        range.array_size    = _array_size;
        range.mip_levels    = _mip_levels;
        range.offset.aspect         = _aspect;
        range.offset.mip_slice      = _mip_slice;
        range.offset.array_slice    = _array_slice;

        return *this;
    }
    TextureBarrierRange& SetTexture(buma3d::ITexture* _texture)
    {
        barrier_range.texture = _texture;
        return *this;
    }
    PipelineBarrierDesc& Finalize()
    {
        barrier_range.subresource_ranges = subres_ranges.data();
        return *parent;
    }

    void Finalize(PipelineBarrierDesc* _parent, const buma3d::TEXTURE_BARRIER_RANGE* _range)
    {
        SetParent(_parent);
        Resize(_range->num_subresource_ranges);
        memcpy(subres_ranges.data(), _range->subresource_ranges, sizeof(buma3d::SUBRESOURCE_RANGE) * _range->num_subresource_ranges);
        barrier_range = *_range;
        Finalize();
    }

    const buma3d::TEXTURE_BARRIER_RANGE& Get() const 
    {
        return barrier_range;
    }

private:
    void Resize(uint32_t _num_subres_ranges)
    {
        if (_num_subres_ranges > (uint32_t)subres_ranges.size())
            subres_ranges.resize(_num_subres_ranges);
    }

private:
    PipelineBarrierDesc*                    parent;
    buma3d::TEXTURE_BARRIER_RANGE           barrier_range;
    std::vector<buma3d::SUBRESOURCE_RANGE>  subres_ranges;

};

class PipelineBarrierDesc
{
public:
    PipelineBarrierDesc()
        : barrier               {}
        , buffer_barriers       {}
        , texture_barriers      {}
        , barrier_ranges        {}
        , num_barrier_ranges    {}
    {}
    PipelineBarrierDesc(uint32_t _num_buffer_barriers_reserve, uint32_t _num_texture_barriers_reserve)
        : barrier               {}
        , buffer_barriers       { _num_buffer_barriers_reserve }
        , texture_barriers      { _num_texture_barriers_reserve }
        , barrier_ranges        {}
        , num_barrier_ranges    {}
    {}
    PipelineBarrierDesc(const PipelineBarrierDesc& _c)
        : barrier               { _c.barrier }
        , buffer_barriers       { _c.buffer_barriers }
        , texture_barriers      { _c.texture_barriers }
        , barrier_ranges        { _c.barrier_ranges }
        , num_barrier_ranges    { _c.num_barrier_ranges }
    {
        for (auto& i : barrier_ranges) i.SetParent(this);
        Finalize();
    }
    PipelineBarrierDesc& operator=(const PipelineBarrierDesc& _c)
    {
        barrier             = _c.barrier;
        buffer_barriers     = _c.buffer_barriers;
        texture_barriers    = _c.texture_barriers;
        barrier_ranges      = _c.barrier_ranges;
        num_barrier_ranges  = _c.num_barrier_ranges;
        for (auto& i : barrier_ranges) i.SetParent(this);
        return Finalize();
    }
    ~PipelineBarrierDesc() {}

    PipelineBarrierDesc& Reset(bool _reset_pipeline_stage_flags = true, bool _reset_dependency_flags = true)
    {
        if (_reset_pipeline_stage_flags)
            barrier.dst_stages = barrier.src_stages = buma3d::PIPELINE_STAGE_FLAG_NONE;
        if (_reset_dependency_flags)
            barrier.dependency_flags = buma3d::DEPENDENCY_FLAG_NONE;

        barrier.num_buffer_barriers  = 0;
        barrier.num_texture_barriers = 0;
        num_barrier_ranges = 0;
        return *this;
    }
    PipelineBarrierDesc& AddBufferBarrier(  buma3d::IBuffer*                _buffer
                                          , buma3d::RESOURCE_STATE          _src_state
                                          , buma3d::RESOURCE_STATE          _dst_state
                                          , buma3d::RESOURCE_BARRIER_FLAG   _barrier_flags  = buma3d::RESOURCE_BARRIER_FLAG_NONE
                                          , buma3d::COMMAND_TYPE            _src_queue_type = buma3d::COMMAND_TYPE_DIRECT
                                          , buma3d::COMMAND_TYPE            _dst_queue_type = buma3d::COMMAND_TYPE_DIRECT)
    {
        Resize(barrier.num_buffer_barriers + 1, &buffer_barriers);
        buffer_barriers.data()[barrier.num_buffer_barriers++] = { _buffer , _src_state , _dst_state , _src_queue_type , _dst_queue_type , _barrier_flags };
        return *this;
    }
    PipelineBarrierDesc& AddBufferBarrier(const buma3d::BUFFER_BARRIER_DESC& _buffer_barrier)
    {
        Resize(barrier.num_buffer_barriers + 1, &buffer_barriers);
        buffer_barriers.data()[barrier.num_buffer_barriers++] = _buffer_barrier;
        return *this;
    }
    PipelineBarrierDesc& AddTextureBarrier(  buma3d::IView*                 _view
                                           , buma3d::RESOURCE_STATE         _src_state
                                           , buma3d::RESOURCE_STATE         _dst_state
                                           , buma3d::RESOURCE_BARRIER_FLAG  _barrier_flags = buma3d::RESOURCE_BARRIER_FLAG_NONE
                                           , buma3d::COMMAND_TYPE           _src_queue_type = buma3d::COMMAND_TYPE_DIRECT
                                           , buma3d::COMMAND_TYPE           _dst_queue_type = buma3d::COMMAND_TYPE_DIRECT)
    {
        Resize(barrier.num_texture_barriers + 1, &texture_barriers);
        (texture_barriers.data()[barrier.num_texture_barriers++] = { buma3d::TEXTURE_BARRIER_TYPE_VIEW , nullptr, _src_state , _dst_state , _src_queue_type , _dst_queue_type , _barrier_flags })
            .view = _view;
        return *this;
    }
    PipelineBarrierDesc& AddTextureBarrierRange(const buma3d::TEXTURE_BARRIER_RANGE*    _barrier_range
                                                , buma3d::RESOURCE_STATE                _src_state
                                                , buma3d::RESOURCE_STATE                _dst_state
                                                , buma3d::RESOURCE_BARRIER_FLAG         _barrier_flags  = buma3d::RESOURCE_BARRIER_FLAG_NONE
                                                , buma3d::COMMAND_TYPE                  _src_queue_type = buma3d::COMMAND_TYPE_DIRECT
                                                , buma3d::COMMAND_TYPE                  _dst_queue_type = buma3d::COMMAND_TYPE_DIRECT)
    {
        Resize(barrier.num_texture_barriers + 1, &texture_barriers);
        texture_barriers.data()[barrier.num_texture_barriers++] = { buma3d::TEXTURE_BARRIER_TYPE_BARRIER_RANGE , nullptr, _src_state , _dst_state , _src_queue_type , _dst_queue_type , _barrier_flags };

        ResizeTextureBarrierRange();
        barrier_ranges.data()[num_barrier_ranges++].Finalize(this, _barrier_range);
        return *this;
    }
    TextureBarrierRange& AddNewTextureBarrierRange(  buma3d::RESOURCE_STATE             _src_state
                                                   , buma3d::RESOURCE_STATE             _dst_state
                                                   , buma3d::RESOURCE_BARRIER_FLAG      _barrier_flags  = buma3d::RESOURCE_BARRIER_FLAG_NONE
                                                   , buma3d::COMMAND_TYPE               _src_queue_type = buma3d::COMMAND_TYPE_DIRECT
                                                   , buma3d::COMMAND_TYPE               _dst_queue_type = buma3d::COMMAND_TYPE_DIRECT)
    {
        Resize(barrier.num_texture_barriers + 1, &texture_barriers);
        texture_barriers.data()[barrier.num_texture_barriers++] = { buma3d::TEXTURE_BARRIER_TYPE_BARRIER_RANGE , nullptr, _src_state , _dst_state , _src_queue_type , _dst_queue_type , _barrier_flags };        

        ResizeTextureBarrierRange();
        return barrier_ranges.data()[num_barrier_ranges++];
    }
    PipelineBarrierDesc& SetPipelineStageFalgs(buma3d::PIPELINE_STAGE_FLAGS _src_stages, buma3d::PIPELINE_STAGE_FLAGS _dst_stages)
    {
        barrier.src_stages = _src_stages;
        barrier.dst_stages = _dst_stages;
        return *this;
    }
    PipelineBarrierDesc& SetDependencyFalgs(buma3d::DEPENDENCY_FLAGS _dependency_flags = buma3d::DEPENDENCY_FLAG_NONE)
    {
        barrier.dependency_flags = _dependency_flags;
        return *this;
    }
    PipelineBarrierDesc& Finalize()
    {
        auto tb = texture_barriers.data();
        auto r = barrier_ranges.data();
        uint32_t cnt = 0;
        for (uint32_t i = 0; i < barrier.num_texture_barriers; i++)
        {
            if (tb[i].type == buma3d::TEXTURE_BARRIER_TYPE_BARRIER_RANGE)
                tb[i].barrier_range = &(r[cnt++].Get());
        }

        barrier.texture_barriers = texture_barriers.data();
        barrier.buffer_barriers  = buffer_barriers.data();
        return *this;
    }

    const buma3d::CMD_PIPELINE_BARRIER& Get() const 
    {
        return barrier;
    }

private:
    template<typename T>
    void Resize(uint32_t _num, std::vector<T>* _dst)
    {
        if (_num > (uint32_t)_dst->size())
            _dst->resize(_num);
    }
    void ResizeTextureBarrierRange()
    {
        if (num_barrier_ranges + 1 > (uint32_t)barrier_ranges.size())
            barrier_ranges.resize(num_barrier_ranges + 1, this);
    }

private:
    buma3d::CMD_PIPELINE_BARRIER                barrier;
    std::vector<buma3d::BUFFER_BARRIER_DESC>    buffer_barriers;
    std::vector<buma3d::TEXTURE_BARRIER_DESC>   texture_barriers;
    std::vector<TextureBarrierRange>            barrier_ranges;
    uint32_t                                    num_barrier_ranges;

};

#pragma endregion pipeline barrier

#pragma region copy buffers

class CopyBufferRegion
{
public:
    CopyBufferRegion(uint32_t _num_regions_reserve)
        : copy_buffer   {}
        , regions       { _num_regions_reserve }
    {
    }
    CopyBufferRegion(const CopyBufferRegion& _c)
        : copy_buffer   { _c.copy_buffer }
        , regions       { _c.regions }
    {
        Finalize();
    }
    CopyBufferRegion& operator=(const CopyBufferRegion& _c)
    {
        copy_buffer   = _c.copy_buffer;
        regions       = _c.regions;
        Finalize();
    }

    ~CopyBufferRegion()
    {
    }

    CopyBufferRegion& Reset(bool _is_keep_src_dst_buffer = false)
    {
        copy_buffer.num_regions = 0;
        if (!_is_keep_src_dst_buffer)
        {
            copy_buffer.src_buffer = nullptr;
            copy_buffer.dst_buffer = nullptr;
        }
        return *this;
    }
    CopyBufferRegion& SetSrcBuffer(buma3d::IBuffer* _src_buffer)
    {
        copy_buffer.src_buffer = _src_buffer;
        return *this;
    }
    CopyBufferRegion& SetDstBuffer(buma3d::IBuffer* _dst_buffer)
    {
        copy_buffer.dst_buffer = _dst_buffer;
        return *this;
    }
    CopyBufferRegion& AddRegion(uint64_t _src_offset, uint64_t _dst_offset, uint64_t _size_in_bytes)
    {
        Resize(copy_buffer.num_regions + 1);
        regions.data()[copy_buffer.num_regions++] = { _src_offset, _dst_offset, _size_in_bytes };
        return *this;
    }
    CopyBufferRegion& Finalize()
    {
        copy_buffer.regions = regions.data();
        return *this;
    }
    const buma3d::CMD_COPY_BUFFER_REGION& Get() const
    {
        return copy_buffer;
    }

private:
    void Resize(uint32_t _size)
    {
        if (_size > (uint32_t)regions.size())
            regions.resize(_size);
    }

private:
    buma3d::CMD_COPY_BUFFER_REGION              copy_buffer;
    std::vector<buma3d::BUFFER_COPY_REGION>     regions;

};

class BufferTextureCopyRegion
{
public:
    BufferTextureCopyRegion(uint32_t _num_regions_reserve)
    {
    }
    BufferTextureCopyRegion(const BufferTextureCopyRegion& _c)
    {
    }
    BufferTextureCopyRegion& operator=(const BufferTextureCopyRegion& _c)
    {
    }

    ~BufferTextureCopyRegion()
    {
    }

private:

};


class CopyBufferToTexture
{
public:
    CopyBufferToTexture(uint32_t _num_regions_reserve)
        : copy_buffer {}
        , offsets     {}
        , extents     {}
        , b3d_regions { _num_regions_reserve }
    {
    }
    CopyBufferToTexture(const CopyBufferToTexture& _c)
        : copy_buffer { _c.copy_buffer }
        , offsets     { _c.offsets     }
        , extents     { _c.extents     }
        , b3d_regions { _c.b3d_regions }
    {
        Finalize();
    }
    CopyBufferToTexture& operator=(const CopyBufferToTexture& _c)
    {
        copy_buffer = _c.copy_buffer;
        offsets     = _c.offsets;
        extents     = _c.extents;
        b3d_regions = _c.b3d_regions;
        Finalize();
    }

    ~CopyBufferToTexture()
    {
    }


    CopyBufferToTexture& Reset(bool _is_keep_src_dst_infos = false)
    {
        copy_buffer.num_regions = 0;
        offsets.clear();
        extents.clear();

        if (_is_keep_src_dst_infos)
            return *this;

        copy_buffer.src_buffer = nullptr;
        copy_buffer.dst_texture = nullptr;
        return *this;
    }
    CopyBufferToTexture& SetSrcBuffer(buma3d::IBuffer* _src_buffer)
    {
        copy_buffer.src_buffer = _src_buffer;
        return *this;
    }
    CopyBufferToTexture& SetDstTexture(buma3d::ITexture* _dst_texture)
    {
        copy_buffer.dst_texture = _dst_texture;
        return *this;
    }
    CopyBufferToTexture& AddRegion(  const buma3d::BUFFER_SUBRESOURCE_LAYOUT& _buffer_layout
                                   , const buma3d::SUBRESOURCE_ARRAY&         _texture_subresource
                                   , const buma3d::OFFSET3D*                  _texture_offset = nullptr
                                   , const buma3d::EXTENT3D*                  _texture_extent = nullptr)
    {
        Resize(copy_buffer.num_regions + 1);
        auto&& r = b3d_regions.data()[copy_buffer.num_regions++] = { _buffer_layout, _texture_subresource, nullptr, nullptr };
        if (_texture_offset)
        {
            offsets.emplace_back(*_texture_offset);
            r.texture_offset = reinterpret_cast<buma3d::OFFSET3D*>(~0ull); // Finalize()へのヒントです。
        }
        if (_texture_extent)
        {
            extents.emplace_back(*_texture_extent);
            r.texture_extent = reinterpret_cast<buma3d::EXTENT3D*>(~0ull);
        }
        return *this;
    }
    CopyBufferToTexture& Finalize()
    {
        uint32_t offset_cnt = 0;
        uint32_t extent_cnt = 0;
        for (auto& i : b3d_regions)
        {
            if (i.texture_offset != nullptr) i.texture_offset = offsets.data() + offset_cnt++;
            if (i.texture_extent != nullptr) i.texture_extent = extents.data() + extent_cnt++;
        }
        copy_buffer.regions = b3d_regions.data();
        return *this;
    }

    const buma3d::CMD_COPY_BUFFER_TO_TEXTURE&   Get()                      const { return copy_buffer; } 
    const buma3d::BUFFER_TEXTURE_COPY_REGION&   GetRegion(uint32_t _index) const { return b3d_regions[_index]; }
    buma3d::BUFFER_TEXTURE_COPY_REGION&         GetRegion(uint32_t _index)       { return b3d_regions[_index]; }

private:
    void Resize(uint32_t _size)
    {
        if (_size > (uint32_t)b3d_regions.size())
            b3d_regions.resize(_size);
    }

private:
    buma3d::CMD_COPY_BUFFER_TO_TEXTURE              copy_buffer;
    std::vector<buma3d::OFFSET3D>                   offsets;
    std::vector<buma3d::EXTENT3D>                   extents;
    std::vector<buma3d::BUFFER_TEXTURE_COPY_REGION> b3d_regions;

};


#pragma endregion copy buffer region

#pragma region root signature

class RootParameter
{
public:
    RootParameter()
        : parameter {}
        , ranges    {}
    {
        Reset();
    }
    ~RootParameter()
    {
        Reset();
    }

    void Reset()
    {
        parameter = {};
        parameter.shader_visibility = buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE;
        ranges.reset();
    }

    void SetShaderVisibility(buma3d::SHADER_VISIBILITY _visibility)
    {
        parameter.shader_visibility = _visibility;
    }

    void InitAsPush32BitConstants(  uint32_t _num32_bit_values
                                  , uint32_t _shader_register
                                  , uint32_t _register_space = 0)
    {
        parameter.type = buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS;
        parameter.inline_constants.num32_bit_values = _num32_bit_values;
        parameter.inline_constants.shader_register  = _shader_register;
        parameter.inline_constants.register_space   = _register_space;
    }

    void InitAsDynamicDescriptor(  buma3d::DESCRIPTOR_TYPE   _type
                                 , uint32_t                  _shader_register
                                 , uint32_t                  _register_space     = 0 
                                 , buma3d::DESCRIPTOR_FLAGS  _flags              = buma3d::DEPENDENCY_FLAG_NONE)
    {
        parameter.type = buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR;
        parameter.dynamic_descriptor.type            = _type;
        parameter.dynamic_descriptor.shader_register = _shader_register;
        parameter.dynamic_descriptor.register_space  = _register_space;
        parameter.dynamic_descriptor.flags           = _flags;
    }

    void InitAsDescriptorTable()
    {
        BUMA_ASSERT(!ranges);
        parameter.type = buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        ranges = std::make_shared<std::vector<buma3d::DESCRIPTOR_RANGE>>();
    }

    void AddRange(  buma3d::DESCRIPTOR_TYPE   _type
                  , uint32_t                  _num_descriptors
                  , uint32_t                  _base_shader_register
                  , uint32_t                  _register_space       = 0
                  , buma3d::DESCRIPTOR_FLAGS  _flags                = buma3d::DEPENDENCY_FLAG_NONE)
    {
        BUMA_ASSERT(ranges);
        ranges->emplace_back(buma3d::DESCRIPTOR_RANGE{ _type, _num_descriptors, _base_shader_register, _register_space, _flags });
        parameter.descriptor_table.num_descriptor_ranges = (uint32_t)ranges->size();
        parameter.descriptor_table.descriptor_ranges     = ranges->data();
    }

    const buma3d::ROOT_PARAMETER& Get()
    {
        return parameter;
    }

private:
    buma3d::ROOT_PARAMETER                                  parameter;
    std::shared_ptr<std::vector<buma3d::DESCRIPTOR_RANGE>>  ranges;

};

class RootSignatureDesc
{
public:
    RootSignatureDesc()
        : desc              {}
        , parameters        {}
        , b3d_parameters    {}
        , static_samplers   {}
        , register_shifts   {}
    {}
    ~RootSignatureDesc()
    {
    }

    void Reset()
    {
        desc = {};
        parameters.clear();
        static_samplers.clear();
    }

    RootParameter& AddNewRootParameter() { return parameters.emplace_back(); }
    void AddRootParameter(RootParameter&&       _parameter) { parameters.emplace_back(std::move(_parameter)); }
    void AddRootParameter(const RootParameter&  _parameter) { parameters.emplace_back(_parameter); }

    void AddStaticSampler(buma3d::STATIC_SAMPLER&&      _static_sampler) { static_samplers.emplace_back(std::move(_static_sampler)); }
    void AddStaticSampler(const buma3d::STATIC_SAMPLER& _static_sampler) { static_samplers.emplace_back(_static_sampler); }
    void AddStaticSampler(uint32_t _shader_register, uint32_t _register_space, buma3d::ISamplerView* _sampler, buma3d::SHADER_VISIBILITY _shader_visibility = buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE)
    { static_samplers.emplace_back(buma3d::STATIC_SAMPLER{ _shader_register, _register_space, _shader_visibility, _sampler }); }

    void SetRegisterShift(buma3d::SHADER_REGISTER_SHIFT&&      _shift) { register_shifts.emplace_back(std::move(_shift)); }
    void SetRegisterShift(const buma3d::SHADER_REGISTER_SHIFT& _shift) { register_shifts.emplace_back(_shift); }
    void SetRegisterShift(buma3d::SHADER_REGISTER_TYPE _type, uint32_t _register_shift, uint32_t _register_space)
    { register_shifts.emplace_back(buma3d::SHADER_REGISTER_SHIFT{ _type, _register_shift,_register_space }); }

    const buma3d::ROOT_SIGNATURE_DESC& Get(buma3d::ROOT_SIGNATURE_FLAGS _flags, buma3d::RAY_TRACING_SHADER_VISIBILITY_FLAGS _raytracing_shader_visibilities = buma3d::RAY_TRACING_SHADER_VISIBILITY_FLAG_NONE)
    {
        b3d_parameters.resize(parameters.size());
        size_t cnt = 0;
        for (auto& i : parameters)
            b3d_parameters.data()[cnt++] = i.Get();

        desc.flags                          = _flags;
        desc.raytracing_shader_visibilities = _raytracing_shader_visibilities;
        desc.num_parameters                 = (uint32_t)b3d_parameters.size();
        desc.parameters                     =           b3d_parameters.data();
        desc.num_static_samplers            = (uint32_t)static_samplers.size();
        desc.static_samplers                =           static_samplers.data();
        desc.num_register_shifts            = (uint32_t)register_shifts.size();
        desc.register_shifts                =           register_shifts.data();
        return desc;
    }

private:
    buma3d::ROOT_SIGNATURE_DESC                 desc;
    std::vector<RootParameter>                  parameters;
    std::vector<buma3d::ROOT_PARAMETER>         b3d_parameters;
    std::vector<buma3d::STATIC_SAMPLER>         static_samplers;
    std::vector<buma3d::SHADER_REGISTER_SHIFT>  register_shifts;

};

#pragma endregion root signature

#pragma region pipeline layout

class DescriptorSetLayoutDesc
{
public:
    DescriptorSetLayoutDesc()
        : desc      {}
        , bindings  {}
    {}
    DescriptorSetLayoutDesc(uint32_t _num_bindings_reserve)
        : desc      {}
        , bindings  { _num_bindings_reserve }
    {}
    DescriptorSetLayoutDesc(const DescriptorSetLayoutDesc& _c)
        : desc      { _c.desc }
        , bindings  { _c.bindings }
    {
        Finalize();
    }
    DescriptorSetLayoutDesc& operator=(const DescriptorSetLayoutDesc& _c)
    {
        desc      = _c.desc;
        bindings  = _c.bindings;
        Finalize();
        return *this;
    }
    ~DescriptorSetLayoutDesc()
    {}

    DescriptorSetLayoutDesc& Reset()
    {
        desc = {};
        for (auto& i : bindings) i = {};
        return *this;
    }

    DescriptorSetLayoutDesc& AddNewBinding(  buma3d::DESCRIPTOR_TYPE     _descriptor_type
                                           , uint32_t                    _base_shader_register
                                           , uint32_t                    _num_descriptors   = 1
                                           , buma3d::SHADER_VISIBILITY   _shader_visibility = buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE
                                           , buma3d::DESCRIPTOR_FLAGS    _flags             = buma3d::DEPENDENCY_FLAG_NONE)
    {
        Resize(desc.num_bindings + 1);
        auto&& b = bindings.data()[desc.num_bindings++];
        b.descriptor_type      = _descriptor_type;
        b.base_shader_register = _base_shader_register;
        b.num_descriptors      = _num_descriptors;
        b.shader_visibility    = _shader_visibility;
        b.flags                = _flags;
        b.static_sampler       = nullptr;
        return *this;
    }
    DescriptorSetLayoutDesc& AddNewBinding(uint32_t                     _base_shader_register
                                           , buma3d::ISamplerView*      _static_sampler
                                           , buma3d::SHADER_VISIBILITY  _shader_visibility = buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE)
    {
        Resize(desc.num_bindings + 1);
        auto&& b = bindings.data()[desc.num_bindings++];
        b.descriptor_type      = buma3d::DESCRIPTOR_TYPE_SAMPLER;
        b.base_shader_register = _base_shader_register;
        b.num_descriptors      = 1;
        b.shader_visibility    = _shader_visibility;
        b.flags                = buma3d::DEPENDENCY_FLAG_NONE;
        b.static_sampler       = _static_sampler;
        return *this;
    }
    DescriptorSetLayoutDesc& SetFlags(buma3d::DESCRIPTOR_SET_LAYOUT_FLAGS _flags)
    {
        desc.flags = _flags;
        return *this;
    }
    void Finalize()
    {
        desc.bindings = bindings.data();
    }

    const buma3d::DESCRIPTOR_SET_LAYOUT_DESC& Get() const { return desc; }

private:
    void Resize(uint32_t _num_bindings)
    {
        if (_num_bindings > bindings.size())
        {
            bindings.resize(_num_bindings);
        }
    }

private:
    buma3d::DESCRIPTOR_SET_LAYOUT_DESC                  desc;
    std::vector<buma3d::DESCRIPTOR_SET_LAYOUT_BINDING>  bindings;

};

class PipelineLayoutDesc
{
public:
    PipelineLayoutDesc()
        : desc              {}
        , set_layouts       {}
        , push_constants    {}
    {}
    PipelineLayoutDesc(uint32_t _num_set_layouts_reserve, uint32_t _num_push_descriptors_reserve)
        : desc              {}
        , set_layouts       { _num_set_layouts_reserve }
        , push_constants    { _num_push_descriptors_reserve }
    {}
    PipelineLayoutDesc(const PipelineLayoutDesc& _c)
        : desc              { _c.desc }
        , set_layouts       { _c.set_layouts }
        , push_constants    { _c.push_constants }
    {
        Finalize();
    }
    PipelineLayoutDesc& operator=(const PipelineLayoutDesc& _c)
    {
        desc              = _c.desc;
        set_layouts       = _c.set_layouts;
        push_constants    = _c.push_constants;
        Finalize();
        return *this;
    }
    ~PipelineLayoutDesc()
    {}

    PipelineLayoutDesc& Reset()
    {
        desc = {};
        for (auto& i : set_layouts)    i = nullptr;
        for (auto& i : push_constants) i = {};
        return *this;
    }
    PipelineLayoutDesc& SetNumLayouts(uint32_t _num_descriptor_set_layouts)
    {
        desc.num_set_layouts = _num_descriptor_set_layouts;
        if (_num_descriptor_set_layouts > (uint32_t)set_layouts.size())
            set_layouts.resize(_num_descriptor_set_layouts);
        return *this;
    }
    PipelineLayoutDesc& SetLayout(uint32_t _descriptor_set_layout_index, buma3d::IDescriptorSetLayout* _layout)
    {
        set_layouts.data()[_descriptor_set_layout_index] = _layout;
        return *this;
    }
    PipelineLayoutDesc& SetLayouts(uint32_t _offset, const std::vector<buma3d::IDescriptorSetLayout*>& _layouts)
    {
        auto set_layouts_data = set_layouts.data();
        for (auto& i : _layouts)
            set_layouts_data[_offset++] = i;

        return *this;
    }
    PipelineLayoutDesc& SetLayouts(uint32_t _offset, const std::vector<buma3d::util::Ptr <buma3d::IDescriptorSetLayout >>& _layouts)
    {
        auto set_layouts_data = set_layouts.data();
        for (auto& i : _layouts)
            set_layouts_data[_offset++] = i.Get();

        return *this;
    }
    PipelineLayoutDesc& AddNewPushConstantParameter(  uint32_t                  _num_32bit_values
                                                    , uint32_t                  _shader_register
                                                    , buma3d::SHADER_VISIBILITY _visibility     = buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE
                                                    , uint32_t                  _register_space = 0)
    {
        if (desc.num_push_constants + 1 > (uint32_t)push_constants.size())
            push_constants.resize(desc.num_push_constants + 1);

        auto&& p = push_constants.data()[desc.num_push_constants++];
        p.visibility       = _visibility;
        p.shader_register  = _shader_register;
        p.register_space   = _register_space;
        p.num_32bit_values = _num_32bit_values;

        return *this;
    }
    PipelineLayoutDesc& SetFlags(buma3d::PIPELINE_LAYOUT_FLAGS _flags)
    {
        desc.flags = _flags;
        return *this;
    }
    void Finalize()
    {
        desc.set_layouts = set_layouts.data();
        desc.push_constants = push_constants.data();
    }

    const buma3d::PIPELINE_LAYOUT_DESC& Get() const { return desc; }

private:
    buma3d::PIPELINE_LAYOUT_DESC                    desc;
    std::vector<buma3d::IDescriptorSetLayout*>      set_layouts;
    std::vector<buma3d::PUSH_CONSTANT_PARAMETER>    push_constants;

};

#pragma endregion pipeline layout

#pragma region deprecated descriptor set update

class WriteDescriptorRange
{
public:
    WriteDescriptorRange()
        : range     {}
        , src_views {}
    {
        src_views = std::make_shared<std::vector<buma3d::IView*>>();
    }

    ~WriteDescriptorRange()
    {
    }

    WriteDescriptorRange& SetDstRange(uint32_t _dst_range_index, uint32_t _dst_first_array_element, uint32_t _num_descriptors)
    {
        range.dst_range_index           = _dst_range_index;
        range.dst_first_array_element   = _dst_first_array_element;
        range.num_descriptors           = _num_descriptors;
        src_views->resize(_num_descriptors);
        return *this;
    }

    template<typename T>
    WriteDescriptorRange& SetSrcView(uint32_t _index, T* _view)
    {
        return SetSrcView(_index, static_cast<buma3d::IView*>(_view));
    }
    template<>
    WriteDescriptorRange& SetSrcView(uint32_t _index, buma3d::IView* _view)
    {
        src_views->at(_index) = _view;
        return *this;
    }
    WriteDescriptorRange& SetSrcViews(uint32_t _offset, const std::initializer_list<buma3d::IView*>& _views)
    {
        size_t idx = _offset;
        for (auto& i : _views)
            src_views->at(idx) = i;

        return *this;
    }

    const buma3d::WRITE_DESCRIPTOR_RANGE& Get()
    {
        range.num_descriptors   = (uint32_t)src_views->size();
        range.src_views         =           src_views->data();
        return range;
    }

private:
    buma3d::WRITE_DESCRIPTOR_RANGE                  range;
    std::shared_ptr<std::vector<buma3d::IView*>>    src_views;

};

class WriteDescriptorTable
{
public:
    WriteDescriptorTable()
        : tables        {}
        , ranges        {}
        , b3d_ranges    {}
    {
        ranges = std::make_shared<std::vector<WriteDescriptorRange>>();
        b3d_ranges = std::make_shared<std::vector<buma3d::WRITE_DESCRIPTOR_RANGE>>();
    }

    ~WriteDescriptorTable()
    {
    }

    WriteDescriptorRange& AddNewWriteDescriptorRange()
    {
        return ranges->emplace_back();
    }

    void Finalize(uint32_t _dst_root_parameter_index)
    {
        tables.dst_root_parameter_index = _dst_root_parameter_index;
        b3d_ranges->resize(ranges->size());
        size_t cnt = 0;
        for (auto& i : *ranges)
            b3d_ranges->data()[cnt++] = i.Get();
    }
    const buma3d::WRITE_DESCRIPTOR_TABLE& Get()
    {
        tables.num_ranges   = (uint32_t)b3d_ranges->size();
        tables.ranges       =           b3d_ranges->data();
        return tables;
    }

private:
    buma3d::WRITE_DESCRIPTOR_TABLE                                  tables;
    std::shared_ptr<std::vector<WriteDescriptorRange>>              ranges;
    std::shared_ptr<std::vector<buma3d::WRITE_DESCRIPTOR_RANGE>>    b3d_ranges;

};

class WriteDescriptorSet0
{
public:
    WriteDescriptorSet0()
        : write_set             {}
        , tables                {}
        , b3d_tables            {}
        , dynamic_descriptors   {}
    {
        tables              = std::make_shared<std::vector<WriteDescriptorTable>>();
        b3d_tables          = std::make_shared<std::vector<buma3d::WRITE_DESCRIPTOR_TABLE>>();
        dynamic_descriptors = std::make_shared<std::vector<buma3d::WRITE_DYNAMIC_DESCRIPTOR0>>();
    }

    ~WriteDescriptorSet0()
    {
    }

    WriteDescriptorTable& AddNewWriteDescriptorTable()
    {
        return tables->emplace_back();
    }
    WriteDescriptorSet0& AddWriteDynamicDescriptor(  uint32_t          _dst_root_parameter_index
                                                   , buma3d::IView*    _src_view
                                                   , uint64_t          _src_view_buffer_offset)
    {
        dynamic_descriptors->emplace_back(buma3d::WRITE_DYNAMIC_DESCRIPTOR0{ _dst_root_parameter_index, _src_view, _src_view_buffer_offset });
        return *this;
    }

    void Finalize(buma3d::IDescriptorSet0* _dst_set)
    {
        write_set.dst_set = _dst_set;

        b3d_tables->resize(tables->size());
        size_t cnt = 0;
        for (auto& i : *tables)
            b3d_tables->data()[cnt++] = i.Get();
    }
    const buma3d::WRITE_DESCRIPTOR_SET0& Get()
    {
        write_set.num_descriptor_tables   = (uint32_t)b3d_tables->size();
        write_set.descriptor_tables       =           b3d_tables->data();
        write_set.num_dynamic_descriptors = (uint32_t)dynamic_descriptors->size();
        write_set.dynamic_descriptors     =           dynamic_descriptors->data();
        return write_set;
    }

private:
    buma3d::WRITE_DESCRIPTOR_SET0                                   write_set;
    std::shared_ptr<std::vector<WriteDescriptorTable>>              tables;
    std::shared_ptr<std::vector<buma3d::WRITE_DESCRIPTOR_TABLE>>    b3d_tables;
    std::shared_ptr<std::vector<buma3d::WRITE_DYNAMIC_DESCRIPTOR0>> dynamic_descriptors;

};

class CopyDescriptorTable
{
public:
    CopyDescriptorTable()
        : table             {}
        , src_ranges        {}
        , dst_ranges        {}
        , num_descriptors   {}
    {
        src_ranges      = std::make_shared<std::vector<buma3d::COPY_DESCRIPTOR_RANGE>>();
        dst_ranges      = std::make_shared<std::vector<buma3d::COPY_DESCRIPTOR_RANGE>>();
        num_descriptors = std::make_shared<std::vector<uint32_t>                     >();
    }

    ~CopyDescriptorTable()
    {
    }

    CopyDescriptorTable& SetRootParameterIndex(uint32_t _src_root_parameter_index, uint32_t _dst_root_parameter_index)
    {
        table.src_root_parameter_index = _src_root_parameter_index;
        table.dst_root_parameter_index = _dst_root_parameter_index;
        return *this;
    }
    CopyDescriptorTable& AddRange(  const buma3d::COPY_DESCRIPTOR_RANGE& _src_ranges
                                  , const buma3d::COPY_DESCRIPTOR_RANGE& _dst_ranges
                                  , uint32_t                             _num_descriptors)
    {
        src_ranges      ->emplace_back(_src_ranges);
        dst_ranges      ->emplace_back(_dst_ranges);
        num_descriptors ->emplace_back(_num_descriptors);
        return *this;
    }
    CopyDescriptorTable& AddRange(  uint32_t _src_range_index, uint32_t _src_first_array_element
                                  , uint32_t _dst_range_index, uint32_t _dst_first_array_element
                                  , uint32_t _num_descriptors)
    {
        src_ranges      ->emplace_back(buma3d::COPY_DESCRIPTOR_RANGE{ _src_range_index, _src_first_array_element });
        dst_ranges      ->emplace_back(buma3d::COPY_DESCRIPTOR_RANGE{ _dst_range_index, _dst_first_array_element });
        num_descriptors ->emplace_back(_num_descriptors);
        return *this;
    }
    const buma3d::COPY_DESCRIPTOR_TABLE& Get()
    {
        table.num_ranges        = (uint32_t)src_ranges->size();
        table.src_ranges        = src_ranges->data();
        table.dst_ranges        = dst_ranges->data();
        table.num_descriptors   = num_descriptors->data();
        return table;
    }

private:
    buma3d::COPY_DESCRIPTOR_TABLE                                table;
    std::shared_ptr<std::vector<buma3d::COPY_DESCRIPTOR_RANGE>>  src_ranges;
    std::shared_ptr<std::vector<buma3d::COPY_DESCRIPTOR_RANGE>>  dst_ranges;
    std::shared_ptr<std::vector<uint32_t>>                       num_descriptors;

};

class CopyDescriptorSet0
{
public:
    CopyDescriptorSet0()
        : copy_set              {}
        , tables                {}
        , b3d_tables            {}
        , dynamic_descriptors   {}
    {
        tables                = std::make_shared<std::vector<CopyDescriptorTable>              >();
        b3d_tables            = std::make_shared<std::vector<buma3d::COPY_DESCRIPTOR_TABLE>    >();
        dynamic_descriptors   = std::make_shared<std::vector<buma3d::COPY_DYNAMIC_DESCRIPTOR>  >();
    }

    ~CopyDescriptorSet0()
    {
    }

    CopyDescriptorTable& AddNewCopyTable()
    {
        return tables->emplace_back();
    }
    CopyDescriptorSet0& AddCopyDynamicDescriptor(uint32_t _src_root_parameter_index, uint32_t _dst_root_parameter_index)
    {
        dynamic_descriptors->emplace_back(buma3d::COPY_DYNAMIC_DESCRIPTOR{ _src_root_parameter_index, _dst_root_parameter_index });
        return *this;
    }
    void Finalize(buma3d::IDescriptorSet0* _src_set, buma3d::IDescriptorSet0* _dst_set)
    {
        b3d_tables->resize(tables->size());
        size_t cnt = 0;
        for (auto& i : *tables)
            b3d_tables->data()[cnt++] = i.Get();

        copy_set.src_set = _src_set;
        copy_set.dst_set = _dst_set;
    }
    const buma3d::COPY_DESCRIPTOR_SET0& Get()
    {
        copy_set.num_descriptor_tables   = (uint32_t)b3d_tables->size();
        copy_set.descriptor_tables       =           b3d_tables->data();
        copy_set.num_dynamic_descriptors = (uint32_t)dynamic_descriptors->size();
        copy_set.dynamic_descriptors     =           dynamic_descriptors->data();
        return copy_set;
    }

private:
    buma3d::COPY_DESCRIPTOR_SET0                                    copy_set;
    std::shared_ptr<std::vector<CopyDescriptorTable>>               tables;
    std::shared_ptr<std::vector<buma3d::COPY_DESCRIPTOR_TABLE>>     b3d_tables;
    std::shared_ptr<std::vector<buma3d::COPY_DYNAMIC_DESCRIPTOR>>   dynamic_descriptors;

};

class UpdateDescriptorSetDesc0
{
public:
    UpdateDescriptorSetDesc0()
        : update_desc               {}
        , write_descriptor_sets     {}
        , copy_descriptor_sets      {}
        , b3d_write_descriptor_sets {}
        , b3d_copy_descriptor_sets  {}
    {
        write_descriptor_sets     = std::make_shared<std::vector<WriteDescriptorSet0>>();
        copy_descriptor_sets      = std::make_shared<std::vector<CopyDescriptorSet0>>();
        b3d_write_descriptor_sets = std::make_shared<std::vector<buma3d::WRITE_DESCRIPTOR_SET0>>();
        b3d_copy_descriptor_sets  = std::make_shared<std::vector<buma3d::COPY_DESCRIPTOR_SET0>>();
    }

    ~UpdateDescriptorSetDesc0()
    {
    }

    WriteDescriptorSet0& AddNewWriteDescriptorSets()
    {
        return write_descriptor_sets->emplace_back();
    }
    CopyDescriptorSet0& AddNewCopyDescriptorSets()
    {
        return copy_descriptor_sets->emplace_back();
    }

    void Reset()
    {
        update_desc = {};
        write_descriptor_sets       ->clear();
        copy_descriptor_sets        ->clear();
        b3d_write_descriptor_sets   ->clear();
        b3d_copy_descriptor_sets    ->clear();
    }

    void Finalize()
    {
        b3d_write_descriptor_sets->resize(write_descriptor_sets->size());
        b3d_copy_descriptor_sets->resize(copy_descriptor_sets->size());

        size_t cnt = 0;
        for (auto& i : *write_descriptor_sets)
            b3d_write_descriptor_sets->data()[cnt++] = i.Get();

        cnt = 0;
        for (auto& i : *copy_descriptor_sets)
            b3d_copy_descriptor_sets->data()[cnt++] = i.Get();
    }

    const buma3d::UPDATE_DESCRIPTOR_SET_DESC0& Get()
    {
        update_desc.num_write_descriptor_sets   = (uint32_t)b3d_write_descriptor_sets->size();
        update_desc.write_descriptor_sets       =           b3d_write_descriptor_sets->data();
        update_desc.num_copy_descriptor_sets    = (uint32_t)b3d_copy_descriptor_sets->size();
        update_desc.copy_descriptor_sets        =           b3d_copy_descriptor_sets->data();
        return update_desc;
    }

private:
    buma3d::UPDATE_DESCRIPTOR_SET_DESC0                         update_desc;
    std::shared_ptr<std::vector<WriteDescriptorSet0>>           write_descriptor_sets;
    std::shared_ptr<std::vector<CopyDescriptorSet0>>            copy_descriptor_sets;
    std::shared_ptr<std::vector<buma3d::WRITE_DESCRIPTOR_SET0>> b3d_write_descriptor_sets;
    std::shared_ptr<std::vector<buma3d::COPY_DESCRIPTOR_SET0>>  b3d_copy_descriptor_sets;

};

#pragma endregion deprecated descriptor set update

#pragma region descriptor set update

class WriteDescriptorBinding;
class WriteDescriptorSet;
class CopyDescriptorBinding;
class CopyDescriptorSet;
class UpdateDescriptorSetDesc;

class WriteDescriptorBinding
{
public:
    WriteDescriptorBinding(WriteDescriptorSet* _parent)
        : parent    { _parent }
        , binding   {}
        , src_views {}
    {}
    WriteDescriptorBinding(const WriteDescriptorBinding& _c)
        : parent    { _c.parent }
        , binding   { _c.binding }
        , src_views { _c.src_views }
    {
        if (parent)
            Finalize();
    }
    WriteDescriptorBinding& operator=(const WriteDescriptorBinding& _c)
    {
        parent    = _c.parent;
        binding   = _c.binding;
        src_views = _c.src_views;
        if (parent)
            Finalize();
        return *this;
    }
    ~WriteDescriptorBinding() {}

    void SetParent(WriteDescriptorSet* _parent)
    {
        parent = _parent;
    }

    WriteDescriptorBinding& Reset()
    {
        binding = {};
        for (auto& i : src_views) i = nullptr;
        return *this;
    }
    WriteDescriptorBinding& SetDstBinding(uint32_t _dst_binding_index, uint32_t _dst_first_array_element = 0)
    {
        binding.dst_binding_index = _dst_binding_index;
        binding.dst_first_array_element = _dst_first_array_element;
        return *this;
    }
    WriteDescriptorBinding& SetNumDescriptors(uint32_t _num_descriptors)
    {
        binding.num_descriptors = _num_descriptors;
        if (_num_descriptors > (uint32_t)src_views.size())
            src_views.resize(_num_descriptors);
        return *this;
    }
    template<typename T>
    WriteDescriptorBinding& SetSrcView(uint32_t _index, T* _view)
    {
        return SetSrcView(_index, static_cast<buma3d::IView*>(_view));
    }
    template<>
    WriteDescriptorBinding& SetSrcView(uint32_t _index, buma3d::IView* _view)
    {
        src_views.data()[_index] = _view;
        return *this;
    }
    WriteDescriptorBinding& SetSrcViews(uint32_t _offset, const std::initializer_list<buma3d::IView*>& _views)
    {
        size_t idx = _offset;
        for (auto& i : _views)
            src_views.data()[idx++] = i;
        return *this;
    }
    WriteDescriptorBinding& SetSrcViews(uint32_t _offset, uint32_t _num_descriptors, buma3d::IView*const * _views)
    {
        for (size_t i = 0; i < _num_descriptors; i++)
            src_views.data()[_offset + i] = _views[i];
        return *this;
    }
    WriteDescriptorSet& Finalize()
    {
        binding.src_views = src_views.data();
        return *parent;
    }

    const buma3d::WRITE_DESCRIPTOR_BINDING& Get() const { return binding; }

private:
    WriteDescriptorSet*                 parent;
    buma3d::WRITE_DESCRIPTOR_BINDING    binding;
    std::vector<buma3d::IView*>         src_views;

};

class WriteDescriptorSet
{
public:
    WriteDescriptorSet(UpdateDescriptorSetDesc* _parent)
        : parent                { _parent }
        , write_set             {}
        , bindings              {}
        , b3d_bindings          {}
        , dynamic_descriptors   {}
    {}
    WriteDescriptorSet(const WriteDescriptorSet& _c)
        : parent                { _c.parent }
        , write_set             { _c.write_set }
        , bindings              { _c.bindings }
        , b3d_bindings          { _c.b3d_bindings }
        , dynamic_descriptors   { _c.dynamic_descriptors }
    {
        for (auto& i : bindings) i.SetParent(this);
        if (parent)
            Finalize();
    }
    WriteDescriptorSet& operator=(const WriteDescriptorSet& _c)
    {
        parent                = _c.parent;
        write_set             = _c.write_set;
        bindings              = _c.bindings;
        b3d_bindings          = _c.b3d_bindings;
        dynamic_descriptors   = _c.dynamic_descriptors;
        for (auto& i : bindings) i.SetParent(this);
        if (parent)
            Finalize();
        return *this;
    }
    ~WriteDescriptorSet()
    {}

    void SetParent(UpdateDescriptorSetDesc* _parent)
    {
        parent = _parent;
        for (auto& i : bindings) i.SetParent(this);
    }

    WriteDescriptorSet& Reset()
    {
        write_set = {};
        for (auto& i : bindings)            i.Reset();
        for (auto& i : b3d_bindings)        i = {};
        for (auto& i : dynamic_descriptors) i = {};
        return *this;
    }
    WriteDescriptorSet& SetDst(buma3d::IDescriptorSet* _dst_set)
    {
        write_set.dst_set = _dst_set;
        return *this;
    }
    WriteDescriptorBinding& AddNewWriteDescriptorBinding()
    {
        Resize(write_set.num_bindings + 1);
        return bindings.data()[write_set.num_bindings++];
    }
    WriteDescriptorSet& AddWriteDynamicDescriptor(  uint32_t          _dst_binding_index
                                                  , buma3d::IView*    _src_view
                                                  , int64_t           _src_view_buffer_offset = 0)
    {
        ResizeDynamicBindings(write_set.num_dynamic_bindings + 1);
        dynamic_descriptors.data()[write_set.num_dynamic_bindings++] = buma3d::WRITE_DYNAMIC_DESCRIPTOR_BINDING{ _dst_binding_index, _src_view, _src_view_buffer_offset };
        return *this;
    }
    UpdateDescriptorSetDesc& Finalize()
    {
        Finalize<void>();
        return *parent;
    }
    template<typename Void, std::enable_if_t<std::is_same_v<Void, void>, int> = 0>
    void Finalize()
    {
        auto bindings_data = bindings.data();
        auto b3d_bindings_data = b3d_bindings.data();
        for (uint32_t i = 0; i < write_set.num_bindings; i++)
            b3d_bindings_data[i] = bindings_data[i].Get();

        write_set.bindings         = b3d_bindings_data;
        write_set.dynamic_bindings = dynamic_descriptors.data();
    }

    const buma3d::WRITE_DESCRIPTOR_SET& Get() const { return write_set; }

private:
    void Resize(uint32_t _num_bindings)
    {
        if (_num_bindings > (uint32_t)bindings.size())
        {
            bindings.resize(_num_bindings, this);
            b3d_bindings.resize(_num_bindings);
        }
    }
    void ResizeDynamicBindings(uint32_t _num_bindings)
    {
        if (_num_bindings > (uint32_t)dynamic_descriptors.size())
        {
            dynamic_descriptors.resize(_num_bindings);
        }
    }

private:
    UpdateDescriptorSetDesc*                                parent;
    buma3d::WRITE_DESCRIPTOR_SET                            write_set;
    std::vector<WriteDescriptorBinding>                     bindings;
    std::vector<buma3d::WRITE_DESCRIPTOR_BINDING>           b3d_bindings;
    std::vector<buma3d::WRITE_DYNAMIC_DESCRIPTOR_BINDING>   dynamic_descriptors;

};

class CopyDescriptorBinding
{
public:
    CopyDescriptorBinding(CopyDescriptorSet* _parent)
        : parent  { _parent }
        , binding {}
    {}
    CopyDescriptorBinding(const CopyDescriptorBinding& _c)
        : parent  { _c.parent }
        , binding { _c.binding }
    {
        if (parent)
            Finalize();
    }
    CopyDescriptorBinding& operator=(const CopyDescriptorBinding& _c)
    {
        parent  = _c.parent;
        binding = _c.binding;
        if (parent)
            Finalize();
        return *this;
    }
    ~CopyDescriptorBinding() {}

    void SetParent(CopyDescriptorSet* _parent)
    {
        parent = _parent;
    }

    CopyDescriptorBinding& Reset()
    {
        binding = {};
        return *this;
    }
    CopyDescriptorBinding& SetSrcBinding(uint32_t _src_binding_index, uint32_t _src_first_array_element = 0)
    {
        binding.src_binding_index       = _src_binding_index;
        binding.src_first_array_element = _src_first_array_element;
        return *this;
    }
    CopyDescriptorBinding& SetDstBinding(uint32_t _dst_binding_index, uint32_t _dst_first_array_element = 0)
    {
        binding.dst_binding_index       = _dst_binding_index;
        binding.dst_first_array_element = _dst_first_array_element;
        return *this;
    }
    CopyDescriptorBinding& SetNumDescriptors(uint32_t _num_descriptors)
    {
        binding.num_descriptors = _num_descriptors;
        return *this;
    }
    CopyDescriptorSet& Finalize() { return *parent; }

    const buma3d::COPY_DESCRIPTOR_BINDING& Get() const { return binding; }

private:
    CopyDescriptorSet*              parent;
    buma3d::COPY_DESCRIPTOR_BINDING binding;

};

class CopyDescriptorSet
{
public:
    CopyDescriptorSet(UpdateDescriptorSetDesc* _parent)
        : parent        { _parent }
        , copy_set      {}
        , bindings      {}
        , b3d_bindings  {}
    {}
    CopyDescriptorSet(const CopyDescriptorSet& _c)
        : parent        { _c.parent }
        , copy_set      { _c.copy_set }
        , bindings      { _c.bindings }
        , b3d_bindings  { _c.b3d_bindings }
    {
        for (auto& i : bindings) i.SetParent(this);
        if (parent)
            Finalize();
    }
    CopyDescriptorSet& operator=(const CopyDescriptorSet& _c)
    {
        parent        = _c.parent;
        copy_set      = _c.copy_set;
        bindings      = _c.bindings;
        b3d_bindings  = _c.b3d_bindings;
        for (auto& i : bindings) i.SetParent(this);
        if (parent)
            Finalize();
        return *this;
    }
    ~CopyDescriptorSet() {}

    void SetParent(UpdateDescriptorSetDesc* _parent)
    {
        parent = _parent;
        for (auto& i : bindings) i.SetParent(this);
    }

    CopyDescriptorSet& Reset()
    {
        copy_set = {};
        for (auto& i : bindings) i.Reset();
        for (auto& i : b3d_bindings) i = {};
        return *this;
    }
    CopyDescriptorSet& SetSrc(buma3d::IDescriptorSet* _src_set)
    {
        copy_set.src_set = _src_set;
        return *this;
    }
    CopyDescriptorSet& SetDst(buma3d::IDescriptorSet* _dst_set)
    {
        copy_set.dst_set = _dst_set;
        return *this;
    }
    CopyDescriptorBinding& AddNewCopyBinding()
    {
        Resize(copy_set.num_bindings + 1);
        return bindings.data()[copy_set.num_bindings++];
    }
    UpdateDescriptorSetDesc& Finalize()
    {
        auto bindings_data     = bindings.data();
        auto b3d_bindings_data = b3d_bindings.data();
        for (uint32_t i = 0; i < copy_set.num_bindings; i++)
            b3d_bindings_data[i] = bindings_data[i].Get();

        copy_set.bindings = b3d_bindings_data;
        return *parent;
    }

    const buma3d::COPY_DESCRIPTOR_SET& Get() const { return copy_set; }

private:
    void Resize(uint32_t _num_bindings)
    {
        if (_num_bindings > bindings.size())
        {
            bindings    .resize(_num_bindings, this);
            b3d_bindings.resize(_num_bindings);
        }
    }

private:
    UpdateDescriptorSetDesc*                        parent;
    buma3d::COPY_DESCRIPTOR_SET                     copy_set;
    std::vector<CopyDescriptorBinding>              bindings;
    std::vector<buma3d::COPY_DESCRIPTOR_BINDING>    b3d_bindings;

};

class UpdateDescriptorSetDesc
{
public:
    UpdateDescriptorSetDesc()
        : update_desc               {}
        , write_descriptor_sets     {}
        , copy_descriptor_sets      {}
        , b3d_write_descriptor_sets {}
        , b3d_copy_descriptor_sets  {}
    {}
    UpdateDescriptorSetDesc(uint32_t _num_write_descriptor_set_reserve, uint32_t _num_copy_descriptor_set_reserve)
        : update_desc               {}
        , write_descriptor_sets     (_num_write_descriptor_set_reserve, this)
        , copy_descriptor_sets      (_num_copy_descriptor_set_reserve , this)
        , b3d_write_descriptor_sets { _num_write_descriptor_set_reserve }
        , b3d_copy_descriptor_sets  { _num_copy_descriptor_set_reserve }
    {}
    UpdateDescriptorSetDesc(const UpdateDescriptorSetDesc& _c)
        : update_desc               { _c.update_desc }
        , write_descriptor_sets     { _c.write_descriptor_sets }
        , copy_descriptor_sets      { _c.copy_descriptor_sets }
        , b3d_write_descriptor_sets { _c.b3d_write_descriptor_sets }
        , b3d_copy_descriptor_sets  { _c.b3d_copy_descriptor_sets }
    {
        for (auto& i : write_descriptor_sets) i.SetParent(this);
        for (auto& i : copy_descriptor_sets) i.SetParent(this);
        Finalize();
    }
    UpdateDescriptorSetDesc& operator=(const UpdateDescriptorSetDesc& _c)
    {
        update_desc               = _c.update_desc;
        write_descriptor_sets     = _c.write_descriptor_sets;
        copy_descriptor_sets      = _c.copy_descriptor_sets;
        b3d_write_descriptor_sets = _c.b3d_write_descriptor_sets;
        b3d_copy_descriptor_sets  = _c.b3d_copy_descriptor_sets;
        for (auto& i : write_descriptor_sets) i.SetParent(this);
        for (auto& i : copy_descriptor_sets) i.SetParent(this);
        Finalize();
        return *this;
    }
    ~UpdateDescriptorSetDesc()
    {}

    UpdateDescriptorSetDesc& Reset()
    {
        update_desc = {};
        for (auto& i : write_descriptor_sets)     i.Reset();
        for (auto& i : copy_descriptor_sets)      i.Reset();
        for (auto& i : b3d_write_descriptor_sets) i = {};
        for (auto& i : b3d_copy_descriptor_sets)  i = {};
        return *this;
    }

    UpdateDescriptorSetDesc& AddWriteDescriptorSet(const WriteDescriptorSet& _write)
    {
        ResizeSets(write_descriptor_sets, b3d_write_descriptor_sets, update_desc.num_write_descriptor_sets + 1);
        (write_descriptor_sets.data()[update_desc.num_write_descriptor_sets++] = _write).SetParent(this);
        return *this;
    }
    WriteDescriptorSet& AddNewWriteDescriptorSet()
    {
        ResizeSets(write_descriptor_sets, b3d_write_descriptor_sets, update_desc.num_write_descriptor_sets + 1);
        return write_descriptor_sets.data()[update_desc.num_write_descriptor_sets++];
    }
    UpdateDescriptorSetDesc& AddCopyDescriptorSet(const CopyDescriptorSet& _copy)
    {
        ResizeSets(copy_descriptor_sets, b3d_copy_descriptor_sets, update_desc.num_copy_descriptor_sets + 1);
        (copy_descriptor_sets.data()[update_desc.num_copy_descriptor_sets++] = _copy).SetParent(this);
        return *this;
    }
    CopyDescriptorSet& AddNewCopyDescriptorSet()
    {
        ResizeSets(copy_descriptor_sets, b3d_copy_descriptor_sets, update_desc.num_copy_descriptor_sets + 1);
        return copy_descriptor_sets.data()[update_desc.num_copy_descriptor_sets++];
    }
    void Finalize()
    {
        auto write_sets_data        = write_descriptor_sets.data();
        auto copy_sets_data         = copy_descriptor_sets.data();
        auto b3d_write_sets_data    = b3d_write_descriptor_sets.data();
        auto b3d_copy_sets_data     = b3d_copy_descriptor_sets.data();

        for (uint32_t i = 0; i < update_desc.num_write_descriptor_sets; i++)
            b3d_write_sets_data[i] = write_sets_data[i].Get();

        for (uint32_t i = 0; i < update_desc.num_copy_descriptor_sets; i++)
            b3d_copy_sets_data[i] = copy_sets_data[i].Get();

        update_desc.write_descriptor_sets = b3d_write_sets_data;
        update_desc.copy_descriptor_sets  = b3d_copy_sets_data;
    }

    const buma3d::UPDATE_DESCRIPTOR_SET_DESC& Get() const { return update_desc; }

private:
    template <typename T, typename U> void ResizeSets(std::vector<T>& _target, std::vector<U>& _b3d_target, uint32_t _num_sets)
    {
        if (_num_sets > (uint32_t)_target.size())
        {
            _target.resize(_num_sets, this);
            _b3d_target.resize(_num_sets);
        }
    }

private:
    buma3d::UPDATE_DESCRIPTOR_SET_DESC          update_desc;
    std::vector<WriteDescriptorSet>             write_descriptor_sets;
    std::vector<CopyDescriptorSet>              copy_descriptor_sets;
    std::vector<buma3d::WRITE_DESCRIPTOR_SET>   b3d_write_descriptor_sets;
    std::vector<buma3d::COPY_DESCRIPTOR_SET>    b3d_copy_descriptor_sets;

};

#pragma endregion descriptor set update

#pragma region descriptor heap/pool

class DescriptorSizes
{
public:
    DescriptorSizes()
        : descriptor_sizes      {}
        , num_sizes             {}
        , total_multiply_count  {}
        , heap_sizes            {}
        , pool_sizes            {}
    {}
    DescriptorSizes(uint32_t _num_descriptor_sizes_reserve)
        : descriptor_sizes      {}
        , num_sizes             {}
        , total_multiply_count  {}
        , heap_sizes            { _num_descriptor_sizes_reserve }
        , pool_sizes            { _num_descriptor_sizes_reserve }
    {}
    DescriptorSizes(const DescriptorSizes& _c)
        : descriptor_sizes      { _c.descriptor_sizes }
        , num_sizes             { _c.num_sizes }
        , total_multiply_count  { _c.total_multiply_count }
        , heap_sizes            { _c.heap_sizes }
        , pool_sizes            { _c.pool_sizes }
    {
        Finalize();
    }
    DescriptorSizes& operator=(const DescriptorSizes& _c)
    {
        descriptor_sizes      = _c.descriptor_sizes;
        num_sizes             = _c.num_sizes;
        total_multiply_count  = _c.total_multiply_count;
        heap_sizes            = _c.heap_sizes;
        pool_sizes            = _c.pool_sizes;
        Finalize();
        return *this;
    }
    ~DescriptorSizes()
    {}

    DescriptorSizes& Reset()
    {
        num_sizes = 0;
        total_multiply_count = 0;
        descriptor_sizes.fill(0);
        for (auto& i : heap_sizes) i = {};
        for (auto& i : pool_sizes) i = {};
        return *this;
    }
    DescriptorSizes& IncrementSize(buma3d::DESCRIPTOR_TYPE _type, uint32_t _size, uint32_t _num_descriptors_multiply = 1)
    {
        total_multiply_count += _num_descriptors_multiply;
        descriptor_sizes[_type] += _size * _num_descriptors_multiply;
        return *this;
    }
    DescriptorSizes& IncrementSizes(const buma3d::DESCRIPTOR_SET_LAYOUT_DESC& _layout_desc, uint32_t _num_descriptors_multiply = 1)
    {
        total_multiply_count += _num_descriptors_multiply;
        for (uint32_t i = 0; i < _layout_desc.num_bindings; i++)
            descriptor_sizes[_layout_desc.bindings[i].descriptor_type] += _layout_desc.bindings[i].num_descriptors * _num_descriptors_multiply;
        return *this;
    }
    DescriptorSizes& IncrementSizes(buma3d::IDescriptorSetLayout* _layout, uint32_t _num_descriptors_multiply = 1)
    {
        return IncrementSizes(_layout->GetDesc(), _num_descriptors_multiply);
    }
    DescriptorSizes& DecrementSize(buma3d::DESCRIPTOR_TYPE _type, uint32_t _size, uint32_t _num_descriptors_multiply = 1)
    {
        total_multiply_count -= _num_descriptors_multiply;
        descriptor_sizes[_type] -= _size * _num_descriptors_multiply;
        return *this;
    }
    DescriptorSizes& DecrementSizes(const buma3d::DESCRIPTOR_SET_LAYOUT_DESC& _layout_desc, uint32_t _num_descriptors_multiply = 1)
    {
        total_multiply_count -= _num_descriptors_multiply;
        for (uint32_t i = 0; i < _layout_desc.num_bindings; i++)
            descriptor_sizes[_layout_desc.bindings[i].descriptor_type] -= _layout_desc.bindings[i].num_descriptors * _num_descriptors_multiply;
        return *this;
    }
    DescriptorSizes& DecrementSizes(buma3d::IDescriptorSetLayout* _layout, uint32_t _num_descriptors_multiply = 1)
    {
        return DecrementSizes(_layout->GetDesc(), _num_descriptors_multiply);
    }
    DescriptorSizes& Finalize()
    {
        num_sizes = CalcDescriptorSizes();
        ConvertSizes(num_sizes, heap_sizes);
        ConvertSizes(num_sizes, pool_sizes);
        return *this;
    }

    uint32_t                                                       GetMaxSetsByTotalMultiplyCount()     const { return total_multiply_count; }
    uint32_t                                                       GetNumPoolSizes()                    const { return CalcDescriptorSizes(); }
    const std::array<uint32_t, buma3d::DESCRIPTOR_TYPE_NUM_TYPES>& GetDescriptorSizes()                 const { return descriptor_sizes; }

    buma3d::DESCRIPTOR_HEAP_DESC GetAsHeapDesc(buma3d::DESCRIPTOR_HEAP_FLAGS _flags = buma3d::DESCRIPTOR_HEAP_FLAG_NONE, buma3d::NodeMask _node_mask = buma3d::B3D_DEFAULT_NODE_MASK) const
    {
        return buma3d::DESCRIPTOR_HEAP_DESC{
              _flags                        // buma3d::DESCRIPTOR_HEAP_FLAGS       flags;
            , num_sizes                     // uint32_t                            num_heap_sizes;
            , heap_sizes.data()             // const buma3d::DESCRIPTOR_HEAP_SIZE* heap_sizes;
            , _node_mask                    // buma3d::NodeMask                    node_mask;
        };
    }
    buma3d::DESCRIPTOR_POOL_DESC GetAsPoolDesc(buma3d::IDescriptorHeap* _heap, uint32_t _max_sets_allocation_count, buma3d::DESCRIPTOR_POOL_FLAGS _flags = buma3d::DESCRIPTOR_POOL_FLAG_NONE) const
    {
        return buma3d::DESCRIPTOR_POOL_DESC{
              _heap                         // buma3d::IDescriptorHeap*            heap;
            , _flags                        // buma3d::DESCRIPTOR_POOL_FLAGS       flags;
            , _max_sets_allocation_count    // uint32_t                            max_sets_allocation_count;
            , num_sizes                     // uint32_t                            num_pool_sizes;
            , pool_sizes.data()             // const buma3d::DESCRIPTOR_POOL_SIZE* pool_sizes;
        };
    }

private:
    uint32_t CalcDescriptorSizes() const 
    {
        uint32_t cnt = 0;
        for (auto& i : descriptor_sizes)
            if (i != 0) cnt++;
        return cnt;
    }
    template<typename T>
    void ConvertSizes(uint32_t _num_sizes, std::vector<T>& _target)
    {
        if (_num_sizes > (uint32_t)_target.size())
            _target.resize(_num_sizes);

        _num_sizes = 0;
        uint32_t type = (uint32_t)buma3d::DESCRIPTOR_TYPE_CBV;
        auto _target_data = _target.data();
        for (auto& i : descriptor_sizes)
        {
            if (i != 0)
            {
                auto&& s = _target_data[_num_sizes++];
                s.type = static_cast<buma3d::DESCRIPTOR_TYPE>(type);
                s.num_descriptors += i;
            }
            type++;
        }
    }

private:
    std::array<uint32_t, buma3d::DESCRIPTOR_TYPE_NUM_TYPES> descriptor_sizes;
    uint32_t                                                num_sizes; // 非ゼロサイズのディスクリプタサイズ数
    uint32_t                                                total_multiply_count;
    std::vector<buma3d::DESCRIPTOR_HEAP_SIZE>               heap_sizes;
    std::vector<buma3d::DESCRIPTOR_POOL_SIZE>               pool_sizes;

};

class DescriptorSetAllocateDesc
{
public:
    DescriptorSetAllocateDesc()
        : desc      {}
        , layouts   {}
        , dst_sets  {}
    {}
    DescriptorSetAllocateDesc(uint32_t _num_descriptor_sets_reserve)
        : desc      {}
        , layouts   { _num_descriptor_sets_reserve }
        , dst_sets  { _num_descriptor_sets_reserve }
    {}
    DescriptorSetAllocateDesc(const DescriptorSetAllocateDesc& _c)
        : desc      { _c.desc }
        , layouts   { _c.layouts }
        , dst_sets  { _c.dst_sets }
    {
        Finalize();
    }
    DescriptorSetAllocateDesc& operator=(const DescriptorSetAllocateDesc& _c)
    {
        desc      = _c.desc;
        layouts   = _c.layouts;
        dst_sets  = _c.dst_sets;
        Finalize();
        return *this;
    }
    ~DescriptorSetAllocateDesc()
    {
        Reset();
    }

    DescriptorSetAllocateDesc& Reset()
    {
        desc = {};
        for (auto& i : layouts)  i = nullptr;
        for (auto& i : dst_sets) util::SafeRelease(i);
        return *this;
    }
    DescriptorSetAllocateDesc& SetNumDescriptorSets(uint32_t _num_descriptor_sets)
    {
        desc.num_descriptor_sets = _num_descriptor_sets;
        if (_num_descriptor_sets > dst_sets.size())
        {
            layouts.resize(_num_descriptor_sets);
            dst_sets.resize(_num_descriptor_sets);
        }
        return *this;
    }
    DescriptorSetAllocateDesc& SetDescriptorSetLayout(uint32_t _index, buma3d::IDescriptorSetLayout* _layout)
    {
        layouts.data()[_index] = _layout;
        return *this;
    }
    DescriptorSetAllocateDesc& SetDescriptorSetLayouts(uint32_t _offset, const std::vector<buma3d::IDescriptorSetLayout*>& _layouts)
    {
        auto layouts_data = layouts.data();
        for (auto& i : _layouts)
            layouts_data[_offset++] = i;
        return *this;
    }
    DescriptorSetAllocateDesc& Finalize()
    {
        desc.set_layouts = layouts.data();
        return *this;
    }

    const buma3d::DESCRIPTOR_SET_ALLOCATE_DESC& Get() const 
    {
        return desc;
    }
    std::pair<uint32_t, buma3d::IDescriptorSet**> GetDst()
    {
        return { desc.num_descriptor_sets, dst_sets.data() };
    }
    std::vector<buma3d::util::Ptr<buma3d::IDescriptorSet>> ConvertDst()
    {
        std::vector<buma3d::util::Ptr<buma3d::IDescriptorSet>> result(desc.num_descriptor_sets);
        auto result_data = result.data();
        auto dst_sets_data = dst_sets.data();
        for (uint32_t i = 0; i < desc.num_descriptor_sets; i++)
            result_data[i] = dst_sets_data[i];

        return result;
    }

private:
    buma3d::DESCRIPTOR_SET_ALLOCATE_DESC        desc;
    std::vector<buma3d::IDescriptorSetLayout*>  layouts;
    std::vector<buma3d::IDescriptorSet*>        dst_sets;

};

#pragma endregion descriptor heap/pool

#pragma region input layout builder

class InputLayoutDesc;
class InputSlotDesc;

class InputElementDesc
{
public:
    InputElementDesc(InputSlotDesc* _parent)
        : parent        { _parent }
        , desc          {}
        , semantic_name {}
    {}
    InputElementDesc(InputSlotDesc* _parent, const char* _semantic_name, uint32_t _semantic_index, buma3d::RESOURCE_FORMAT _format, uint32_t _aligned_byte_offset = buma3d::B3D_APPEND_ALIGNED_ELEMENT)
        : parent        { _parent }
        , desc          { nullptr, _semantic_index, _format, _aligned_byte_offset }
        , semantic_name { _semantic_name }
    {
        semantic_name.assign(_semantic_name);
        Finalize();
    }
    InputElementDesc(const InputElementDesc& _c)
        : parent        { _c.parent }
        , desc          { _c.desc }
        , semantic_name { _c.semantic_name }
    {
        if (parent)
            Finalize();
    }
    InputElementDesc& operator=(const InputElementDesc& _c)
    {
        parent        = _c.parent;
        desc          = _c.desc;
        semantic_name = _c.semantic_name;
        if (parent)
            Finalize();
        return *this;
    }
    ~InputElementDesc()
    {}

    void SetParent(InputSlotDesc* _parent)
    {
        parent = _parent;
    }

    InputElementDesc& Reset()
    {
        semantic_name.clear();
        desc = {};
        return *this;
    }
    InputElementDesc& Set(const char* _semantic_name, uint32_t _semantic_index, buma3d::RESOURCE_FORMAT _format, uint32_t _aligned_byte_offset = buma3d::B3D_APPEND_ALIGNED_ELEMENT)
    {
        semantic_name.assign(_semantic_name);
        desc = { nullptr, _semantic_index, _format, _aligned_byte_offset };
        return *this;
    }
    InputSlotDesc& Finalize()
    {
        desc.semantic_name = semantic_name.c_str();
        return *parent;
    }

    const buma3d::INPUT_ELEMENT_DESC& Get() const { return desc; }

private:
    InputSlotDesc*              parent;
    std::string                 semantic_name;
    buma3d::INPUT_ELEMENT_DESC  desc;

};

class InputSlotDesc
{
public:
    InputSlotDesc(InputLayoutDesc* _parent)
        : parent        { _parent }
        , desc          {}
        , elements      {}
        , b3d_elements  {}
    {}
    InputSlotDesc(const InputSlotDesc& _c)
        : parent        { _c.parent }
        , desc          { _c.desc }
        , elements      { _c.elements }
        , b3d_elements  { _c.b3d_elements }
    {
        for (auto& i : elements) i.SetParent(this);
        if (_c.parent)
            Finalize();
    }
    InputSlotDesc& operator=(const InputSlotDesc& _c)
    {
        parent        = _c.parent;
        desc          = _c.desc;
        elements      = _c.elements;
        b3d_elements  = _c.b3d_elements;
        for (auto& i : elements) i.SetParent(this);
        if (_c.parent)
            Finalize();
        return *this;
    }
    ~InputSlotDesc()
    {}

    void SetParent(InputLayoutDesc* _parent)
    {
        for (auto& i : elements) i.SetParent(this);
        parent = _parent;
    }

    InputSlotDesc& Reset()
    {
        desc = {};
        for (auto& i : elements) i.Reset();
        for (auto& i : b3d_elements) i = {};
        return *this;
    }
    InputSlotDesc& SetSlotNumber          (uint32_t                     _slot_number)                                                   { desc.slot_number             = _slot_number;     return *this; }
    InputSlotDesc& SetStrideInBytes       (uint32_t                     _stride_in_bytes)                                               { desc.stride_in_bytes         = _stride_in_bytes; return *this; }
    InputSlotDesc& SetClassification      (buma3d::INPUT_CLASSIFICATION _classification = buma3d::INPUT_CLASSIFICATION_PER_VERTEX_DATA) { desc.classification          = _classification;  return *this; }
    InputSlotDesc& SetInstanceDataStepRate(uint32_t                     _step_rate = 0)                                                 { desc.instance_data_step_rate = _step_rate;       return *this; }
    InputElementDesc& AddNewInputElement()
    {
        Resize(desc.num_elements + 1);
        return elements.data()[desc.num_elements++];
    }
    InputSlotDesc& AddNewInputElement(const char* _semantic_name, uint32_t _semantic_index, buma3d::RESOURCE_FORMAT _format, uint32_t _aligned_byte_offset = buma3d::B3D_APPEND_ALIGNED_ELEMENT)
    {
        AddNewInputElement() = InputElementDesc(this, _semantic_name, _semantic_index, _format, _aligned_byte_offset);
        return *this;
    }
    InputLayoutDesc& Finalize()
    {
        auto e = elements.data();
        auto b3de = b3d_elements.data();
        for (uint32_t i = 0; i < desc.num_elements; i++)
            b3de[i] = e[i].Get();

        return *parent;
    }

    const buma3d::INPUT_SLOT_DESC& Get() const 
    {
        return desc;
    }

private:
    void Resize(uint32_t _num_elements)
    {
        if (_num_elements > desc.num_elements)
        {
            elements.resize(_num_elements, this);
            b3d_elements.resize(_num_elements);
            desc.elements = b3d_elements.data();
        }
    }

private:
    InputLayoutDesc*                        parent;
    buma3d::INPUT_SLOT_DESC                 desc;
    std::vector<InputElementDesc>           elements;
    std::vector<buma3d::INPUT_ELEMENT_DESC> b3d_elements;

};

class InputLayoutDesc
{
public:
    InputLayoutDesc(uint32_t _num_input_slots_reserve = 0)
        : desc      {}
        , slots     (_num_input_slots_reserve, this)
        , b3d_slots { _num_input_slots_reserve }
    {}
    InputLayoutDesc(const InputLayoutDesc& _c)
        : desc      { _c.desc }
        , slots     { _c.slots }
        , b3d_slots { _c.b3d_slots }
    {
        for (auto& i : slots) i.SetParent(this);
        Finalize();
    }
    InputLayoutDesc& operator=(const InputLayoutDesc& _c)
    {
        desc      = _c.desc;
        slots     = _c.slots;
        b3d_slots = _c.b3d_slots;
        for (auto& i : slots) i.SetParent(this);
        Finalize();
        return *this;
    }
    ~InputLayoutDesc()
    {}

    InputLayoutDesc& Reset()
    {
        desc = {};
        for (auto& i : slots) i.Reset();
        for (auto& i : b3d_slots) i = {};
        return *this;
    }
    InputSlotDesc& AddNewInputSlot()
    {
        Resize(desc.num_input_slots + 1);
        return slots.data()[desc.num_input_slots++];
    }
    void Finalize()
    {
        auto s = slots.data();
        auto b3ds = b3d_slots.data();
        for (uint32_t i = 0; i < desc.num_input_slots; i++)
            b3ds[i] = s[i].Get();

        desc.input_slots = b3ds;
    }

    const buma3d::INPUT_LAYOUT_DESC& Get() const 
    {
        return desc;
    }

private:
    void Resize(uint32_t _num_slots)
    {
        if (_num_slots > desc.num_input_slots)
        {
            slots.resize(_num_slots, this);
            b3d_slots.resize(_num_slots);
            desc.input_slots = b3d_slots.data();
        }
    }

private:
    buma3d::INPUT_LAYOUT_DESC               desc;
    std::vector<InputSlotDesc>              slots;
    std::vector<buma3d::INPUT_SLOT_DESC>    b3d_slots;

};

#pragma endregion input layout builder

#pragma region blend state

class BlendStateDesc;

class RenderTargetBlendDesc
{
public:
    RenderTargetBlendDesc(BlendStateDesc* _parent)
        : parent    { _parent }
        , desc      {}
    {}
    RenderTargetBlendDesc(const RenderTargetBlendDesc& _c)
        : parent    { _c.parent }
        , desc      { _c.desc }
    {}
    ~RenderTargetBlendDesc()
    {}

    void SetParent(BlendStateDesc* _parent)
    {
        parent = _parent;
    }

    RenderTargetBlendDesc& Reset()
    {
        desc = {};
        return *this;
    }
    RenderTargetBlendDesc& Src      (buma3d::BLEND_FACTOR _factor) { desc.src_blend = _factor; return *this; }
    RenderTargetBlendDesc& Op       (buma3d::BLEND_OP     _op)     { desc.blend_op  = _op;     return *this; }
    RenderTargetBlendDesc& Dst      (buma3d::BLEND_FACTOR _factor) { desc.dst_blend = _factor; return *this; }
    RenderTargetBlendDesc& SrcAlpha (buma3d::BLEND_FACTOR _factor) { desc.src_blend_alpha = _factor; return *this; }
    RenderTargetBlendDesc& OpAlpha  (buma3d::BLEND_OP     _op)     { desc.blend_op_alpha  = _op;     return *this; }
    RenderTargetBlendDesc& DstAlpha (buma3d::BLEND_FACTOR _factor) { desc.dst_blend_alpha = _factor; return *this; }
    RenderTargetBlendDesc& ColorWriteMask(buma3d::COLOR_WRITE_FLAGS _color_write_mask = buma3d::COLOR_WRITE_FLAG_ALL) { desc.color_write_mask = _color_write_mask; return *this; }
    RenderTargetBlendDesc& BlendDisabled(buma3d::COLOR_WRITE_FLAGS _color_write_mask = buma3d::COLOR_WRITE_FLAG_ALL)
    {
        desc = {};
        desc.is_enabled_blend = false;
        desc.color_write_mask = _color_write_mask;
        return *this;
    }
    RenderTargetBlendDesc& BlendAdditive(buma3d::COLOR_WRITE_FLAGS _color_write_mask = buma3d::COLOR_WRITE_FLAG_ALL)
    {
        desc.is_enabled_blend = true;
        desc.src_blend        = buma3d::BLEND_FACTOR_SRC_ALPHA;
        desc.dst_blend        = buma3d::BLEND_FACTOR_ONE;
        desc.blend_op         = buma3d::BLEND_OP_ADD;
        desc.src_blend_alpha  = buma3d::BLEND_FACTOR_ZERO;
        desc.dst_blend_alpha  = buma3d::BLEND_FACTOR_ONE;
        desc.blend_op_alpha   = buma3d::BLEND_OP_ADD;
        desc.color_write_mask = _color_write_mask;
        return *this;
    }
    RenderTargetBlendDesc& BlendSubtractive(buma3d::COLOR_WRITE_FLAGS _color_write_mask = buma3d::COLOR_WRITE_FLAG_ALL)
    {
        desc.is_enabled_blend = true;
        desc.src_blend        = buma3d::BLEND_FACTOR_SRC_ALPHA;
        desc.dst_blend        = buma3d::BLEND_FACTOR_ONE;
        desc.blend_op         = buma3d::BLEND_OP_REVERSE_SUBTRACT;
        desc.src_blend_alpha  = buma3d::BLEND_FACTOR_ZERO;
        desc.dst_blend_alpha  = buma3d::BLEND_FACTOR_ONE;
        desc.blend_op_alpha   = buma3d::BLEND_OP_ADD;
        desc.color_write_mask = _color_write_mask;
        return *this;
    }
    RenderTargetBlendDesc& BlendAlpha(buma3d::COLOR_WRITE_FLAGS _color_write_mask = buma3d::COLOR_WRITE_FLAG_ALL)
    {
        desc.is_enabled_blend = true;
        desc.src_blend        = buma3d::BLEND_FACTOR_SRC_ALPHA; // src.rgb * src.a
        desc.dst_blend        = buma3d::BLEND_FACTOR_SRC_ALPHA_INVERTED;
        desc.blend_op         = buma3d::BLEND_OP_ADD;
        desc.src_blend_alpha  = buma3d::BLEND_FACTOR_ONE;
        desc.dst_blend_alpha  = buma3d::BLEND_FACTOR_SRC_ALPHA_INVERTED;
        desc.blend_op_alpha   = buma3d::BLEND_OP_ADD;
        desc.color_write_mask = _color_write_mask;
        return *this;
    }
    RenderTargetBlendDesc& BlendPMA(buma3d::COLOR_WRITE_FLAGS _color_write_mask = buma3d::COLOR_WRITE_FLAG_ALL)
    {
        desc.is_enabled_blend = true;
        desc.src_blend        = buma3d::BLEND_FACTOR_ONE; // 事前乗算済み(src.rgb * src.aの結果を画像に焼き込むためsrc.aが1だと指定可能) 加算合成として振る舞うことも可能です。
        desc.dst_blend        = buma3d::BLEND_FACTOR_SRC_ALPHA_INVERTED;
        desc.blend_op         = buma3d::BLEND_OP_ADD;
        desc.src_blend_alpha  = buma3d::BLEND_FACTOR_ONE;
        desc.dst_blend_alpha  = buma3d::BLEND_FACTOR_SRC_ALPHA_INVERTED;
        desc.blend_op_alpha   = buma3d::BLEND_OP_ADD;
        desc.color_write_mask = _color_write_mask;
        return *this;
    }
    BlendStateDesc& Finalize()
    {
        return *parent;
    }

    const buma3d::RENDER_TARGET_BLEND_DESC& Get() const { return desc; }

private:
    BlendStateDesc*                     parent;
    buma3d::RENDER_TARGET_BLEND_DESC    desc;

};

class BlendStateDesc
{
public:
    BlendStateDesc(uint32_t _num_attachments = 1)
        : desc      {}
        , blend     { _num_attachments, this }
        , b3d_blend {}
    {
        Reset();
    }
    BlendStateDesc(const BlendStateDesc& _c)
        : desc      { _c.desc }
        , blend     { _c.blend }
        , b3d_blend { _c.b3d_blend }
    {
        for (auto& i : blend) i.SetParent(this);
        Finalize();
    }
    BlendStateDesc& operator=(const BlendStateDesc& _c)
    {
        desc      = _c.desc;
        blend     = _c.blend;
        b3d_blend = _c.b3d_blend;
        for (auto& i : blend) i.SetParent(this);
        Finalize();
        return *this;
    }
    ~BlendStateDesc()
    {}

    BlendStateDesc& Reset()
    {
        for (auto& i : blend)
            i.BlendDisabled();

        desc.is_enabled_independent_blend = false;
        desc.is_enabled_logic_op          = false;
        desc.logic_op                     = buma3d::LOGIC_OP_CLEAR;
        desc.num_attachments              = 0;
        desc.blend_constants              = { 1,1,1,1 };

        return *this;
    }
    BlendStateDesc&         SetLogicOp                  (buma3d::LOGIC_OP _logic_op)       { desc.logic_op = _logic_op; desc.is_enabled_logic_op = true; return *this; }
    BlendStateDesc&         SetBlendConstants           (const buma3d::COLOR4& _constants) { desc.blend_constants = _constants; return *this; }
    BlendStateDesc&         SetIndependentBlendEnabled  (bool _is_enabled)                 { desc.is_enabled_independent_blend = _is_enabled; desc.is_enabled_logic_op = false; return *this; }
    BlendStateDesc&         SetNumAttachmemns           (uint32_t _num_attachments)        { Resize(_num_attachments); desc.num_attachments = _num_attachments; return *this; }
    RenderTargetBlendDesc&  GetBlendDesc                (uint32_t _index)                  { return blend[_index]; }
    void Finalize()
    {
        auto b    = blend.data();
        auto b3db = b3d_blend.data();
        auto c = desc.is_enabled_independent_blend ? desc.num_attachments : std::min(desc.num_attachments, 1u);
        for (uint32_t i = 0; i < c; i++)
            b3db[i] = b[i].Get();

        desc.attachments = b3db;
    }

    const buma3d::BLEND_STATE_DESC& Get() const 
    {
        return desc;
    }

private:
    void Resize(uint32_t _num_attachments)
    {
        if (!desc.is_enabled_independent_blend)
            _num_attachments = 1;
        if (_num_attachments > desc.num_attachments)
        {
            blend.resize(_num_attachments, this);
            b3d_blend.resize(_num_attachments);
        }
    }

private:
    buma3d::BLEND_STATE_DESC                        desc;
    std::vector<RenderTargetBlendDesc>              blend;
    std::vector<buma3d::RENDER_TARGET_BLEND_DESC>   b3d_blend;

};

#pragma endregion blend state

#pragma region pipeline

class PipelineShaderStageDescs
{
public:
    PipelineShaderStageDescs(uint32_t _num_shader_stages_reserve = 0)
        : num_shader_stages {}
        , descs             { _num_shader_stages_reserve }
        , entry_point_names {}
        , stages            {}
    {
    }

    ~PipelineShaderStageDescs()
    {
    }

    PipelineShaderStageDescs& Reset()
    {
        num_shader_stages = 0;
        entry_point_names.clear();
        return *this;
    }

    PipelineShaderStageDescs& AddStage(  buma3d::SHADER_STAGE_FLAG           _stage
                                       , buma3d::IShaderModule*              _module
                                       , const char*                         _entry_point_name
                                       , buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE)
    {
        BUMA_ASSERT(!(stages & _stage));
        stages |= _stage;
        Resize(num_shader_stages + 1);
        auto&& ep = entry_point_names.emplace(_entry_point_name).first;
        descs.data()[num_shader_stages] = { _flags, _stage, _module, ep->c_str() };
        num_shader_stages++;
        return *this;
    }
    PipelineShaderStageDescs& AddVS(buma3d::IShaderModule* _module, const char* _entry_point_name, buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE) { AddStage(buma3d::SHADER_STAGE_FLAG_VERTEX   , _module, _entry_point_name, _flags); return *this; }
    PipelineShaderStageDescs& AddHS(buma3d::IShaderModule* _module, const char* _entry_point_name, buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE) { AddStage(buma3d::SHADER_STAGE_FLAG_HULL     , _module, _entry_point_name, _flags); return *this; }
    PipelineShaderStageDescs& AddDS(buma3d::IShaderModule* _module, const char* _entry_point_name, buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE) { AddStage(buma3d::SHADER_STAGE_FLAG_DOMAIN   , _module, _entry_point_name, _flags); return *this; }
    PipelineShaderStageDescs& AddGS(buma3d::IShaderModule* _module, const char* _entry_point_name, buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE) { AddStage(buma3d::SHADER_STAGE_FLAG_GEOMETRY , _module, _entry_point_name, _flags); return *this; }
    PipelineShaderStageDescs& AddPS(buma3d::IShaderModule* _module, const char* _entry_point_name, buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE) { AddStage(buma3d::SHADER_STAGE_FLAG_PIXEL    , _module, _entry_point_name, _flags); return *this; }
    PipelineShaderStageDescs& AddCS(buma3d::IShaderModule* _module, const char* _entry_point_name, buma3d::PIPELINE_SHADER_STAGE_FLAGS _flags = buma3d::PIPELINE_SHADER_STAGE_FLAG_NONE) { AddStage(buma3d::SHADER_STAGE_FLAG_COMPUTE  , _module, _entry_point_name, _flags); return *this; }

    uint32_t                                    GetSize() const { return num_shader_stages; }
    const buma3d::PIPELINE_SHADER_STAGE_DESC*   Get()     const { return descs.data(); }
    const buma3d::PIPELINE_SHADER_STAGE_DESC*   Get(buma3d::SHADER_STAGE_FLAG _stage) const
    {
        auto find = std::find_if(descs.begin(), descs.end(), [_stage](const buma3d::PIPELINE_SHADER_STAGE_DESC& _d) { return _d.stage == _stage; });
        return find == descs.end() ? nullptr : &(*find);
    }
    bool HasStage(buma3d::SHADER_STAGE_FLAG _stage)
    {
        auto find = std::find_if(descs.begin(), descs.end(), [_stage](const buma3d::PIPELINE_SHADER_STAGE_DESC& _d) { return _d.stage == _stage; });
        return find != descs.end();
    }

private:
    void Resize(uint32_t _num_shader_stages)
    {
        if (_num_shader_stages > num_shader_stages)
        {
            descs.resize(_num_shader_stages);
        }
    }

private:
    uint32_t                                        num_shader_stages;
    std::vector<buma3d::PIPELINE_SHADER_STAGE_DESC> descs;
    std::set<std::string>                           entry_point_names;
    buma3d::SHADER_STAGE_FLAGS                      stages;

};

class ViewportStateDesc
{
public:
    ViewportStateDesc(uint32_t _num_viewports = 1, uint32_t _num_rects = 1)
        : desc          {}
        , viewports     { _num_viewports }
        , scissor_rects { _num_rects }
    {
    }
    ViewportStateDesc(const ViewportStateDesc& _c)
        : desc          { _c.desc }
        , viewports     { _c.viewports }
        , scissor_rects { _c.scissor_rects }
    {
        Finalize();
    }
    ViewportStateDesc& operator=(const ViewportStateDesc& _c)
    {
        desc          = _c.desc;
        viewports     = _c.viewports;
        scissor_rects = _c.scissor_rects;
        Finalize();
    }

    ~ViewportStateDesc()
    {
    }

    ViewportStateDesc& Reset()
    {
        desc = {};
        for (auto& i : viewports) i = {};
        for (auto& i : scissor_rects) i = {};
        return *this;
    }
    ViewportStateDesc& SetCounts(uint32_t _num_viewports = 1, uint32_t _num_rects = 1)
    {
        desc.num_viewports     = _num_viewports;
        desc.num_scissor_rects = _num_rects;
        Resize(_num_viewports, _num_rects);
        return *this;
    }
    ViewportStateDesc& SetViewport(uint32_t _index, const buma3d::VIEWPORT& _v)
    {
        viewports[_index] = _v;
        return *this;
    }
    ViewportStateDesc& SetViewport(uint32_t _index, float _x, float _y, float _width, float _height, float _min_depth = buma3d::B3D_VIEWPORT_MIN_DEPTH, float _max_depth = buma3d::B3D_VIEWPORT_MAX_DEPTH)
    {
        viewports[_index] = { _x, _y, _width, _height, _min_depth, _max_depth };
        return *this;
    }
    ViewportStateDesc& SetViewports(const std::function<void(uint32_t _index, buma3d::VIEWPORT* _v)>& _Setter, uint32_t _offset = 0)
    {
        auto v = viewports.data();
        for (uint32_t i = _offset; i < desc.num_viewports; i++)
            _Setter(i, v + i);
        return *this;
    }
    ViewportStateDesc& SetScissor(uint32_t _index, const buma3d::SCISSOR_RECT& _r)
    {
        scissor_rects[_index] = _r;
        return *this;
    }
    ViewportStateDesc& SetScissor(uint32_t _index, int32_t _x, int32_t _y, uint32_t _w, uint32_t _h)
    {
        scissor_rects[_index] = { _x, _y, _w, _h };
        return *this;
    }
    ViewportStateDesc& SetScissors(const std::function<void(uint32_t _index, buma3d::SCISSOR_RECT* _v)>& _Setter, uint32_t _offset = 0)
    {
        auto r = scissor_rects.data();
        for (uint32_t i = _offset; i < desc.num_scissor_rects; i++)
            _Setter(i, r + i);
        return *this;
    }
    ViewportStateDesc& Finalize(bool _is_viewport_dynamic = false, bool _is_scissor_dynamic = false)
    {
        desc.viewports     = _is_viewport_dynamic ? nullptr : viewports.data();
        desc.scissor_rects = _is_scissor_dynamic ? nullptr : scissor_rects.data();
        return *this;
    }
    const buma3d::VIEWPORT_STATE_DESC& Get() const { return desc; }

private:
    void Resize(uint32_t _num_viewports, uint32_t _num_rects)
    {
        if (_num_viewports > (uint32_t)viewports.size())
            viewports.resize(_num_viewports);

        if (_num_rects > (uint32_t)scissor_rects.size())
            scissor_rects.resize(_num_rects);
    }

private:
    buma3d::VIEWPORT_STATE_DESC         desc;
    std::vector<buma3d::VIEWPORT>       viewports;      // DYNAMIC_STATE_VIEWPORTが無効な場合に使用するビューポートの配列です。 DYNAMIC_STATE_VIEWPORTが有効の場合、値はnullptrである必要があります。
    std::vector<buma3d::SCISSOR_RECT>   scissor_rects;  // DYNAMIC_STATE_SCISSORが無効な場合に使用するシザー矩形の配列です。    DYNAMIC_STATE_SCISSORが有効の場合、値はnullptrである必要があります。

};

class RasterizationStateDesc
{
public:
    RasterizationStateDesc()
        : desc{}
    {
    }
    RasterizationStateDesc(const buma3d::RASTERIZATION_STATE_DESC& _desc)
        : desc{ _desc }
    {
    }

    ~RasterizationStateDesc()
    {
    }

    RasterizationStateDesc& Reset()
    {
        desc = buma3d::init::RasterizationStateDesc(buma3d::FILL_MODE_SOLID, buma3d::CULL_MODE_NONE, false, false);
        return *this;
    }
    RasterizationStateDesc& FillMode             (buma3d::FILL_MODE _fill_mode)     { desc.fill_mode = _fill_mode;                                   return *this; }
    RasterizationStateDesc& CullMode             (buma3d::CULL_MODE _cull_mode)     { desc.cull_mode = _cull_mode;                                   return *this; }
    RasterizationStateDesc& FrontCounterClockwise(bool _is_front_counter_clockwise) { desc.is_front_counter_clockwise = _is_front_counter_clockwise; return *this; }
    RasterizationStateDesc& DepthClipEnabled     (bool _is_enabled_depth_clip)      { desc.is_enabled_depth_clip = _is_enabled_depth_clip;           return *this; }
    RasterizationStateDesc& DepthBias            (bool _is_enabled_depth_bias, int32_t _depth_bias_scale = 0, float _depth_bias_clamp = 0.f, float _depth_bias_slope_scale = 0.f)
    {
        desc.is_enabled_depth_bias = _is_enabled_depth_bias;
        BUMA_ASSERT(!desc.is_enabled_depth_bias
               && _depth_bias_scale       == 0
               && _depth_bias_clamp       == 0.f
               && _depth_bias_slope_scale == 0.f);
        desc.depth_bias_scale       = _depth_bias_scale;
        desc.depth_bias_clamp       = _depth_bias_clamp;
        desc.depth_bias_slope_scale = _depth_bias_slope_scale;
        return *this;
    }
    RasterizationStateDesc& ConservativeRasterEnabled(bool _is_enabled_conservative_raster) { desc.is_enabled_conservative_raster = _is_enabled_conservative_raster; return *this; }
    RasterizationStateDesc& LineRasterization(buma3d::LINE_RASTERIZATION_MODE _line_rasterization_mode = buma3d::LINE_RASTERIZATION_MODE_DEFAULT, float _line_width = 1.f)
    {
        desc.line_rasterization_mode = _line_rasterization_mode;
        desc.line_width              = _line_width;
        return *this;
    }

    const buma3d::RASTERIZATION_STATE_DESC& Get() const { return desc; }

private:
    buma3d::RASTERIZATION_STATE_DESC desc;

};

class MultisampleStateDesc
{
public:
    MultisampleStateDesc()
        : desc          {}
        , sample_masks  { ~0u, ~0u }
    {
    }
    MultisampleStateDesc(const MultisampleStateDesc& _c)
        : desc          { _c.desc }
        , sample_masks  { _c.sample_masks[0], _c.sample_masks[1] }
    {
        Finalize();
    }
    MultisampleStateDesc& operator=(const MultisampleStateDesc& _c)
    {
        desc = _c.desc;
        sample_masks[0] = _c.sample_masks[0];
        sample_masks[1] = _c.sample_masks[1];
        Finalize();
    }

    ~MultisampleStateDesc()
    {
    }

    MultisampleStateDesc& Reset()
    {
        desc = buma3d::init::MultisampleStateDesc();
        return *this;
    }
    MultisampleStateDesc& RasterizationSamples(uint32_t _rasterization_samples = 1)
    {
        desc.rasterization_samples = _rasterization_samples;
        return *this;
    }
    template <uint32_t MaskIndex>
    MultisampleStateDesc& SampleMask(buma3d::SampleMask _mask = ~0u)
    {
        static_assert(MaskIndex < 2);
        sample_masks[MaskIndex] = _mask;
        return *this;
    }
    MultisampleStateDesc& AlphaToCoverage(bool _is_enabled_alpha_to_coverage)
    {
        desc.is_enabled_alpha_to_coverage = _is_enabled_alpha_to_coverage;
        return *this;
    }
    MultisampleStateDesc& SampleRateShading(bool _is_enabled_sample_rate_shading)
    {
        desc.is_enabled_sample_rate_shading = _is_enabled_sample_rate_shading;
        return *this;
    }
    MultisampleStateDesc& Finalize()
    {
        desc.sample_masks = sample_masks;
        return *this;
    }
    const buma3d::MULTISAMPLE_STATE_DESC& Get() const 
    {
        return desc;
    }

private:
    buma3d::MULTISAMPLE_STATE_DESC              desc;
    buma3d::SampleMask                          sample_masks[2];
    //std::unique_ptr<SamplePositionDesc>       sample_pos_desc;

};

class DepthStencilStateDesc
{
public:
    DepthStencilStateDesc()
        : desc{ buma3d::init::DepthStencilStateDesc() }
    {
    }

    ~DepthStencilStateDesc()
    {
    }

    DepthStencilStateDesc& Reset()
    {
        desc = buma3d::init::DepthStencilStateDesc();
        return *this;
    }
    DepthStencilStateDesc& DepthTest(bool _is_enabled_depth_test, bool _is_enabled_depth_write = false, buma3d::COMPARISON_FUNC _depth_comparison_func = buma3d::COMPARISON_FUNC_NEVER)
    {
        desc.is_enabled_depth_test  = _is_enabled_depth_test;
        desc.is_enabled_depth_write = _is_enabled_depth_write;
        desc.depth_comparison_func  = _depth_comparison_func;
        return *this;
    }
    DepthStencilStateDesc& DepthBoundsTest(bool _is_enabled_depth_bounds_test, float _min_depth_bounds = 0.f, float _max_depth_bounds = 1.f)
    {
        desc.is_enabled_depth_bounds_test = _is_enabled_depth_bounds_test;
        desc.min_depth_bounds             = _min_depth_bounds;
        desc.max_depth_bounds             = _max_depth_bounds;
        return *this;
    }
    DepthStencilStateDesc& StencilTestFront(const buma3d::DEPTH_STENCILOP_DESC& _stencil_front_face = buma3d::init::DepthStencilOpDesc())
    {
        desc.is_enabled_stencil_test = true;
        desc.stencil_front_face = _stencil_front_face;
        return *this;
    }
    DepthStencilStateDesc& StencilTestBack(const buma3d::DEPTH_STENCILOP_DESC& _stencil_back_face = buma3d::init::DepthStencilOpDesc())
    {
        desc.is_enabled_stencil_test = true;
        desc.stencil_back_face = _stencil_back_face;
        return *this;
    }
    DepthStencilStateDesc& StencilTestDisable()
    {
        desc.is_enabled_stencil_test = false;
        desc.stencil_front_face      = buma3d::init::DepthStencilOpDesc();
        desc.stencil_back_face       = buma3d::init::DepthStencilOpDesc();
        return *this;
    }
    DepthStencilStateDesc& DepthStencilDisable()
    {
        desc = buma3d::init::DepthStencilStateDesc();
        return *this;
    }
    const buma3d::DEPTH_STENCIL_STATE_DESC& Get() const
    {
        return desc;
    }

private:
    buma3d::DEPTH_STENCIL_STATE_DESC desc;

};


class PipelineStateDesc
{
public:
    PipelineStateDesc()
        : gpso                      {}
        , cpso                      {}
        , shader_stage_desc         {}
        , input_layout_desc         {}
        , input_assembly_state_desc {}
        , viewport_state_desc       {}
        , rasterization_state_desc  {}
        , multisample_state_desc    {}
        , depth_stencil_state_desc  {}
        , blend_state_desc          {}
        , dynamic_state_desc        {}
        , dynamic_states            {}
    {
        Reset();
    }
    PipelineStateDesc(const PipelineStateDesc& _c)
        : gpso                      { _c.gpso }
        , cpso                      { _c.cpso }
        , shader_stage_desc         { _c.shader_stage_desc }
        , input_layout_desc         { _c.input_layout_desc }
        , input_assembly_state_desc { _c.input_assembly_state_desc }
        , viewport_state_desc       { _c.viewport_state_desc }
        , rasterization_state_desc  { _c.rasterization_state_desc }
        , multisample_state_desc    { _c.multisample_state_desc }
        , depth_stencil_state_desc  { _c.depth_stencil_state_desc }
        , blend_state_desc          { _c.blend_state_desc }
        , dynamic_state_desc        { _c.dynamic_state_desc }
        , dynamic_states            { _c.dynamic_states }
    {        
        Finalize();
    }
    PipelineStateDesc& operator=(const PipelineStateDesc& _c)
    {        
        gpso                      = _c.gpso;
        cpso                      = _c.cpso;
        shader_stage_desc         = _c.shader_stage_desc;
        input_layout_desc         = _c.input_layout_desc;
        input_assembly_state_desc = _c.input_assembly_state_desc;
        viewport_state_desc       = _c.viewport_state_desc;
        rasterization_state_desc  = _c.rasterization_state_desc;
        multisample_state_desc    = _c.multisample_state_desc;
        depth_stencil_state_desc  = _c.depth_stencil_state_desc;
        blend_state_desc          = _c.blend_state_desc;
        dynamic_state_desc        = _c.dynamic_state_desc;
        dynamic_states            = _c.dynamic_states;
        Finalize();
    }

    ~PipelineStateDesc()
    {
    }

    PipelineStateDesc& Reset()
    {
        gpso = {};
        cpso = {};
        shader_stage_desc        .Reset();
        input_layout_desc        .Reset();
        input_assembly_state_desc = { buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
        viewport_state_desc      .Reset();
        rasterization_state_desc .Reset();
        multisample_state_desc   .Reset();
        depth_stencil_state_desc .Reset();
        blend_state_desc         .Reset();
        dynamic_state_desc        = {};
        dynamic_states           .clear();
        return *this;
    }
    PipelineStateDesc&                 SetPipelineLayout    (buma3d::IPipelineLayout*     _pipeline_layout)                              { gpso.pipeline_layout = _pipeline_layout; return *this; }
    PipelineStateDesc&                 SetRenderPass        (buma3d::IRenderPass*         _render_pass)                                  { gpso.render_pass     = _render_pass;     return *this; }
    PipelineStateDesc&                 SetSubpass           (uint32_t                     _subpass   = 0)                                { gpso.subpass         = _subpass;         return *this; }
    PipelineStateDesc&                 SetNodeMask          (buma3d::NodeMask             _node_mask = buma3d::B3D_DEFAULT_NODE_MASK)    { gpso.node_mask       = _node_mask;       return *this; }
    PipelineStateDesc&                 SetPipelineStateFlags(buma3d::PIPELINE_STATE_FLAGS _flags     = buma3d::PIPELINE_STAGE_FLAG_NONE) { gpso.flags           = _flags;           return *this; }
    PipelineStateDesc&                 AddDynamicState      (buma3d::DYNAMIC_STATE        _state)                                        { dynamic_states.emplace_back(_state);     return *this; }
    PipelineShaderStageDescs&          ShaderStages         () { return shader_stage_desc; }
    InputLayoutDesc&                   InputLayout          () { return input_layout_desc; }
    buma3d::INPUT_ASSEMBLY_STATE_DESC& InputAssemblyState   () { return input_assembly_state_desc; }
    ViewportStateDesc&                 ViewportState        () { return viewport_state_desc; }
    RasterizationStateDesc&            RasterizationState   () { return rasterization_state_desc; }
    MultisampleStateDesc&              MultisampleState     () { return multisample_state_desc; }
    DepthStencilStateDesc&             DepthStencilState    () { return depth_stencil_state_desc; }
    BlendStateDesc&                    BlendState           () { return blend_state_desc; }
    PipelineStateDesc&                 Finalize             ()
    {
        gpso.num_shader_stages      = shader_stage_desc.GetSize();
        gpso.shader_stages          = shader_stage_desc.Get();
        gpso.input_layout           = &input_layout_desc.Get();
        gpso.input_assembly_state   = &input_assembly_state_desc;
        gpso.tessellation_state     = nullptr; //&tessellation_state_desc;
        gpso.viewport_state         = &viewport_state_desc.Get();
        gpso.rasterization_state    = &rasterization_state_desc.Get();
        gpso.stream_output          = nullptr; //&stream_output_desc
        gpso.multisample_state      = &multisample_state_desc.Get();
        gpso.depth_stencil_state    = &depth_stencil_state_desc.Get();
        gpso.blend_state            = &blend_state_desc.Get();
        gpso.dynamic_state          = &dynamic_state_desc;

        dynamic_state_desc.num_dynamic_states = (uint32_t)dynamic_states.size();
        dynamic_state_desc.dynamic_states     = dynamic_states.data();

        if (auto cs = shader_stage_desc.Get(buma3d::SHADER_STAGE_FLAG_COMPUTE))
        {
            cpso.node_mask       = gpso.node_mask;
            cpso.pipeline_layout = gpso.pipeline_layout;
            cpso.shader_stage    = *cs;
        }

        return *this;
    }
    const buma3d::GRAPHICS_PIPELINE_STATE_DESC& GetAsGraphics() const { return gpso; }
    const buma3d::COMPUTE_PIPELINE_STATE_DESC&  GetAsCompute()  const { BUMA_ASSERT(cpso.shader_stage.module); return cpso; }

private:
    buma3d::GRAPHICS_PIPELINE_STATE_DESC    gpso;
    buma3d::COMPUTE_PIPELINE_STATE_DESC     cpso;
    PipelineShaderStageDescs                shader_stage_desc;
    InputLayoutDesc                         input_layout_desc;
    buma3d::INPUT_ASSEMBLY_STATE_DESC       input_assembly_state_desc;
    //buma3d::TESSELLATION_STATE_DESC       tessellation_state_desc;
    ViewportStateDesc                       viewport_state_desc;
    RasterizationStateDesc                  rasterization_state_desc;
    //buma3d::STREAM_OUTPUT_DESC            stream_output_desc;
    MultisampleStateDesc                    multisample_state_desc;
    DepthStencilStateDesc                   depth_stencil_state_desc;
    BlendStateDesc                          blend_state_desc;
    buma3d::DYNAMIC_STATE_DESC              dynamic_state_desc;
    std::vector<buma3d::DYNAMIC_STATE>      dynamic_states;

};


#pragma endregion pipeline

#pragma region resources

class SamplerDesc
{
public:
    SamplerDesc()
        : desc{}
    {
    }
    SamplerDesc(const SamplerDesc& _c)
        : desc{ _c.desc }
    {
    }
    SamplerDesc& operator=(const SamplerDesc& _c)
    {
        desc = _c.desc;
    }

    ~SamplerDesc()
    {
    }

    SamplerDesc& Reset()
    {
        desc = buma3d::init::SamplerDesc();
        return *this;
    }
    SamplerDesc& FilterStandard(  buma3d::SAMPLER_FILTER_REDUCTION_MODE _reduction_mode = buma3d::SAMPLER_FILTER_REDUCTION_MODE_STANDARD
                                , buma3d::COMPARISON_FUNC               _comparison_func = buma3d::COMPARISON_FUNC_NEVER)
    {
        desc.filter.mode            = buma3d::SAMPLER_FILTER_MODE_STANDARD;
        desc.filter.reduction_mode  = _reduction_mode;
        desc.filter.max_anisotropy  = 1;
        desc.filter.comparison_func = _comparison_func;
        return *this;
    }
    SamplerDesc& FilterAnisotropy(  uint32_t                              _max_anisotropy
                                  , buma3d::SAMPLER_FILTER_REDUCTION_MODE _reduction_mode = buma3d::SAMPLER_FILTER_REDUCTION_MODE_STANDARD
                                  , buma3d::COMPARISON_FUNC               _comparison_func = buma3d::COMPARISON_FUNC_NEVER)
    {
        desc.filter.mode            = buma3d::SAMPLER_FILTER_MODE_ANISOTROPIC;
        desc.filter.reduction_mode  = _reduction_mode;
        desc.filter.max_anisotropy  = _max_anisotropy;
        desc.filter.comparison_func = _comparison_func;
        return *this;
    }
    SamplerDesc& TextureSampleMode(  buma3d::TEXTURE_SAMPLE_MODE _min = buma3d::TEXTURE_SAMPLE_MODE_LINEAR
                                   , buma3d::TEXTURE_SAMPLE_MODE _mag = buma3d::TEXTURE_SAMPLE_MODE_LINEAR
                                   , buma3d::TEXTURE_SAMPLE_MODE _mip = buma3d::TEXTURE_SAMPLE_MODE_LINEAR)
    {
        BUMA_ASSERT((desc.filter.mode != buma3d::SAMPLER_FILTER_MODE_ANISOTROPIC) || (_min == _mag == _mip == buma3d::TEXTURE_SAMPLE_MODE_LINEAR));
        BUMA_ASSERT(_mip != buma3d::TEXTURE_SAMPLE_MODE_CUBIC_IMG);
        desc.texture.sample = { _min, _mag, _mip };
        return *this;
    }
    SamplerDesc& TextureSampleModes(buma3d::TEXTURE_SAMPLE_MODE _minmagmip = buma3d::TEXTURE_SAMPLE_MODE_LINEAR)
    {
        return TextureSampleMode(_minmagmip, _minmagmip
                                 , _minmagmip == buma3d::TEXTURE_SAMPLE_MODE_CUBIC_IMG ? buma3d::TEXTURE_SAMPLE_MODE_LINEAR : _minmagmip);
    }
    SamplerDesc& TextureAddressMode(  buma3d::TEXTURE_ADDRESS_MODE _u = buma3d::TEXTURE_ADDRESS_MODE_WRAP
                                    , buma3d::TEXTURE_ADDRESS_MODE _v = buma3d::TEXTURE_ADDRESS_MODE_WRAP
                                    , buma3d::TEXTURE_ADDRESS_MODE _w = buma3d::TEXTURE_ADDRESS_MODE_WRAP)
    {
        desc.texture.address = { _u, _v, _w };
        return *this;
    }
    SamplerDesc& TextureAddressModes(buma3d::TEXTURE_ADDRESS_MODE _uvw = buma3d::TEXTURE_ADDRESS_MODE_WRAP)
    {
        desc.texture.address = { _uvw , _uvw , _uvw };
        return *this;
    }
    SamplerDesc& BorderColor(buma3d::BORDER_COLOR _border_color = buma3d::BORDER_COLOR_OPAQUE_BLACK_FLOAT)
    {
        desc.border_color = _border_color;
        return *this;
    }
    const buma3d::SAMPLER_DESC& Get() const { return desc; }

private:
    buma3d::SAMPLER_DESC desc;

};

class BufferView
{
public:
    BufferView(buma3d::VIEW_DESC& _view_desc, buma3d::BUFFER_VIEW& _buffer_view)
        : view_desc     { _view_desc }
        , buffer_view   { _buffer_view }
    {
    }

    void StructuredBufferView(uint32_t _structure_byte_stride, uint32_t _first_element, uint32_t _num_elements)
    {
        view_desc.format                  = buma3d::RESOURCE_FORMAT_UNKNOWN;
        view_desc.dimension               = buma3d::VIEW_DIMENSION_BUFFER_STRUCTURED;
        buffer_view.first_element         = _first_element;
        buffer_view.num_elements          = _num_elements;
        buffer_view.structure_byte_stride = _structure_byte_stride;
    }
    void ByteAddressBufferView(uint32_t _first_element, uint32_t _num_elements)
    {
        view_desc.format                  = buma3d::RESOURCE_FORMAT_R32_TYPELESS;
        view_desc.dimension               = buma3d::VIEW_DIMENSION_BUFFER_BYTEADDRESS;
        buffer_view.first_element         = _first_element;
        buffer_view.num_elements          = _num_elements;
        buffer_view.structure_byte_stride = 0;
    }
    void TypedBufferView(buma3d::RESOURCE_FORMAT _format, uint32_t _first_element, uint32_t _num_elements)
    {
        view_desc.format                  = buma3d::RESOURCE_FORMAT_UNKNOWN;
        view_desc.dimension               = buma3d::VIEW_DIMENSION_BUFFER_TYPED;
        buffer_view.first_element         = _first_element;
        buffer_view.num_elements          = _num_elements;
        buffer_view.structure_byte_stride = 0;
    }

private:
    buma3d::VIEW_DESC& view_desc;
    buma3d::BUFFER_VIEW& buffer_view;

};

class TextureView
{
public:
    TextureView(buma3d::VIEW_DESC& _view_desc, buma3d::TEXTURE_VIEW& _texture_view)
        : view_desc     { _view_desc }
        , texture_view  { _texture_view }
    {
    }

    TextureView& Default(buma3d::RESOURCE_FORMAT _format)
    {
        TextureDimension();
        TextureFormat(_format);
        TextureAspect();
        MipLevels(0, view_desc.type == buma3d::VIEW_TYPE_RENDER_TARGET || view_desc.type == buma3d::VIEW_TYPE_DEPTH_STENCIL ? 1 : buma3d::B3D_USE_REMAINING_MIP_LEVELS);
        ArraySize();
        ComponentSwizzle();
        return *this;
    }
    TextureView& TextureDimension(buma3d::VIEW_DIMENSION _texture_dimension = buma3d::VIEW_DIMENSION_TEXTURE_2D)
    {
        view_desc.dimension = _texture_dimension;
        return *this;
    }
    TextureView& TextureFormat(buma3d::RESOURCE_FORMAT _format)
    {
        view_desc.format = _format;
        return *this;
    }
    TextureView& TextureAspect(buma3d::TEXTURE_ASPECT_FLAGS _aspect = buma3d::TEXTURE_ASPECT_FLAG_COLOR)
    {
        texture_view.subresource_range.offset.aspect = _aspect;
        return *this;
    }
    TextureView& MipLevels(uint32_t _mip_slice = 0, uint32_t _mip_levels = buma3d::B3D_USE_REMAINING_MIP_LEVELS)
    {
        BUMA_ASSERT(view_desc.type == buma3d::VIEW_TYPE_RENDER_TARGET ? _mip_levels == 1 : true);
        BUMA_ASSERT(view_desc.type == buma3d::VIEW_TYPE_DEPTH_STENCIL ? _mip_levels == 1 : true);
        texture_view.subresource_range.offset.mip_slice = _mip_slice;
        texture_view.subresource_range.mip_levels = _mip_levels;
        return *this;
    }
    TextureView& ArraySize(uint32_t _array_slice = 0, uint32_t _array_size = buma3d::B3D_USE_REMAINING_ARRAY_SIZES)
    {
        texture_view.subresource_range.offset.array_slice = _array_slice;
        texture_view.subresource_range.array_size = _array_size;
        return *this;
    }
    TextureView& ComponentSwizzle(  buma3d::COMPONENT_SWIZZLE _r = buma3d::COMPONENT_SWIZZLE_IDENTITY
                                  , buma3d::COMPONENT_SWIZZLE _g = buma3d::COMPONENT_SWIZZLE_IDENTITY
                                  , buma3d::COMPONENT_SWIZZLE _b = buma3d::COMPONENT_SWIZZLE_IDENTITY
                                  , buma3d::COMPONENT_SWIZZLE _a = buma3d::COMPONENT_SWIZZLE_IDENTITY)
    {
        texture_view.components = { _r, _g, _b, _a };
        return *this;
    }

private:
    buma3d::VIEW_DESC& view_desc;
    buma3d::TEXTURE_VIEW& texture_view;

};

class VertexBufferViewDesc
{
public:
    VertexBufferViewDesc(uint32_t _num_input_slots = 1)
        : desc              { 1 }
        , buffer_offsets    { 1 }
        , sizes_in_bytes    { 1 }
        , strides_in_bytes  { 1 }
    {}
    VertexBufferViewDesc(const VertexBufferViewDesc& _c)
        : desc              { _c.desc }
        , buffer_offsets    { _c.buffer_offsets }
        , sizes_in_bytes    { _c.sizes_in_bytes }
        , strides_in_bytes  { _c.strides_in_bytes }
    {}
    VertexBufferViewDesc& operator=(const VertexBufferViewDesc& _c)
    {
        desc             = _c.desc;
        buffer_offsets   = _c.buffer_offsets;
        sizes_in_bytes   = _c.sizes_in_bytes;
        strides_in_bytes = _c.strides_in_bytes;
    }
    ~VertexBufferViewDesc()
    {}

    VertexBufferViewDesc& Reset()
    {
        desc.num_input_slots = {};
        buffer_offsets.clear();
        sizes_in_bytes.clear();
        strides_in_bytes.clear();
        return *this;
    }
    VertexBufferViewDesc& SetNumInputSlots(uint32_t _num_input_slots = 1)
    {
        desc.num_input_slots = _num_input_slots;
        buffer_offsets.resize(_num_input_slots);
        sizes_in_bytes.resize(_num_input_slots);
        strides_in_bytes.resize(_num_input_slots);
        return *this;
    }
    VertexBufferViewDesc& SetView(uint32_t _index, uint64_t _buffer_offset, uint32_t _size_in_bytes, uint32_t _stride_in_bytes)
    {
        BUMA_ASSERT(_index < desc.num_input_slots);
        buffer_offsets  [_index] = _buffer_offset;
        sizes_in_bytes  [_index] = _size_in_bytes;
        strides_in_bytes[_index] = _stride_in_bytes;
        return *this;
    }
    VertexBufferViewDesc& Finalize()
    {
        desc.buffer_offsets   = buffer_offsets  .data();
        desc.sizes_in_bytes   = sizes_in_bytes  .data();
        desc.strides_in_bytes = strides_in_bytes.data();
        return *this;
    }
    const buma3d::VERTEX_BUFFER_VIEW_DESC& Get() const
    {
        return desc;
    }

private:
    buma3d::VERTEX_BUFFER_VIEW_DESC desc;
    std::vector<uint64_t>           buffer_offsets;
    std::vector<uint32_t>           sizes_in_bytes;
    std::vector<uint32_t>           strides_in_bytes;

};


class ShaderResourceViewDesc
{
public:
    ShaderResourceViewDesc()
        : desc{ buma3d::VIEW_TYPE_SHADER_RESOURCE }
    {
    }

    ShaderResourceViewDesc(const ShaderResourceViewDesc& _c)
        : desc{ _c.desc }
    {
    }
    ShaderResourceViewDesc& operator=(const ShaderResourceViewDesc& _c)
    {
        desc = _c.desc;
    }

    ~ShaderResourceViewDesc()
    {
    }

    ShaderResourceViewDesc& Reset()
    {
        desc = { buma3d::VIEW_TYPE_SHADER_RESOURCE };
        return *this;
    }
    BufferView AsBufferView()
    {
        return BufferView(desc.view, desc.buffer);
    }
    TextureView AsTextureView()
    {
        return TextureView(desc.view, desc.texture);
    }
    ShaderResourceViewDesc& SetFlags(buma3d::SHADER_RESOURCE_VIEW_FLAGS _flags = buma3d::SHADER_RESOURCE_VIEW_FLAG_NONE)
    {
        desc.flags = _flags;
        return *this;
    }
    const buma3d::SHADER_RESOURCE_VIEW_DESC& Get() const { return desc; }

private:
    buma3d::SHADER_RESOURCE_VIEW_DESC desc;

};

class UnorderedAccessViewDesc
{
public:
    UnorderedAccessViewDesc()
        : desc{ buma3d::VIEW_TYPE_UNORDERED_ACCESS }
    {
    }

    UnorderedAccessViewDesc(const UnorderedAccessViewDesc& _c)
        : desc{ _c.desc }
    {
    }
    UnorderedAccessViewDesc& operator=(const UnorderedAccessViewDesc& _c)
    {
        desc = _c.desc;
    }

    ~UnorderedAccessViewDesc()
    {
    }

    UnorderedAccessViewDesc& Reset()
    {
        desc = { buma3d::VIEW_TYPE_UNORDERED_ACCESS };
        return *this;
    }
    BufferView AsBufferView()
    {
        return BufferView(desc.view, desc.buffer);
    }
    UnorderedAccessViewDesc& SetCounterOffset(uint64_t _offset_in_bytes = 0)
    {
        desc.counter_offset_in_bytes = _offset_in_bytes;
        return *this;
    }
    TextureView AsTextureView()
    {
        return TextureView(desc.view, desc.texture);
    }
    UnorderedAccessViewDesc& SetFlags(buma3d::UNORDERED_ACCESS_VIEW_FLAGS _flags = buma3d::UNORDERED_ACCESS_VIEW_FLAG_NONE)
    {
        desc.flags = _flags;
        return *this;
    }
    const buma3d::UNORDERED_ACCESS_VIEW_DESC& Get() const { return desc; }

private:
    buma3d::UNORDERED_ACCESS_VIEW_DESC desc;

};

class RenderTargetViewDesc
{
public:
    RenderTargetViewDesc()
        : desc{ buma3d::VIEW_TYPE_RENDER_TARGET }
    {
    }
    RenderTargetViewDesc(buma3d::RESOURCE_FORMAT _format)
        : desc{ buma3d::VIEW_TYPE_RENDER_TARGET }
    {
        Reset(_format);
    }

    RenderTargetViewDesc(const RenderTargetViewDesc& _c)
        : desc{ _c.desc }
    {
    }
    RenderTargetViewDesc& operator=(const RenderTargetViewDesc& _c)
    {
        desc = _c.desc;
    }

    ~RenderTargetViewDesc()
    {
    }

    RenderTargetViewDesc& Reset(buma3d::RESOURCE_FORMAT _format = buma3d::RESOURCE_FORMAT_UNKNOWN)
    {
        desc = { buma3d::VIEW_TYPE_RENDER_TARGET };
        AsTextureView().Default(_format).MipLevels(0, 1);
        return *this;
    }
    TextureView AsTextureView()
    {
        return TextureView(desc.view, desc.texture);
    }
    RenderTargetViewDesc& SetFlags(buma3d::RENDER_TARGET_VIEW_FLAGS _flags = buma3d::RENDER_TARGET_VIEW_FLAG_NONE)
    {
        desc.flags = _flags;
        return *this;
    }
    const buma3d::RENDER_TARGET_VIEW_DESC& Get() const { return desc; }

private:
    buma3d::RENDER_TARGET_VIEW_DESC desc;

};

class DepthStencilViewDesc
{
public:
    DepthStencilViewDesc()
        : desc{ buma3d::VIEW_TYPE_DEPTH_STENCIL }
    {
    }

    DepthStencilViewDesc(const DepthStencilViewDesc& _c)
        : desc{ _c.desc }
    {
    }
    DepthStencilViewDesc& operator=(const DepthStencilViewDesc& _c)
    {
        desc = _c.desc;
    }

    ~DepthStencilViewDesc()
    {
    }

    DepthStencilViewDesc& Reset()
    {
        desc = { buma3d::VIEW_TYPE_DEPTH_STENCIL };
        return *this;
    }
    TextureView AsTextureView()
    {
        return TextureView(desc.view, desc.texture);
    }
    DepthStencilViewDesc& SetFlags(buma3d::DEPTH_STENCIL_VIEW_FLAGS _flags = buma3d::DEPTH_STENCIL_VIEW_FLAG_NONE)
    {
        desc.flags = _flags;
        return *this;
    }
    const buma3d::DEPTH_STENCIL_VIEW_DESC& Get() const { return desc; }

private:
    buma3d::DEPTH_STENCIL_VIEW_DESC desc;

};


#pragma endregion resources



}// namespace util
}// namespace buma
