#pragma once
#include "./VariableSizeAllocationsManager.h"

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <Utils/Utils.h>

#include <memory>
#include <array>
#include <unordered_set>

namespace buma
{

struct RESOURCE_HEAP_ALLOCATION
{
    operator bool() const { return allocation; }
    void*                                       parent_page; // ResourceHeapAllocationPage*
    buma3d::IResourceHeap*                      heap;
    VariableSizeAllocationsManager::ALLOCATION  allocation;
    size_t                                      alignment;
    size_t                                      aligned_offset;
    size_t                                      aligned_size;
    uint32_t                                    pool_index;
};

class ResourceHeapAllocator;
class ResourceHeapAllocationPage
{
public:
    struct RESOURCE_HEAP_PAGE_DESC
    {
        size_t      page_size;
        size_t      alignment;
        size_t      min_alignment;
        uint32_t    heap_index;
        bool        is_enabled_map;
    };

public:
    ResourceHeapAllocationPage(ResourceHeapAllocator& _owner, const RESOURCE_HEAP_PAGE_DESC& _desc);
    ResourceHeapAllocationPage(const ResourceHeapAllocationPage&) = delete;
    ~ResourceHeapAllocationPage();

    bool Allocate(size_t _size, size_t _alignment, RESOURCE_HEAP_ALLOCATION* _dst_allocation);
    void Free(RESOURCE_HEAP_ALLOCATION& _allocation);
    void Reset();
    const RESOURCE_HEAP_PAGE_DESC& GetHeapDesc() const;
    bool IsFull() const;

private:
    ResourceHeapAllocator&                          owner;
    std::unique_ptr<VariableSizeAllocationsManager> allocation_manager;
    buma3d::util::Ptr<buma3d::IResourceHeap>        heap;
    bool                                            is_enabled_map;

};

class ResourceHeapsAllocator;
class ResourceHeapAllocator
{
    friend class ResourceHeapAllocationPage;
public:
    ResourceHeapAllocator(ResourceHeapsAllocator& _owner, const ResourceHeapAllocationPage::RESOURCE_HEAP_PAGE_DESC& _page_desc);
    ResourceHeapAllocator(const ResourceHeapAllocator&) = delete;
    ~ResourceHeapAllocator();

    bool Allocate(size_t _size, size_t _alignment, RESOURCE_HEAP_ALLOCATION* _dst_allocation);
    void Free(RESOURCE_HEAP_ALLOCATION& _allocation);
    void Reset();

private:
    void ChangePage();
    void AddNewPage();

private:
    ResourceHeapsAllocator&                                             owner;
    ResourceHeapAllocationPage::RESOURCE_HEAP_PAGE_DESC                 page_desc;
    std::unordered_set<std::unique_ptr<ResourceHeapAllocationPage>>     pages;
    std::unordered_set<ResourceHeapAllocationPage*>                     available_pages;
    ResourceHeapAllocationPage*                                         current_page;

};

class ResourceHeapsAllocator
{
    friend class ResourceHeapAllocationPage;
    friend class ResourceHeapAllocator;

public:
    static constexpr size_t MIN_PAGE_SIZE           = util::Mib(128);
    static constexpr size_t ALLOCATOR_INDEX_SHIFT   = util::Log2Cexpr<size_t>(MIN_PAGE_SIZE);
    static constexpr size_t ALLOCATOR_POOL_COUNT    = sizeof(size_t) * 8 - ALLOCATOR_INDEX_SHIFT;
    static_assert((MIN_PAGE_SIZE & (MIN_PAGE_SIZE - 1)) == 0, "min_page_size size must be a power of 2");

public:
    ResourceHeapsAllocator(buma3d::IDeviceAdapter* _adapter, buma3d::IDevice* _device);
    ResourceHeapsAllocator(const ResourceHeapsAllocator&) = delete;
    ~ResourceHeapsAllocator();

    RESOURCE_HEAP_ALLOCATION Allocate(size_t _size, size_t _alignment, uint32_t _heap_index);
    void Free(RESOURCE_HEAP_ALLOCATION& _allocation);
    void Reset();

private:
    size_t GetPoolIndexFromSize(size_t _x);
    size_t GetPoolIndex(size_t _x);
    size_t GetPageSizeFromPoolIndex(size_t _x);

private:
    using HeapAllocationsByType = std::array<std::unique_ptr<ResourceHeapAllocator>, /*heap type bitsで表現可能な種類の最大数*/32>;
    using HeapAllocationsBySize = std::array<HeapAllocationsByType, ALLOCATOR_POOL_COUNT>;
    buma3d::util::Ptr<buma3d::IDevice>              device;
    HeapAllocationsBySize                           allocations;
    buma3d::DEVICE_ADAPTER_LIMITS                   limits;
    std::vector<buma3d::RESOURCE_HEAP_PROPERTIES>   heap_props;

};


}// namespace buma
