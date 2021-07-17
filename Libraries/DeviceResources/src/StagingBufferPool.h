#pragma once
#include <DeviceResources/DeviceResources.h>

#include <Utils/Utils.h>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <mutex>
#include <vector>

namespace buma
{

// 単一リソースのデータの一部
struct BUFFER_ALLOCATION_PART
{
    buma3d::IBuffer*            parent_resouce; // マップしたあるリソース
    void*                       map_data_part;  // マップしたあるリソースのデータの一部(バイト単位でオフセットされた)
    buma3d::GpuVirtualAddress   gpu_address;    // マップしたあるリソースのGPU仮想アドレス(バイト単位でオフセットされたアドレス)
    size_t                      data_offset;    // マップしたあるリソースのデータのアドレスオフセット
    size_t                      size_in_bytes;  // バイト数
};

class StagingBufferPool;
class BufferPageAllocator;

class BufferPage
{
    friend class StagingBufferPool;
    friend class BufferPageAllocator;

public:
    BufferPage(BufferPageAllocator& _owner, size_t _page_size);
    ~BufferPage();

    void                    Reset                       ();

    bool                    CheckFreeSpace              ();                                                   // 空き容量が256byte以下の場合ページの割当を止めます
    bool                    CheckIsAllocatable          (size_t _size_in_bytes, size_t _alignment);           // 引数を条件にサイズを確保できるか確認します
    bool                    CheckIsAllocatableAligned   (size_t _aligned_size_in_bytes, size_t _alignment);   // アライメントされたサイズを確保できるか確認します

    BUFFER_ALLOCATION_PART  Allocate                    (size_t _size_in_bytes, size_t _aligned_size_in_bytes, size_t _alignment);
    BUFFER_ALLOCATION_PART  AllocateUnsafe              (size_t _size_in_bytes, size_t _aligned_size_in_bytes, size_t _alignment);

    size_t                  GetPageSize                 () const { return page_size; }
    size_t                  GetOffset                   () const { return offset; }
                                                           
    bool                    IsFull                      () const { return is_full; }

    void Flush();
    void Invalidate();

private:
    BufferPageAllocator&                    owner;
    buma3d::util::Ptr<buma3d::IBuffer>      resource;
    void*                                   map_data_base_ptr;          // mapしたデータのベースポインタ
    buma3d::GpuVirtualAddress               gpu_virtual_address_base;   // GPU仮想アドレスの先頭 

    size_t                                  page_size;                  // 作成するリソースのサイズ
    size_t                                  offset;                     // Allocateした際に進めるオフセット

    bool                                    is_full;                    // almost full
    std::mutex                              allocate_mutex;

};

class BufferPageAllocator
{
    friend class BufferPage;

public:
    BufferPageAllocator(StagingBufferPool& _owner, size_t _size);
    ~BufferPageAllocator() { Reset(); }

    void Reset();

    std::shared_ptr<BufferPage> MakeAndGetNewBufferPage();
    void                        MakeNewBufferPage();
    std::shared_ptr<BufferPage> FindAllocatablePage     (size_t _aligned_size_in_bytes, size_t _alignment);
    void                        ChangeMainPage          (size_t _aligned_size_in_bytes, size_t _alignment); // main_buffer_pageを入れ替えます
    BUFFER_ALLOCATION_PART      Allocate                (size_t _size_in_bytes, size_t _alignment);         // 指定サイズの領域を割り当たBUFFER_ALLOCATION_PARTを返します

    const size_t                GetPageCount            () const { return buffer_pages.size(); }
    const size_t                GetTotalBufferSize      () const { return buffer_page_allocation_size * buffer_pages.size(); }

    bool                        IsAllocated             () const { return main_buffer_page.operator bool(); }

    void Flush();
    void Invalidate();

private:
    StagingBufferPool&                          owner;
    std::vector<std::shared_ptr<BufferPage>>    buffer_pages;
    std::vector<std::shared_ptr<BufferPage>>    available_buffer_pages;
    std::shared_ptr<BufferPage>                 main_buffer_page;

    const size_t                                buffer_page_allocation_size;

};

class StagingBufferPool
{
    friend class BufferPage;
    friend class BufferPageAllocator;

public:
    StagingBufferPool(DeviceResources& _dr, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_prop_flags, buma3d::BUFFER_USAGE_FLAGS _usage_flags, const size_t _min_page_size = util::Mib(8));
    ~StagingBufferPool();

    BUFFER_ALLOCATION_PART AllocateBufferPart(size_t _size_in_bytes, size_t _alignment);
    BUFFER_ALLOCATION_PART AllocateConstantBufferPart(size_t _size_in_bytes);

    void ResetPages();
    buma3d::util::Ptr<buma3d::IDevice> GetDevice() { return device; }
    void Flush();
    void Invalidate();
    void MakeVisible()
    {
        if (need_flush)
            Flush();
        else if (need_invalidate)
            Invalidate();
    }

    bool NeedFlush     () const { return need_flush; }
    bool NeedInvalidate() const { return need_invalidate; }

private:
    size_t GetPoolIndexFromSize(size_t _x);
    size_t GetPoolIndex(size_t _x);
    size_t GetPageSizeFromPoolIndex(size_t _x);

private:
    const size_t MIN_PAGE_SIZE;
    const size_t ALLOCATOR_INDEX_SHIFT;
    const size_t ALLOCATOR_POOL_COUNT;

private:
    DeviceResources&                                    dr;
    bool                                                need_flush;
    bool                                                need_invalidate;
    const buma3d::RESOURCE_HEAP_PROPERTIES*             heap_prop;
    buma3d::BUFFER_USAGE_FLAGS                          usage_flags;
    buma3d::util::Ptr<buma3d::IDevice>                  device;
    std::vector<std::unique_ptr<BufferPageAllocator>>   buffer_page_allocators;
    const buma3d::DEVICE_ADAPTER_LIMITS&                limits;

};


}// namespace buma
