#include "./ResourceHeapAllocator.h"

#include <Buma3DHelpers/Buma3DHelpers.h>

namespace buma
{

#pragma region ResourceHeapAllocationPage

ResourceHeapAllocationPage::ResourceHeapAllocationPage(ResourceHeapAllocator& _owner, const RESOURCE_HEAP_PAGE_DESC& _desc)
    : owner                 { _owner }
    , allocation_manager    {}
    , heap                  {}
    , is_enabled_map        {}
{
    buma3d::RESOURCE_HEAP_DESC heap_desc{};
    heap_desc.heap_index            = _desc.heap_index;
    heap_desc.size_in_bytes         = _desc.page_size;
    heap_desc.alignment             = _desc.alignment;
    heap_desc.flags                 = buma3d::RESOURCE_HEAP_FLAG_NONE;
    heap_desc.creation_node_mask    = buma3d::B3D_DEFAULT_NODE_MASK;
    heap_desc.visible_node_mask     = buma3d::B3D_DEFAULT_NODE_MASK;
    auto bmr = owner.owner.device->CreateResourceHeap(heap_desc, &heap);
    BMR_ASSERT(bmr);
    std::stringstream ss;
    ss << "ResourceHeapAllocationPage::heap : ";
    ss << "heap index: " << _desc.heap_index;
    ss << ", page size: " << _desc.page_size;
    ss << ", alignment: " << _desc.alignment;
    heap->SetName(ss.str().c_str());

    allocation_manager = std::make_unique<VariableSizeAllocationsManager>(_desc.page_size, _desc.min_alignment);

    if (_desc.is_enabled_map)
    {
        is_enabled_map = true;
        bmr = heap->Map();
        BMR_ASSERT(bmr);
    }
}

ResourceHeapAllocationPage::~ResourceHeapAllocationPage()
{
    allocation_manager.reset();
    if (heap && is_enabled_map)
        heap->Unmap();
}

bool ResourceHeapAllocationPage::Allocate(size_t _size, size_t _alignment, RESOURCE_HEAP_ALLOCATION* _dst_allocation)
{
    *_dst_allocation = { this, heap.Get(), allocation_manager->Allocate(_size, _alignment) };
    _dst_allocation->parent_page = this;
    if (!_dst_allocation->allocation)
        return false;

    _dst_allocation->parent_page    = this;
    _dst_allocation->alignment      = _alignment;
    _dst_allocation->aligned_offset = util::AlignUp(_dst_allocation->allocation.offset, _alignment);
    _dst_allocation->aligned_size   = util::AlignUp(_size, _alignment);
    BUMA_ASSERT((_dst_allocation->aligned_offset - _dst_allocation->allocation.offset) + _dst_allocation->aligned_size <= _dst_allocation->allocation.size);

    return true;
}

void ResourceHeapAllocationPage::Free(RESOURCE_HEAP_ALLOCATION& _allocation)
{
    BUMA_ASSERT(_allocation == true);
    allocation_manager->Free(_allocation.allocation);
}

void ResourceHeapAllocationPage::Reset()
{
    allocation_manager->Reset();
}

const ResourceHeapAllocationPage::RESOURCE_HEAP_PAGE_DESC&
ResourceHeapAllocationPage::GetHeapDesc() const
{
    return owner.page_desc;
}

bool ResourceHeapAllocationPage::IsFull() const
{
    return owner.page_desc.min_alignment > allocation_manager->GetMaxBlockSize();
}

#pragma endregion ResourceHeapAllocationPage

#pragma region ResourceHeapAllocator

ResourceHeapAllocator::ResourceHeapAllocator(ResourceHeapsAllocator& _owner, const ResourceHeapAllocationPage::RESOURCE_HEAP_PAGE_DESC& _page_desc)
    : owner             { _owner }
    , page_desc         { _page_desc }
    , pages             {}
    , available_pages   {}
    , current_page      {}

{
    AddNewPage();
    Reset();
}

ResourceHeapAllocator::~ResourceHeapAllocator()
{
    current_page = nullptr;
    available_pages.clear();
    pages.clear();
}

bool ResourceHeapAllocator::Allocate(size_t _size, size_t _alignment, RESOURCE_HEAP_ALLOCATION* _dst_allocation)
{
    if (!(current_page->Allocate(_size, _alignment, _dst_allocation)))
    {
        ChangePage();
        return current_page->Allocate(_size, _alignment, _dst_allocation);
    }
    return true;
}

void ResourceHeapAllocator::Free(RESOURCE_HEAP_ALLOCATION& _allocation)
{
    BUMA_ASSERT(_allocation == true);
    if (current_page != _allocation.parent_page)        
        available_pages.emplace(static_cast<ResourceHeapAllocationPage*>(_allocation.parent_page));
    reinterpret_cast<ResourceHeapAllocationPage*>(_allocation.parent_page)->Free(_allocation);
}

void ResourceHeapAllocator::Reset()
{
    available_pages.clear();
    for (auto& i : pages)
    {
        i->Reset();
        available_pages.emplace(i.get());
    }
    current_page = *available_pages.begin();
}

void ResourceHeapAllocator::ChangePage()
{
    if (current_page->IsFull())
        available_pages.erase(current_page);

    current_page = nullptr;

    if (available_pages.empty())
        AddNewPage();

    current_page = *available_pages.begin();
}

void ResourceHeapAllocator::AddNewPage()
{
    auto raw = (pages.emplace(std::make_unique<ResourceHeapAllocationPage>(*this, page_desc))).first->get();
    available_pages.emplace(raw);
}

#pragma endregion ResourceHeapAllocator

#pragma region ResourceHeapsAllocator

ResourceHeapsAllocator::ResourceHeapsAllocator(buma3d::IDeviceAdapter* _adapter, buma3d::IDevice* _device)
    : device        { _device }
    , allocations   {}
    , limits        {}
    , heap_props    {}
{
    heap_props.resize(_device->GetResourceHeapProperties(nullptr));
    _device->GetResourceHeapProperties(heap_props.data());

    _adapter->GetDeviceAdapterLimits(&limits);
}

ResourceHeapsAllocator::~ResourceHeapsAllocator()
{
    for (auto& i : allocations)
    {
        for (auto& j : i)
            j.reset();
    }
}

RESOURCE_HEAP_ALLOCATION ResourceHeapsAllocator::Allocate(size_t _size, size_t _alignment, uint32_t _heap_index)
{
    auto pool_size = util::NextPow2(_size + _alignment);
    auto pool_index = GetPoolIndex(pool_size);

    auto&& pool = allocations[pool_index];
    auto&& heap_allocator = pool[_heap_index];
    if (!heap_allocator)
    {
        ResourceHeapAllocationPage::RESOURCE_HEAP_PAGE_DESC desc{};
        desc.heap_index     = _heap_index;
        desc.page_size      = GetPageSizeFromPoolIndex(pool_index);
        desc.alignment      = limits.max_resource_heap_alignment;
        desc.min_alignment  = limits.min_resource_heap_alignment;
        desc.is_enabled_map = heap_props[_heap_index].flags & (buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE);
        heap_allocator = std::make_unique<ResourceHeapAllocator>(*this, desc);
    }

    RESOURCE_HEAP_ALLOCATION result{};
    heap_allocator->Allocate(_size, _alignment, &result);
    result.pool_index = (uint32_t)pool_index;
    return result;
}

void ResourceHeapsAllocator::Free(RESOURCE_HEAP_ALLOCATION& _allocation)
{
    auto heap_index = reinterpret_cast<ResourceHeapAllocationPage*>(_allocation.parent_page)->GetHeapDesc().heap_index;
    allocations[_allocation.pool_index][heap_index]->Free(_allocation);
}

void ResourceHeapsAllocator::Reset()
{
    for (auto& i : allocations)
    {
        for (auto& j : i)
        if (j)
            j->Reset();
    }
}

size_t ResourceHeapsAllocator::GetPoolIndexFromSize(size_t _x)
{
    size_t allocator_page_size = _x >> ALLOCATOR_INDEX_SHIFT;
    int bit_index = 0;
    bit_index = util::GetFirstBitIndex(allocator_page_size);
    return bit_index != -1 ? bit_index + 1 : 0;
}

size_t ResourceHeapsAllocator::GetPoolIndex(size_t _x)
{
    return GetPoolIndexFromSize(util::NextPow2(_x));
}

size_t ResourceHeapsAllocator::GetPageSizeFromPoolIndex(size_t _x)
{
    _x = (_x == 0) ? 0 : _x - 1; // clamp to zero
    return std::max<size_t>(MIN_PAGE_SIZE, size_t(1) << (_x + ALLOCATOR_INDEX_SHIFT));
}


#pragma endregion ResourceHeapsAllocator


}// namespace buma
