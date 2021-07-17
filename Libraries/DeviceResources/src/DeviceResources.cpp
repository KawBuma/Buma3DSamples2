#include <DeviceResources/DeviceResources.h>
#include <DeviceResources/ResourceTexture.h>
#include <DeviceResources/ResourceBuffer.h>
#include <DeviceResources/SwapChain.h>
#include <DeviceResources/CopyContext.h>
#include <DeviceResources/CommandQueue.h>

#include "./ResourceHeapAllocator.h"
#include "./ResourceHeapProperties.h"

#ifdef BUMA_DEBUG
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_ALL
#else
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_INFO
#endif // BUMA_DEBUG
#include <Utils/Logger.h>
#include <spdlog/fmt/ostr.h>

#include <Buma3DHelpers/B3DDescHelpers.h>

#include <filesystem>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // _WIN32

namespace buma
{

namespace /*anonymous*/
{

template<typename T>
struct LOGFMT : public T
{
    static const LOGFMT<T>& F(const T& c) { return reinterpret_cast<const LOGFMT<T>&>(c); }

    template<typename OStream>
    friend OStream& operator<<(OStream& os, const T& _d);
};
template<typename T>
const LOGFMT<T>& TOFMT(const T& c) { return LOGFMT<T>::F(c); }

const char* GetCommandTypeStr(buma3d::COMMAND_TYPE _type)
{
    switch (_type)
    {
    case buma3d::COMMAND_TYPE_DIRECT        : return "DIRECT";
    case buma3d::COMMAND_TYPE_DIRECT_ONLY   : return "DIRECT_ONLY";
    case buma3d::COMMAND_TYPE_COMPUTE_ONLY  : return "COMPUTE_ONLY";
    case buma3d::COMMAND_TYPE_COPY_ONLY     : return "COPY_ONLY";
    case buma3d::COMMAND_TYPE_VIDEO_DECODE  : return "VIDEO_DECODE";
    case buma3d::COMMAND_TYPE_VIDEO_PROCESS : return "VIDEO_PROCESS";
    case buma3d::COMMAND_TYPE_VIDEO_ENCODE  : return "VIDEO_ENCODE";
    default: return "unknown";
    }
}

template<typename OStream>
OStream& operator<<(OStream& os, buma3d::DEVICE_ADAPTER_TYPE _t)
{
    switch (_t)
    {
    case buma3d::DEVICE_ADAPTER_TYPE_OTHER            : os << "DEVICE_ADAPTER_TYPE_OTHER";          break;
    case buma3d::DEVICE_ADAPTER_TYPE_INTEGRATED_GPU   : os << "DEVICE_ADAPTER_TYPE_INTEGRATED_GPU"; break;
    case buma3d::DEVICE_ADAPTER_TYPE_DISCRETE_GPU     : os << "DEVICE_ADAPTER_TYPE_DISCRETE_GPU";   break;
    case buma3d::DEVICE_ADAPTER_TYPE_VIRTUAL_GPU      : os << "DEVICE_ADAPTER_TYPE_VIRTUAL_GPU";    break;
    case buma3d::DEVICE_ADAPTER_TYPE_CPU              : os << "DEVICE_ADAPTER_TYPE_CPU";            break;
    default:
        os << "unknown";
        break;
    }
    return os;
}

template<typename OStream>
OStream& operator<<(OStream& os, const buma3d::DEVICE_ADAPTER_DESC& _d)
{
    os
    <<   "\tdevice_name             : " << _d.device_name
    << "\n\tvendor_id               : " << _d.vendor_id
    << "\n\tdevice_id               : " << _d.device_id
    << "\n\tadapter_luid            : " << util::GetLUIDString(_d.adapter_luid)
    << "\n\tdedicated_video_memory  : " << _d.dedicated_video_memory
    << "\n\tshared_system_memory    : " << _d.shared_system_memory
    << "\n\tnode_count              : " << _d.node_count
    << "\n\tadapter_type            : " << _d.adapter_type;
    return os;
}

void B3D_APIENTRY B3DMessageCallback(buma3d::DEBUG_MESSAGE_SEVERITY _severity, buma3d::DEBUG_MESSAGE_CATEGORY_FLAG _category, const buma3d::Char8T* const _message, void* _user_data)
{
    switch (_severity)
    {
    case buma3d::DEBUG_MESSAGE_SEVERITY_INFO:       buma::log::info    ("[b3d] {}", _message); break;
    case buma3d::DEBUG_MESSAGE_SEVERITY_WARNING:    buma::log::warn    ("[b3d] {}", _message); break;
    case buma3d::DEBUG_MESSAGE_SEVERITY_ERROR:      buma::log::error   ("[b3d] {}", _message); break;
    case buma3d::DEBUG_MESSAGE_SEVERITY_CORRUPTION: buma::log::critical("[b3d] {}", _message); break;
    case buma3d::DEBUG_MESSAGE_SEVERITY_OTHER:      buma::log::debug   ("[b3d] {}", _message); break;
    default:
        break;
    }
}


} // namespace /*anonymous*/


struct DeviceResources::B3D_PFN
{
    void*                                       b3d_module;
    buma3d::PFN_Buma3DInitialize                Buma3DInitialize;
    buma3d::PFN_Buma3DGetInternalHeaderVersion  Buma3DGetInternalHeaderVersion;
    buma3d::PFN_Buma3DCreateDeviceFactory       Buma3DCreateDeviceFactory;
    buma3d::PFN_Buma3DUninitialize              Buma3DUninitialize;
};

DeviceResources::DeviceResources(const DEVICE_RESOURCE_DESC& _desc, const char* _library_dir)
    : desc                     {}
    , pfn                      {}
    , factory                  {}
    , adapter                  {}
    , device                   {}
    , limits                   {}
    , cmd_queues               {}    
    , queue_props              {}
    , resource_heaps_allocator {}
    , resource_heap_props      {}
//  , gpu_timer_pools          {}
    , copy_context             {}
    , frame_value              {}
{
    Init(_desc, _library_dir);
}

DeviceResources::~DeviceResources()
{
    BUMA_LOGI("Deinitialize DeviceResources");
    WaitForGpu();
    copy_context.reset();
    resource_heaps_allocator.reset();
    resource_heap_props.reset();
    UninitB3D();
}

bool DeviceResources::Init(const DEVICE_RESOURCE_DESC& _desc, const char* _library_dir)
{
    BUMA_LOGI("Initialize DeviceResources");

    desc = _desc;
    pfn = std::make_unique<B3D_PFN>();
    if (!LoadB3D(desc.type, _library_dir))              return false;
    if (!InitB3D())                                     return false;
    if (!CreateDeviceFactory())                         return false;
    if (!PickAdapter())                                 return false;
    if (!CreateDevice())                                return false;
    if (!GetCommandQueues())                            return false;

    resource_heap_props      = std::make_shared<ResourceHeapProperties>(device.Get());
    resource_heaps_allocator = std::make_unique<ResourceHeapsAllocator>(adapter.Get(), device.Get());

    auto copy_context_type = buma3d::COMMAND_TYPE_DIRECT;
    copy_context = std::make_unique<CopyContext>(*this, copy_context_type);

    return true;
}

buma::Buffer* DeviceResources::CreateBuffer(const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags)
{
    return new Buffer(*this, RESOURCE_CREATE_TYPE_PLACED, _desc, _heap_flags, _deny_heap_flags);
}

buma::Texture* DeviceResources::CreateTexture(const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags)
{
    return new Texture(*this, RESOURCE_CREATE_TYPE_PLACED, _desc, _heap_flags, _deny_heap_flags);
}

SwapChain* DeviceResources::CreateSwapChain(const buma3d::SURFACE_DESC& _surface_desc, const buma3d::SWAP_CHAIN_BUFFER_DESC& _buffer, buma3d::SWAP_CHAIN_FLAGS _flags)
{
    buma3d::util::Ptr<buma3d::ISurface> surface{};
    auto bmr = adapter->CreateSurface(_surface_desc, &surface);
    BMR_ASSERT(bmr);

    CommandQueue* queue = GetPresentableQueue(surface.Get());
    if (!queue)
    {
        BUMA_LOGC("Presentable command queue not found");
    }

    return new SwapChain(*this, _buffer, _flags, surface.Get(), queue);
}

CommandQueue* DeviceResources::GetPresentableQueue(buma3d::ISurface* _surface)
{
    CommandQueue* queue = nullptr;
    for (auto& i : cmd_queues)
    {
        if (i.empty())
            continue;

        auto&& que = i.front();
        auto support_present = adapter->QueryPresentationSupport(que->GetCommandType(), _surface);
        if (util::IsSucceeded(support_present))
        {
            queue = que;
            BUMA_LOGI("Presentable queue found at queue type {}", GetCommandTypeStr(que->GetCommandType()));
            break;
        }
    }

    return queue;
}

void DeviceResources::DestroyBuffer(buma::Buffer* _buffer)
{
    delete _buffer;
}

void DeviceResources::DestroyTexture(buma::Texture* _texture)
{
    delete _texture;
}

void DeviceResources::DestroySwapChain(SwapChain* _swapchain)
{
    delete _swapchain;
}

void DeviceResources::BeginFrame()
{
    // TODO: BeginFrame()
}

void DeviceResources::EndFrame()
{
    // TODO: EndFrame()
    frame_value++;
}

CopyContext& DeviceResources::GetCopyContext()
{
    copy_context->Begin();
    return *copy_context;
}

uint64_t DeviceResources::QueueSubmit(CommandQueue& _queue)
{
    BUMA_LOGT("Submitting queued commands");

    if (copy_context->HasCommand())
    {
        BUMA_LOGT("Add copy context");
        auto&& copy_submit_info = copy_context->End();

        if (copy_context->GetCommandType() == _queue.GetCommandType())
        {
            BUMA_LOGT("Copy context submission");
            _queue.PrependCommandList(copy_submit_info.command_lists_to_execute[0]);
            _queue.PrependSignalFence(copy_submit_info.signal_fence.fences[0], copy_submit_info.signal_fence.fence_values[0]);
        }
        else
        {
            BUMA_LOGT("Copy-queue context submission");
            buma3d::SUBMIT_DESC sd{ 1, &copy_submit_info };
            auto bmr = GetCommandQueues(copy_context->GetCommandType()).front()->GetCommandQueue()->Submit(sd);
            BMR_ASSERT(bmr);
        }
    }

    uint64_t value_at_completion = _queue.SubmitFromDr();

    BUMA_LOGT("Submitted queued commands");

    return value_at_completion;
}

bool DeviceResources::LoadB3D(INTERNAL_API_TYPE _type, const char* _library_dir)
{
    std::filesystem::path path(_library_dir ? _library_dir : ".");

    switch (_type)
    {
    case buma::INTERNAL_API_TYPE_D3D12  : path /= "Buma3D_D3D12";  log::info("Buma3D: Runs on D3D12 API"); break;
    case buma::INTERNAL_API_TYPE_VULKAN : path /= "Buma3D_Vulkan"; log::info("Buma3D: Runs on Vulkan API"); break;
    default:
        break;
    }

#ifdef _WIN32
    // dllを読み込む
    pfn->b3d_module = LoadLibrary(path.c_str());
#else
    // TODO: Support non-windows
#endif // _WIN32

    BUMA_ASSERT(pfn->b3d_module != NULL);
    return pfn->b3d_module != NULL;
}

bool DeviceResources::InitB3D()
{
    BUMA_LOGI("Initialize Buma3D");
    pfn->Buma3DInitialize               = (buma3d::PFN_Buma3DInitialize)              GetProcAddress((HMODULE)pfn->b3d_module, "Buma3DInitialize");
    pfn->Buma3DGetInternalHeaderVersion = (buma3d::PFN_Buma3DGetInternalHeaderVersion)GetProcAddress((HMODULE)pfn->b3d_module, "Buma3DGetInternalHeaderVersion");
    pfn->Buma3DCreateDeviceFactory      = (buma3d::PFN_Buma3DCreateDeviceFactory)     GetProcAddress((HMODULE)pfn->b3d_module, "Buma3DCreateDeviceFactory");
    pfn->Buma3DUninitialize             = (buma3d::PFN_Buma3DUninitialize)            GetProcAddress((HMODULE)pfn->b3d_module, "Buma3DUninitialize");

    buma3d::ALLOCATOR_DESC b3d_desc{};
    b3d_desc.is_enabled_allocator_debug = false;
    b3d_desc.custom_allocator           = nullptr;

    auto bmr = pfn->Buma3DInitialize(b3d_desc);
    BUMA_ASSERT(bmr == buma3d::BMRESULT_SUCCEED);
    return bmr == buma3d::BMRESULT_SUCCEED;
}

bool DeviceResources::CreateDeviceFactory()
{
    buma3d::DEVICE_FACTORY_DESC fac_desc{};
    fac_desc.flags            = buma3d::DEVICE_FACTORY_FLAG_NONE;
    fac_desc.debug.is_enabled = desc.is_enabled_debug;

    if (fac_desc.debug.is_enabled || IS_DEBUG)
    {
        fac_desc.debug.debug_message_callback.user_data = nullptr;
        fac_desc.debug.debug_message_callback.Callback  = &B3DMessageCallback;
    }

    // デバッグメッセージの構成
    buma3d::DEBUG_MESSAGE_DESC descs[buma3d::DEBUG_MESSAGE_SEVERITY_END]{};
    for (size_t i = 0; i < buma3d::DEBUG_MESSAGE_SEVERITY_END; i++)
    {
        auto&& debug_desc = descs[i];
        debug_desc.severity               = buma3d::DEBUG_MESSAGE_SEVERITY(i);
        debug_desc.category_flags         = buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_ALL & (~buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_PERFORMANCE);
        debug_desc.is_enabled_debug_break = false;// debug_desc.severity == buma3d::DEBUG_MESSAGE_SEVERITY_ERROR; // レポート時のブレイク
    }
    fac_desc.debug.num_debug_messages = ARRAYSIZE(descs);
    fac_desc.debug.debug_messages     = descs;

    // GPU検証機能の設定
    fac_desc.debug.gpu_based_validation.is_enabled  = false;
    fac_desc.debug.gpu_based_validation.flags       = buma3d::GPU_BASED_VALIDATION_FLAG_NONE;

    auto bmr = pfn->Buma3DCreateDeviceFactory(fac_desc, &factory);
    if (bmr != buma3d::BMRESULT_SUCCEED)
        return false;

    return true;
}

bool DeviceResources::PickAdapter()
{
    // 高パフォーマンスアダプタを取得
    uint32_t cnt = 0;
    uint64_t max_vram = 0;
    buma3d::util::Ptr<buma3d::IDeviceAdapter> adapter_tmp{};
    while (factory->EnumAdapters(cnt++, &adapter_tmp) != buma3d::BMRESULT_FAILED_OUT_OF_RANGE)
    {
        auto&& adapter_desc = adapter_tmp->GetDesc();
        if (max_vram < adapter_desc.dedicated_video_memory)
        {
            max_vram = adapter_desc.dedicated_video_memory;
            adapter = adapter_tmp;
        }
        BUMA_LOGI("Found adapter: \n {}", TOFMT(adapter_desc));
    }
    if (!adapter)
        return false;

    auto&& d = adapter->GetDesc();
    BUMA_LOGI("Picked adapter: \n {}", TOFMT(d));
    adapter->GetDeviceAdapterLimits(&limits);
    return true;
}

bool DeviceResources::CreateDevice()
{
    // デバイス作成
    buma3d::DEVICE_DESC dd{};
    dd.adapter = adapter.Get();

    // コマンドキューの情報を取得。
    auto num_queue_tyeps = dd.adapter->GetCommandQueueProperties(nullptr);// サイズを返します。
    queue_props.resize(num_queue_tyeps);
    adapter->GetCommandQueueProperties(queue_props.data());

    // コマンドキュー作成情報を構成
    std::vector<buma3d::COMMAND_QUEUE_CREATE_DESC>           queue_descs     (num_queue_tyeps);
    std::vector<std::vector<buma3d::COMMAND_QUEUE_PRIORITY>> queue_priorities(num_queue_tyeps);
    std::vector<std::vector<buma3d::NodeMask>>               queue_node_masks(num_queue_tyeps);
    for (uint32_t i = 0; i < num_queue_tyeps; i++)
    {
        auto&& qd = queue_descs[i];
        qd.type  = queue_props[i].type;
        qd.flags = buma3d::COMMAND_QUEUE_FLAG_NONE;

        /*
        HACK: プロファイラーのクラッシュを防ぐためにコマンドキューは必要最低限のインスタンス数で作成します。 
              コマンドキューを複数作成すると、プロファイラー(PIX,RenderDoc,Nsight)がコマンドの解析時にほとんどクラッシュする現象を確認しました。
              複数のキューを作成し、そのうちの1つしか使用しない場合でも、クラッシュが発生します。
              何らかの特殊なケースではない限り、コマンドキューの数は各タイプ毎に1つとします。 
        */
        //qd.num_queues = std::min(std::thread::hardware_concurrency(), queue_props[i].num_max_queues);
        qd.num_queues = 1;

        auto&& qps  = queue_priorities[i];
        auto&& qnms = queue_node_masks[i];
        qps .resize(qd.num_queues, buma3d::COMMAND_QUEUE_PRIORITY_DEFAULT);
        qnms.resize(qd.num_queues, buma3d::B3D_DEFAULT_NODE_MASK);
        qd.priorities = qps.data();
        qd.node_masks = qnms.data();
    }
    dd.num_queue_create_descs = (uint32_t)queue_descs.size();
    dd.queue_create_descs     = queue_descs.data();
    dd.flags                  = buma3d::DEVICE_FLAG_NONE;

    auto bmr = factory->CreateDevice(dd, &device);
    BUMA_ASSERT(bmr == buma3d::BMRESULT_SUCCEED);
    return bmr == buma3d::BMRESULT_SUCCEED;
}

bool DeviceResources::GetCommandQueues()
{
    for (uint32_t i = 0; i < uint32_t(buma3d::COMMAND_TYPE_NUM_TYPES); i++)
    {
        buma3d::util::Ptr<buma3d::ICommandQueue> ptr;
        uint32_t cnt = 0;
        auto&& queues = cmd_queues[i];
        while (device->GetCommandQueue(buma3d::COMMAND_TYPE(i), cnt++, &ptr) == buma3d::BMRESULT_SUCCEED)
        {
            queues.emplace_back(new CommandQueue(*this, ptr));
        }

        if (!queues.empty())
        {
            BUMA_LOGI("Found command queue type {}", GetCommandTypeStr(buma3d::COMMAND_TYPE(i)));
        }
    }
    return true;
}

void DeviceResources::UninitB3D()
{
    if (!pfn || !pfn->b3d_module)
        return;

    BUMA_LOGI("Uninitialize Buma3D");
    //gpu_timer_pools.reset();

    for (auto&& i_que : cmd_queues)
        for (auto&& i : i_que)
            util::SafeDelete(i);
    device.Reset();
    adapter.Reset();
    factory.Reset();

    pfn->Buma3DUninitialize();

#ifdef _WIN32
    FreeLibrary((HMODULE)pfn->b3d_module);
    pfn->b3d_module = NULL;
#endif // _WIN32

    pfn.reset();
}

bool DeviceResources::WaitForGpu()
{
    if (!device)
        return false;
    return device->WaitIdle() == buma3d::BMRESULT_SUCCEED;
}


}// namespace buma
