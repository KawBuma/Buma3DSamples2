#include "./StagingBufferPool.h"
#include "./ResourceHeapProperties.h"

#include <Buma3DHelpers/B3DInit.h>
#include <Buma3DHelpers/Buma3DHelpers.h>

#ifdef _DEBUG
#define BMTEXT(_x) 

#else
#define BMTEXT(_x) 

#endif

namespace buma
{

#pragma region BufferPage

BufferPage::BufferPage(BufferPageAllocator& _owner, size_t _page_size)
    : owner                     { _owner }
    , resource                  {}
    , map_data_base_ptr         {}
    , gpu_virtual_address_base  {}
    , page_size                 { _page_size }
    , offset                    {}
    , is_full                   {}
{
    auto desc = buma3d::init::CommittedResourceDesc(owner.owner.heap_prop->heap_index, buma3d::RESOURCE_HEAP_FLAG_NONE
                                                    , buma3d::init::BufferResourceDesc(_page_size, owner.owner.usage_flags));

    auto bmr = owner.owner.device->CreateCommittedResource(desc, &resource);
    BMR_ASSERT(bmr);

    resource->SetName(("BufferPage size: " + std::to_string(_page_size)).c_str());

    bmr = resource->GetHeap()->Map();
    BMR_ASSERT(bmr);

    buma3d::MAPPED_RANGE range{};
    bmr = resource->GetHeap()->GetMappedData(&range, &map_data_base_ptr);
    BMR_ASSERT(bmr);

    gpu_virtual_address_base = resource->GetGPUVirtualAddress();
}

BufferPage::~BufferPage()
{
    resource->GetHeap()->Unmap();
    map_data_base_ptr        = nullptr;
    gpu_virtual_address_base = 0;
    page_size                = 0;
    offset                   = 0;
}

void BufferPage::Reset() 
{
    offset = 0; 
}

bool BufferPage::CheckFreeSpace()
{
    if (page_size - offset < owner.owner.limits.min_constant_buffer_offset_alignment)
        is_full = true;

    return is_full;
}

bool BufferPage::CheckIsAllocatable(size_t _size_in_bytes, size_t _alignment)
{
    auto aligned_size   = util::AlignUp(_size_in_bytes, _alignment);
    auto aligned_offset = util::AlignUp(offset         , _alignment);

    // 作成したリソースのサイズを超えない場合true
    return (aligned_offset + aligned_size) <= page_size;
}

bool BufferPage::CheckIsAllocatableAligned(size_t _aligned_size_in_bytes, size_t _alignment)
{
    return (_aligned_size_in_bytes + util::AlignUp(offset, _alignment)) <= page_size;
}

BUFFER_ALLOCATION_PART BufferPage::Allocate(size_t _size_in_bytes, size_t _aligned_size_in_bytes, size_t _alignment)
{
    //auto aligned_size = util::AlignUp(_size_in_bytes, _alignment);
    auto aligned_offset = util::AlignUp(offset, _alignment);

    std::lock_guard<std::mutex> allocate_guard(allocate_mutex);

    // サイズを超える場合nulptrを返す
    if (CheckIsAllocatableAligned(_aligned_size_in_bytes, aligned_offset))
    {
        offset = aligned_offset + _aligned_size_in_bytes;

        return BUFFER_ALLOCATION_PART{
              resource.Get()                                                 
            , static_cast<unsigned char*>(map_data_base_ptr) + aligned_offset
            , gpu_virtual_address_base                       + aligned_offset
            , aligned_offset                                                 
            , _size_in_bytes                                                 
        };
    }
    else
    {
        CheckFreeSpace();

        return BUFFER_ALLOCATION_PART{
              resource.Get()
            , nullptr
            , 0
            , 0
            , 0
        };
    }
}

BUFFER_ALLOCATION_PART BufferPage::AllocateUnsafe(size_t _size_in_bytes, size_t _aligned_size_in_bytes, size_t _alignment)
{
    //auto aligned_size    = util::AlignUp(_size_in_bytes, _alignment);
    auto aligned_offset = util::AlignUp(offset, _alignment);

    std::lock_guard<std::mutex> allocate_guard(allocate_mutex);

    offset = aligned_offset + _aligned_size_in_bytes;
    return BUFFER_ALLOCATION_PART{
          resource.Get()
        , static_cast<unsigned char*>(map_data_base_ptr) + aligned_offset
        , gpu_virtual_address_base                       + aligned_offset
        , aligned_offset
        , _size_in_bytes
    };
}

void BufferPage::Flush()
{
    if (offset != 0)
    {
        buma3d::MAPPED_RANGE range{ 0, offset };
        auto bmr = resource->GetHeap()->FlushMappedRanges(1, &range);
        BMR_ASSERT(bmr);
    }
}

void BufferPage::Invalidate()
{
    if (offset != 0)
    {
        buma3d::MAPPED_RANGE range{ 0, offset };
        auto bmr = resource->GetHeap()->InvalidateMappedRanges(1, &range);
        BMR_ASSERT(bmr);
    }
}

#pragma endregion BufferPage

#pragma region BufferPageAllocator

BufferPageAllocator::BufferPageAllocator(StagingBufferPool& _owner, size_t _size)
    : owner                         { _owner }
    , buffer_page_allocation_size   { _size }
{

}

void BufferPageAllocator::Reset()
{
    if (buffer_pages.empty())
        return;

    for (auto&& i : buffer_pages)
        i->Reset();

    // available_buffer_pagesのbackを設定
    available_buffer_pages  = buffer_pages;
    main_buffer_page        = available_buffer_pages.back();
    available_buffer_pages.pop_back();
}

std::shared_ptr<BufferPage> BufferPageAllocator::MakeAndGetNewBufferPage()
{
    auto new_buffer_page = std::make_shared<BufferPage>(*this, buffer_page_allocation_size);
    buffer_pages.emplace_back(std::move(new_buffer_page));
    BMTEXT("BufferPageAllocator - Allocated size: " + std::to_string(buffer_page_allocation_size) + ", this size total: " + std::to_string(buffer_pages.size()));
    return buffer_pages.back();
}
void BufferPageAllocator::MakeNewBufferPage()
{
    auto new_buffer_page = std::make_shared<BufferPage>(*this, buffer_page_allocation_size);
    buffer_pages.emplace_back(std::move(new_buffer_page));
}

BUFFER_ALLOCATION_PART BufferPageAllocator::Allocate(size_t _size_in_bytes, size_t _alignment)
{
    if (main_buffer_page == false)
        main_buffer_page = MakeAndGetNewBufferPage();

    auto aligned_size = util::AlignUp(_size_in_bytes, _alignment);
    BUFFER_ALLOCATION_PART buffer_part = main_buffer_page->Allocate(_size_in_bytes, aligned_size, _alignment);

    if (buffer_part.map_data_part == nullptr)
    {
        // main_buffer_pageの空き容量がなくなった場合に最優先のページを変更する
        if (main_buffer_page->IsFull() == false)
        {
            ChangeMainPage(aligned_size, _alignment);
            buffer_part = main_buffer_page->AllocateUnsafe(_size_in_bytes, aligned_size, _alignment);
        }
        else
        {
            auto page = FindAllocatablePage(aligned_size, _alignment);
            buffer_part = page->AllocateUnsafe(_size_in_bytes, aligned_size, _alignment);
        }
    }

    return buffer_part;
}

void BufferPageAllocator::Flush()
{
    for (auto& i : buffer_pages)
        i->Flush();
}

void BufferPageAllocator::Invalidate()
{
    for (auto& i : buffer_pages)
        i->Invalidate();
}

std::shared_ptr<BufferPage> BufferPageAllocator::FindAllocatablePage(size_t _aligned_size_in_bytes, size_t _alignment)
{
    //size_t aligned_size = util::AlignUp(_size_in_bytes, _alignment);

    for (auto&& i : available_buffer_pages)
        if (i->CheckIsAllocatableAligned(_aligned_size_in_bytes, _alignment) == true)
            return i;

    // 見つからなかった場合
    return MakeAndGetNewBufferPage();
}

void BufferPageAllocator::ChangeMainPage(size_t _aligned_size_in_bytes, size_t _alignment)
{
    //size_t aligned_size = util::AlignUp(_size_in_bytes, _alignment);

    auto* pages = available_buffer_pages.data();
    auto  size    = available_buffer_pages.size();
    for (size_t i = 0; i < size; i++)
    {
        auto&& page = pages[i];
        if (page->CheckIsAllocatableAligned(_aligned_size_in_bytes, _alignment) == true)
        {
            main_buffer_page = page;
            util::EraseContainerElem(available_buffer_pages, i); // i番目をmain_buffer_pageに渡した後に除外(buffer_pagesが所有)
            return;
        }
    }

    // 見つからなかった場合
    main_buffer_page = MakeAndGetNewBufferPage();
}

#pragma endregion BufferPageAllocator

#pragma region StagingBufferPool

StagingBufferPool::StagingBufferPool(DeviceResources& _dr, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_prop_flags, buma3d::BUFFER_USAGE_FLAGS _usage_flags, const size_t _min_page_size)
    : MIN_PAGE_SIZE             { _min_page_size }
    , ALLOCATOR_INDEX_SHIFT     { util::Log2(MIN_PAGE_SIZE) }
    , ALLOCATOR_POOL_COUNT      { sizeof(size_t) * 8 - ALLOCATOR_INDEX_SHIFT }
    , dr                        { _dr }
    , need_flush                {}
    , need_invalidate           {}
    , heap_prop                 {}
    , usage_flags               { _usage_flags }
    , device                    { _dr.GetDevice() }
    , buffer_page_allocators    {}
    , limits                    { _dr.GetDeviceAdapterLimits() }
{
    assert((MIN_PAGE_SIZE& (MIN_PAGE_SIZE - 1)) == 0 && "MIN_PAGE_SIZE size must be a power of 2");

    // READ_BACKとUPLAODは同時に行いません。
    BUMA_ASSERT((_heap_prop_flags& (buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE))
                != (buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE));

    buma3d::util::Ptr<buma3d::IBuffer> res;
    device->CreatePlacedResource(buma3d::init::BufferResourceDesc(512, usage_flags), &res);

    if (_heap_prop_flags & buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE)
    {
        auto&& readable_heaps = dr.GetResourceHeapProperties()->GetHostReadableHeaps();
        heap_prop = readable_heaps.Filter(buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT).Find(res.Get());
        if (!heap_prop)
        {
            heap_prop = readable_heaps.Find(res.Get());
            need_flush = true;
        }
    }

    else if (_heap_prop_flags & buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE)
    {
        auto&& writable_heaps = dr.GetResourceHeapProperties()->GetHostWritableHeaps();
        heap_prop = writable_heaps.Filter(buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT).Find(res.Get());
        if (!heap_prop)
        {
            heap_prop = writable_heaps.Find(res.Get());
            need_invalidate = true;
        }
    }

    if (!heap_prop)
        throw std::runtime_error("StagingBufferPool: heap_prop not found.");

    int counter = 0;
    buffer_page_allocators.resize(ALLOCATOR_POOL_COUNT);
    for (auto&& i : buffer_page_allocators)
    {
        auto page_size = GetPageSizeFromPoolIndex(counter);
        i = std::make_unique<BufferPageAllocator>(*this, page_size);
        counter++;
    }
}

StagingBufferPool::~StagingBufferPool()
{
    ResetPages();
}

BUFFER_ALLOCATION_PART StagingBufferPool::AllocateBufferPart(size_t _size_in_bytes, size_t _alignment)
{
    auto pool_size  = util::NextPow2(_size_in_bytes + _alignment);
    auto pool_index = GetPoolIndex(pool_size);

    auto& pool = buffer_page_allocators[pool_index];
    BUMA_ASSERT(pool != nullptr);

    return pool->Allocate(_size_in_bytes, _alignment);
}

BUFFER_ALLOCATION_PART StagingBufferPool::AllocateConstantBufferPart(size_t _size_in_bytes)
{
    return AllocateBufferPart(_size_in_bytes, limits.min_constant_buffer_offset_alignment);
}

void StagingBufferPool::ResetPages()
{
    for (auto&& i : buffer_page_allocators)
        i->Reset();
}

void StagingBufferPool::Flush()
{
    for (auto&& i : buffer_page_allocators)
        i->Flush();
}

void StagingBufferPool::Invalidate()
{
    for (auto&& i : buffer_page_allocators)
        i->Invalidate();
}

size_t StagingBufferPool::GetPoolIndexFromSize(size_t _x)
{
    size_t allocator_page_size = _x >> ALLOCATOR_INDEX_SHIFT;
    int bit_index = 0;
    bit_index = util::GetFirstBitIndex(allocator_page_size);
    return bit_index != -1 ? bit_index + 1 : 0;
}

size_t StagingBufferPool::GetPoolIndex(size_t _x)
{
    return GetPoolIndexFromSize(util::NextPow2(_x));
}

size_t StagingBufferPool::GetPageSizeFromPoolIndex(size_t _x)
{
    _x = (_x == 0) ? 0 : _x - 1; // clamp to zero
    return std::max<size_t>(MIN_PAGE_SIZE, size_t(1) << (_x + ALLOCATOR_INDEX_SHIFT));
}

#pragma endregion StagingBufferPool


}// namespace buma
