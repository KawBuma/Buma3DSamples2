#include <DeviceResources/Resource.h>

#include <DeviceResources/DeviceResources.h>

#include "./ResourceHeapProperties.h"
#include "./ResourceHeapAllocator.h"

#include <Utils/Utils.h>

namespace buma
{

ResourceBase::ResourceBase(DeviceResources& _dr, RESOURCE_CREATE_TYPE _create_type)
    : dr              { _dr }
    , create_type     { _create_type }
    , resource        {}
    , heap_allocation {}
{
}

ResourceBase::~ResourceBase()
{
    resource.Reset();
    if (heap_allocation && *heap_allocation)
    {
        dr.GetResourceHeapsAllocator()->Free(*heap_allocation);
        delete heap_allocation;
        heap_allocation = nullptr;
    }
}

bool ResourceBase::CreateResource(const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags)
{
    auto&& d = dr.GetDevice();

    switch (create_type)
    {
    case buma::RESOURCE_CREATE_TYPE_COMMITTED:
    {
        buma3d::RESOURCE_ALLOCATION_INFO        info{};
        buma3d::RESOURCE_HEAP_ALLOCATION_INFO   heap_info{};
        auto bmr = dr.GetDevice()->GetResourceAllocationInfo(1, resource.GetAddressOf(), &info, &heap_info);
        BMR_ASSERT(bmr);

        uint32_t compatible_heap_bits = dr.GetResourceHeapProperties()->FindCompatibleHeaps(_heap_flags, _deny_heap_flags);
        BUMA_ASSERT((compatible_heap_bits & heap_info.heap_type_bits) == 0x0);

        buma3d::COMMITTED_RESOURCE_DESC desc{};
        desc.heap_index         = (uint32_t)util::GetFirstBitIndex(compatible_heap_bits & heap_info.heap_type_bits);
        desc.heap_flags         = buma3d::RESOURCE_HEAP_FLAG_NONE;
        desc.creation_node_mask = buma3d::B3D_DEFAULT_NODE_MASK;
        desc.visible_node_mask  = buma3d::B3D_DEFAULT_NODE_MASK;
        desc.resource_desc      = _desc;
        bmr = d->CreateCommittedResource(desc, &resource);
        BMR_ASSERT(bmr);
        break;
    }

    case buma::RESOURCE_CREATE_TYPE_PLACED:
    {
        auto bmr = d->CreatePlacedResource(_desc, &resource);
        BMR_ASSERT(bmr);

        auto result = AllocateHeap(_heap_flags, _deny_heap_flags);
        BUMA_ASSERT(result);

        result = Bind();
        BUMA_ASSERT(result);
        break;
    }

    case buma::RESOURCE_CREATE_TYPE_RESERVED:
    {
        BUMA_ASSERT(false && "TODO: RESOURCE_CREATE_TYPE_RESERVED");
        break;
    }

    case buma::RESOURCE_CREATE_TYPE_SWAP_CHAIN:
    {
        /* DO NOTHING */
        break;
    }

    default:
        BUMA_ASSERT(false);
        break;
    }

    return true;
}

bool ResourceBase::AllocateHeap(buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags)
{
    buma3d::RESOURCE_ALLOCATION_INFO        info{};
    buma3d::RESOURCE_HEAP_ALLOCATION_INFO   heap_info{};
    auto bmr = dr.GetDevice()->GetResourceAllocationInfo(1, resource.GetAddressOf(), &info, &heap_info);
    if (util::IsFailed(bmr))
        return false;

    uint32_t compatible_heap_bits = dr.GetResourceHeapProperties()->FindCompatibleHeaps(_heap_flags, _deny_heap_flags);
    if ((compatible_heap_bits & heap_info.heap_type_bits) == 0x0)
        return false;

    heap_allocation = new RESOURCE_HEAP_ALLOCATION{};
    *heap_allocation = dr.GetResourceHeapsAllocator()->Allocate(heap_info.total_size_in_bytes, heap_info.required_alignment, (uint32_t)util::GetFirstBitIndex(compatible_heap_bits & heap_info.heap_type_bits));
    return heap_allocation;
}

bool ResourceBase::Bind()
{
    buma3d::BIND_RESOURCE_HEAP_INFO bi{};
    bi.src_heap            = heap_allocation->heap;
    bi.src_heap_offset     = heap_allocation->aligned_offset;
    bi.num_bind_node_masks = 0;
    bi.bind_node_masks     = nullptr;
    bi.dst_resource        = resource.Get();

    auto bmr = dr.GetDevice()->BindResourceHeaps(1, &bi);
    if (util::IsFailed(bmr))
        return false;

    return true;
}


}// namespace buma
