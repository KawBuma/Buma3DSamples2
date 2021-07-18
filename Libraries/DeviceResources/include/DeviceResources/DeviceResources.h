#pragma once

#include <memory>
#include <vector>
#include <string>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

namespace buma
{

class Buffer;
class Texture;
class SwapChain;
class CommandQueue;

class ResourceHeapProperties;
class ResourceHeapsAllocator;

class CopyContext;

enum INTERNAL_API_TYPE
{
      INTERNAL_API_TYPE_D3D12
    , INTERNAL_API_TYPE_VULKAN
};

struct DEVICE_RESOURCE_DESC
{
    INTERNAL_API_TYPE   type;
    bool                use_performance_adapter; // trueの場合、adapter_indexは無視され、高パフォーマンスアダプタを優先します。 
    uint32_t            adapter_index;
    bool                is_enabled_debug;
};

class DeviceResources
{
public:
    // _library_dir Buma3D_D3D12 または Buma3D_Vulkan が存在するディレクトリを指定します。 
    DeviceResources(const DEVICE_RESOURCE_DESC& _desc, const char* _library_dir);
    ~DeviceResources();

public:
    Buffer* CreateBuffer(const buma3d::RESOURCE_DESC& _desc
                         , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL
                         , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_COPY_DST_FIXED);

    Texture* CreateTexture(const buma3d::RESOURCE_DESC& _desc
                           , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL
                           , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_COPY_DST_FIXED);

    SwapChain* CreateSwapChain(const buma3d::SURFACE_DESC& _surface_desc, const buma3d::SWAP_CHAIN_BUFFER_DESC& _buffer, buma3d::SWAP_CHAIN_FLAGS _flags);

    void DestroyBuffer(Buffer* _buffer);
    void DestroyTexture(Texture* _texture);
    void DestroySwapChain(SwapChain* _swapchain);

    void BeginFrame();
    void EndFrame();

    CopyContext& GetCopyContext();

    /**
     * @brief キューに追加されたコマンドリストを送信します 
     * @param _queue 送信するコマンドが存在するコマンドキューを指定します
     * @return このSubmitが完了するタイミングでシグナルされる値を返します
    */
    uint64_t QueueSubmit(CommandQueue& _queue);

    bool WaitForGpu();

public:
    INTERNAL_API_TYPE                                           GetApiType()                                        const { return desc.type; }
    const buma3d::DEVICE_ADAPTER_LIMITS&                        GetDeviceAdapterLimits()                            const { return limits; }
    const std::shared_ptr<ResourceHeapProperties>&              GetResourceHeapProperties()                         const { return resource_heap_props; }
    const std::vector<buma3d::COMMAND_QUEUE_PROPERTIES>&        GetQueueProperties()                                const { return queue_props; }
    //const std::vector<std::shared_ptr<buma::GpuTimerPool>>&   GetGpuTimerPool(const buma3d::COMMAND_TYPE _type)   const { return direct_gpu_timer_pools[_type]; }

    const buma3d::util::Ptr<buma3d::IDeviceFactory>&            GetFactory()                                    const { return factory; }
    const buma3d::util::Ptr<buma3d::IDeviceAdapter>&            GetAdapter()                                    const { return adapter; }
    const buma3d::util::Ptr<buma3d::IDevice>&                   GetDevice()                                     const { return device; }
    const std::vector<CommandQueue*>&                           GetCommandQueues(buma3d::COMMAND_TYPE _type)    const { return cmd_queues[_type]; }
    ResourceHeapsAllocator*                                     GetResourceHeapsAllocator()                     const { return resource_heaps_allocator.get(); }

private:
    bool Init(const DEVICE_RESOURCE_DESC& _desc, const char* _library_dir);
    bool LoadB3D(INTERNAL_API_TYPE _type, const char* _library_dir);
    bool InitB3D();
    bool CreateDeviceFactory();
    bool PickAdapter();
    bool CreateDevice();
    bool GetCommandQueues();
    void UninitB3D();
    CommandQueue* GetPresentableQueue(buma3d::ISurface* _surface);

private:
    DEVICE_RESOURCE_DESC                                    desc;

    struct B3D_PFN;
    std::unique_ptr<B3D_PFN>                                pfn;
    buma3d::util::Ptr<buma3d::IDeviceFactory>               factory;
    buma3d::util::Ptr<buma3d::IDeviceAdapter>               adapter;
    buma3d::util::Ptr<buma3d::IDevice>                      device;
    buma3d::DEVICE_ADAPTER_LIMITS                           limits;

    std::vector<CommandQueue*>                              cmd_queues[buma3d::COMMAND_TYPE_NUM_TYPES];         // [COMMAND_TYPE]
    std::vector<buma3d::COMMAND_QUEUE_PROPERTIES>           queue_props;

    std::unique_ptr<ResourceHeapsAllocator>                 resource_heaps_allocator;
    std::shared_ptr<ResourceHeapProperties>                 resource_heap_props;

    //std::vector<std::shared_ptr<buma::GpuTimerPool>>      gpu_timer_pools[buma3d::COMMAND_TYPE_NUM_TYPES];    // [COMMAND_TYPE]

    std::unique_ptr<CopyContext>                            copy_context;

    uint64_t                                                frame_value;

};


} // namespace buma
