#include <DeviceResources/ResourceBuffer.h>

#include <ResourceHeapAllocator.h>

#include <Buma3DHelpers/B3DInit.h>

namespace buma
{

Buffer::Buffer(DeviceResources& _dr, RESOURCE_CREATE_TYPE _create_type
               , const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS  _deny_heap_flags)
    : ResourceBase(_dr, _create_type)
    , mapped_data{}
    , mapped_range{}
    , srvs{}
    , uavs{}
{
    CreateResource(_desc, _heap_flags, _deny_heap_flags);

    if (_heap_flags & (buma3d::init::HEAP_HOST_VISIBLE_FLAGS))
    {
        auto bmr = resource->GetHeap()->GetMappedData(&mapped_range, &mapped_data);
        BMR_ASSERT(bmr);

        if (heap_allocation)
        {
            mapped_data         = static_cast<uint8_t*>(mapped_data) + heap_allocation->aligned_offset;
            mapped_range.offset = heap_allocation->aligned_offset;
            mapped_range.size   = heap_allocation->aligned_size;
        }
    }

    srvs.Init(GetB3DBuffer().Get());
    uavs.Init(GetB3DBuffer().Get());
}

Buffer::~Buffer()
{
}

void* Buffer::GetMppedData()
{
    return mapped_data;
}

const buma3d::MAPPED_RANGE* Buffer::GetMppedRange() const
{
    return (mapped_data ? &mapped_range : nullptr);
}

void Buffer::Flush(const buma3d::MAPPED_RANGE* _range)
{
    if (!mapped_data)
        return;

    auto bmr = resource->GetHeap()->FlushMappedRanges(1, &mapped_range);
    BMR_ASSERT(bmr);
}

void Buffer::Invalidate(const buma3d::MAPPED_RANGE* _range)
{
    if (!mapped_data)
        return;

    auto bmr = resource->GetHeap()->InvalidateMappedRanges(1, &mapped_range);
    BMR_ASSERT(bmr);
}

buma3d::IShaderResourceView* Buffer::GetSRV(const buma3d::SHADER_RESOURCE_VIEW_DESC& _desc)
{
    return srvs.GetOrCreate(_desc);
}

buma3d::IUnorderedAccessView* Buffer::GetUAV(const buma3d::UNORDERED_ACCESS_VIEW_DESC& _desc)
{
    return uavs.GetOrCreate(_desc);
}


}// namespace buma
