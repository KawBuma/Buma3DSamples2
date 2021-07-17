#include <SampleBase/SampleBase.h>

#include <DeviceResources/SwapChain.h>
#include <DeviceResources/CommandQueue.h>

#include <Buma3DHelpers/Buma3DHelpers.h>
#include <Buma3DHelpers/B3DDescHelpers.h>

#include <Utils/Definitions.h>

#ifdef BUMA_DEBUG
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_ALL
#else
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_INFO
#endif // BUMA_DEBUG
#include <Utils/Logger.h>

#include <ShaderTools/ShaderLoader.h>
#include <DeviceResources/ShaderToolsConv.h>

#include <SDL.h>
#include <SDL_syswm.h>

#ifdef _WIN32
#undef CreateWindow
#endif // _WIN32

#include <filesystem>

namespace buma
{

SampleBase::SampleBase(PlatformBase& _platform)
    : ApplicationBase(_platform)
    , window                  {}
    , dr                      {}
    , adapter                 {}
    , device                  {}
    , swapchain               {}
    , present_queue           {}
    , buffer_count            {}
    , buffer_format           {}
    , buffer_index            {}
    , current_fence_value     {}
    , submission_fence_values {}
    , submission_fence        {}
    , acquire_fence           {}
    , acquire_fence_to_cpu    {}
    , present_fence           {}
    , null_layout             {}
    , sampler_point           {}
    , sampler_linear          {}
    , sampler_aniso           {}
    , render_pass             {}
    , framebuffers            {}
    , set_layouts             {}
    , pipeline_layout         {}
    , descriptor_heap         {}
    , descriptor_pool         {}
    , descriptor_sets         {}
    , shader_modules          {}
    , pipeline                {}
    , command_allocators      {}
    , command_lists           {}
    , descriptor_update       {}
{
}

SampleBase::~SampleBase()
{
}

bool SampleBase::CreateWindow(uint32_t _w, uint32_t _h, const char* _title)
{
    BUMA_ASSERT(window == nullptr);

    WINDOW_DESC desc{};
    desc.window_title = _title;
    desc.width        = _w;
    desc.height       = _h;
    desc.offset_x     = INT_MAX;
    desc.offset_y     = INT_MAX;
    desc.style        = WINDOW_STYLE_WINDOWED;
    desc.state        = WINDOW_STATE_RESTORE;
    desc.flags        = WINDOW_FLAG_RESIZABLE;

    window = platform.CreateWindow(desc);

    return true;
}

bool SampleBase::CreateDeviceResources(const char* _library_dir)
{
    BUMA_ASSERT(dr == nullptr);

    DEVICE_RESOURCE_DESC desc{ INTERNAL_API_TYPE_VULKAN, IS_DEBUG };

    // 使用するAPIとデバッグの有効化をコマンドラインから取得します。
    if (platform.HasArgument("--api"))
    {
        auto api = platform.FindArgument("--api");
        ++api;
             if (*api == "d3d12")  desc.type = INTERNAL_API_TYPE_D3D12;
        else if (*api == "vulkan") desc.type = INTERNAL_API_TYPE_VULKAN;
    }
    if (platform.HasArgument("--b3d-debug"))
    {
        desc.is_enabled_debug = true;
    }

    dr = std::make_unique<DeviceResources>(desc, _library_dir);
    adapter = dr->GetAdapter();
    device  = dr->GetDevice();

    return true;
}

bool SampleBase::CreateSwapChain(const buma3d::SWAP_CHAIN_BUFFER_DESC& _buffer_desc, buma3d::SWAP_CHAIN_FLAGS _flags)
{
    BUMA_ASSERT(window != nullptr);
    BUMA_ASSERT(swapchain == nullptr);

#ifdef _WIN32
    SDL_SysWMinfo syswm{};
    SDL_GetWindowWMInfo((SDL_Window*)window->GetNativeHandle(), &syswm);

    buma3d::SURFACE_PLATFORM_DATA_WINDOWS data{};
    buma3d::SURFACE_DESC surface_desc{ buma3d::SURFACE_PLATFORM_DATA_TYPE_WINDOWS, &data };
    data.hinstance = syswm.info.win.hinstance;
    data.hwnd      = syswm.info.win.window;
#else
    // TODO: support non-windows
#endif // _WIN32

    swapchain = dr->CreateSwapChain(surface_desc, _buffer_desc, _flags);
    swapchain->SetName("SampleBase::swapchain");

    buffer_count = swapchain->GetBufferDesc().count;
    buffer_format = swapchain->GetBufferDesc().format_desc.format;

    present_queue = swapchain->GetPresentableQueue();

    return true;
}

void SampleBase::CreateNullDescriptorSetLayout()
{
    util::DescriptorSetLayoutDesc desc;
    desc.Finalize();
    auto bmr = dr->GetDevice()->CreateDescriptorSetLayout(desc.Get(), &null_layout);
    BMR_ASSERT(bmr);
    null_layout->SetName("SampleBase::null_layout");
}

void SampleBase::CreateDefaultSamplers()
{
    util::SamplerDesc samp_desc{};
    samp_desc
        .FilterStandard()
        .TextureAddressModes(buma3d::TEXTURE_ADDRESS_MODE_WRAP);

    samp_desc.TextureSampleModes(buma3d::TEXTURE_SAMPLE_MODE_POINT);
    auto bmr = dr->GetDevice()->CreateSampler(samp_desc.TextureSampleModes(buma3d::TEXTURE_SAMPLE_MODE_POINT).Get(), &sampler_point);
    BMR_ASSERT(bmr);

    samp_desc.TextureSampleModes(buma3d::TEXTURE_SAMPLE_MODE_LINEAR);
    bmr = dr->GetDevice()->CreateSampler(samp_desc.Get(), &sampler_linear);
    BMR_ASSERT(bmr);

    samp_desc.FilterAnisotropy((uint32_t)dr->GetDeviceAdapterLimits().max_sampler_anisotropy);
    bmr = dr->GetDevice()->CreateSampler(samp_desc.Get(), &sampler_aniso);
    BMR_ASSERT(bmr);

    sampler_point->SetName("SampleBase::sampler_point");
    sampler_linear->SetName("SampleBase::sampler_linear");
    sampler_aniso->SetName("SampleBase::sampler_aniso");
}

void SampleBase::CreateFencesForSwapChain()
{
    // FENCE_TYPE_GPU_TO_CPU, FENCE_TYPE_GPU_TO_GPU フェンスではシグナルされたかどうかをバイナリ値(2値)で判定するため、1つのフェンスで1つのシグナル待機のみを管理することが可能です。
    // このためバックバッファの数に応じて、またはフェンスにが必要な処理の数に応じて、複数のフェンスを作成する必要があります。
    auto bmr = dr->GetDevice()->CreateFence(buma3d::init::BinaryFenceDesc(), &acquire_fence);
    BMR_ASSERT(bmr);
    bmr = dr->GetDevice()->CreateFence(buma3d::init::BinaryCpuFenceDesc(), &acquire_fence_to_cpu);
    BMR_ASSERT(bmr);
    bmr = dr->GetDevice()->CreateFence(buma3d::init::BinaryFenceDesc(), &present_fence);
    BMR_ASSERT(bmr);

    // 代わりに、FENCE_TYPE_TIMELINEは待機するシグナル値を保存するための整数配列を割り当てます。
    // TIMELINEフェンスはシグナル操作と待機操作を値ベースで管理し、
    // 各バッファがどのフェンス値でシグナルされるかどうかを識別できるため、1つだけのフェンスで複数のシグナル待機を管理することが可能です。
    submission_fence_values.resize(buffer_count);
    bmr = dr->GetDevice()->CreateFence(buma3d::init::TimelineFenceDesc(), &submission_fence);
    BMR_ASSERT(bmr);

    acquire_fence->SetName("SampleBase::acquire_fence");
    acquire_fence_to_cpu->SetName("SampleBase::acquire_fence_to_cpu");
    submission_fence->SetName("SampleBase::submission_fence");
    present_fence->SetName("SampleBase::present_fence");
}

void SampleBase::CreateDescriptorUpdate()
{
    auto bmr = dr->GetDevice()->CreateDescriptorUpdate({}, &descriptor_update);
    BMR_ASSERT(bmr);
}

bool SampleBase::CreateDescriptorHeapAndPool()
{
    if (set_layouts.empty())
        return true;

    util::DescriptorSizes sizes;
    for (auto& i : set_layouts)
    {
        sizes.IncrementSizes(i.Get(), buffer_count);
    }

    sizes.Finalize();

    auto bmr = dr->GetDevice()->CreateDescriptorHeap(
        sizes.GetAsHeapDesc(buma3d::DESCRIPTOR_HEAP_FLAG_NONE, buma3d::B3D_DEFAULT_NODE_MASK)
        , &descriptor_heap);
    BMR_ASSERT(bmr);
    descriptor_heap->SetName("SampleBase::descriptor_heap");

    bmr = dr->GetDevice()->CreateDescriptorPool(
        sizes.GetAsPoolDesc(descriptor_heap.Get(), sizes.GetMaxSetsByTotalMultiplyCount(), buma3d::DESCRIPTOR_POOL_FLAG_NONE)
        , &descriptor_pool);
    BMR_ASSERT(bmr);

    descriptor_pool->SetName("SampleBase::descriptor_pool");
    return true;
}

bool SampleBase::AllocateDescriptorSets()
{
    std::vector<buma3d::IDescriptorSet*>         sets(buffer_count);
    std::vector<buma3d::IDescriptorSetLayout*>   layouts(buffer_count);
    size_t cnt = 0;
    for (auto& i : set_layouts)
        layouts[cnt++] = i.Get();

    buma3d::DESCRIPTOR_SET_ALLOCATE_DESC allocate_desc{};
    allocate_desc.num_descriptor_sets = buffer_count;
    allocate_desc.set_layouts = layouts.data();
    auto bmr = descriptor_pool->AllocateDescriptorSets(allocate_desc, sets.data());
    BMR_ASSERT(bmr);

    cnt = 0;
    descriptor_sets.reserve(buffer_count);
    for (auto& i : sets)
    {
        i->SetName(("SampleBase::descriptor_sets " + std::to_string(cnt++)).c_str());
        descriptor_sets.emplace_back().Attach(i);
    }
    return true;
}

bool SampleBase::CreateRenderPass()
{
    namespace b = buma3d;
    b::RENDER_PASS_DESC render_pass_desc{};

    b::ATTACHMENT_DESC attachment{};
    attachment.flags        = b::ATTACHMENT_FLAG_NONE;
    attachment.format       = buffer_format;
    attachment.sample_count = 1;
    attachment.load_op      = b::ATTACHMENT_LOAD_OP_CLEAR;
    attachment.store_op     = b::ATTACHMENT_STORE_OP_STORE;
    attachment.begin_state  = b::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE;
    attachment.end_state    = b::RESOURCE_STATE_PRESENT;

    b::ATTACHMENT_REFERENCE color_attachment_ref{};
    color_attachment_ref.attachment_index             = 0;
    color_attachment_ref.state_at_pass                = b::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE;
    color_attachment_ref.stencil_state_at_pass        = {};
    color_attachment_ref.input_attachment_aspect_mask = b::TEXTURE_ASPECT_FLAG_COLOR;

    b::SUBPASS_DESC subpass_desc{};
    subpass_desc.flags                          = b::SUBPASS_FLAG_NONE;
    subpass_desc.pipeline_bind_point            = b::PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.view_mask                      = 0x0;
    subpass_desc.num_color_attachments          = 1;
    subpass_desc.color_attachments              = &color_attachment_ref;
    subpass_desc.resolve_attachments            = nullptr;
    subpass_desc.depth_stencil_attachment       = nullptr;

    b::SUBPASS_DEPENDENCY dependencies[] = { {},{} };
    dependencies[0].src_subpass                 = b::B3D_SUBPASS_EXTERNAL;
    dependencies[0].dst_subpass                 = 0;
    dependencies[0].src_stage_mask              = b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE;
    dependencies[0].dst_stage_mask              = b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT;
    dependencies[0].src_access                  = b::RESOURCE_ACCESS_FLAG_NONE;
    dependencies[0].dst_access                  = b::RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE;
    dependencies[0].dependency_flags            = b::DEPENDENCY_FLAG_BY_REGION;
    dependencies[0].view_offset                 = 0;

    dependencies[1].src_subpass                 = 0;
    dependencies[1].dst_subpass                 = b::B3D_SUBPASS_EXTERNAL;
    dependencies[1].src_stage_mask              = b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT;
    dependencies[1].dst_stage_mask              = b::PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE;
    dependencies[1].src_access                  = b::RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE;
    dependencies[1].dst_access                  = b::RESOURCE_ACCESS_FLAG_NONE;
    dependencies[1].dependency_flags            = b::DEPENDENCY_FLAG_BY_REGION;
    dependencies[1].view_offset                 = 0;

    render_pass_desc.flags                      = b::RENDER_PASS_FLAG_NONE;
    render_pass_desc.num_attachments            = 1;
    render_pass_desc.attachments                = &attachment;
    render_pass_desc.num_subpasses              = 1;
    render_pass_desc.subpasses                  = &subpass_desc;
    render_pass_desc.num_dependencies           = _countof(dependencies);
    render_pass_desc.dependencies               = dependencies;
    render_pass_desc.num_correlated_view_masks  = 0;
    render_pass_desc.correlated_view_masks      = nullptr;

    auto bmr = dr->GetDevice()->CreateRenderPass(render_pass_desc, &render_pass);
    BMR_ASSERT(bmr);
    render_pass->SetName("SampleBase::render_pass");
    return true;
}

bool SampleBase::CreateFramebuffers()
{
    framebuffers.resize(buffer_count);
    auto&& back_buffers = swapchain->GetBuffers();
    buma3d::FRAMEBUFFER_DESC fb_desc{};
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        buma3d::IView* attachment = back_buffers[i].rtv;
        fb_desc.flags           = buma3d::FRAMEBUFFER_FLAG_NONE;
        fb_desc.render_pass     = render_pass.Get();
        fb_desc.num_attachments = 1;
        fb_desc.attachments     = &attachment;

        auto bmr = dr->GetDevice()->CreateFramebuffer(fb_desc, &framebuffers[i]);
        BMR_ASSERT(bmr);
        framebuffers[i]->SetName(("SampleBase::framebuffers " + std::to_string(i)).c_str());
    }
    return true;
}

bool SampleBase::CreatePipelineLayout()
{
    util::PipelineLayoutDesc desc((uint32_t)set_layouts.size(), 0);
    desc
        .SetNumLayouts((uint32_t)set_layouts.size())
        .SetLayouts(0, set_layouts)
        .SetFlags(buma3d::PIPELINE_LAYOUT_FLAG_NONE)
        .Finalize();

    auto bmr = dr->GetDevice()->CreatePipelineLayout(desc.Get(), &pipeline_layout);
    BMR_ASSERT(bmr);
    pipeline_layout->SetName("SampleBase::pipeline_layout");

    return true;
}

bool SampleBase::CreateCommandAllocator()
{
    uint32_t cnt = 0;
    command_allocators.resize(buffer_count);
    for (auto& i : command_allocators)
    {
        buma3d::COMMAND_ALLOCATOR_DESC cad{};
        cad.type = buma3d::COMMAND_TYPE_DIRECT;
        cad.level = buma3d::COMMAND_LIST_LEVEL_PRIMARY;
        cad.flags = buma3d::COMMAND_ALLOCATOR_FLAG_NONE;

        auto bmr = dr->GetDevice()->CreateCommandAllocator(cad, &i);
        BMR_ASSERT(bmr);
        i->SetName(("SampleBase::command_allocators " + std::to_string(cnt++)).c_str());
    }

    return true;
}

bool SampleBase::CreateCommandLists()
{
    command_lists.resize(buffer_count);
    buma3d::COMMAND_LIST_DESC cld{};
    cld.type = present_queue->GetCommandType();
    cld.level = buma3d::COMMAND_LIST_LEVEL_PRIMARY;
    cld.node_mask = buma3d::B3D_DEFAULT_NODE_MASK;
    uint32_t cnt = 0;
    for (auto& i : command_lists)
    {
        cld.allocator = command_allocators[cnt].Get();
        auto bmr = dr->GetDevice()->AllocateCommandList(cld, &i);
        BMR_ASSERT(bmr);
        i->SetName(std::string("SampleBase::command_lists " + std::to_string(cnt++)).c_str());
    }

    return true;
}

buma3d::IShaderModule* SampleBase::CreateShaderModule(const char* _path, buma3d::SHADER_STAGE_FLAG _stage, const char* _entry_point)
{
    // シェーダをコンパイル
    shader::LOAD_SHADER_DESC desc{};
    desc.filename    = _path;
    desc.entry_point = _entry_point;
    desc.defines     = {};
    desc.stage       = shader::ConvertShaderStage(_stage);

    desc.options.pack_matrices_in_row_major = false;
    desc.options.enable_16bit_types         = false;
    desc.options.enable_debug_info          = IS_DEBUG;
    desc.options.disable_optimizations      = false;
    desc.options.optimization_level         = 3; // 0 to 3, no optimization to most optimization
    desc.options.shader_model               = { 6, 4 };

    std::vector<uint8_t> bytecode;
    shader::ShaderLoader::LoadShaderFromHLSL(shader::ConvertApiType(dr->GetApiType()), desc, &bytecode);
    BUMA_ASSERT(!bytecode.empty());

    // シェーダモジュールを作成
    buma3d::SHADER_MODULE_DESC module_desc{};
    module_desc.flags                    = buma3d::SHADER_MODULE_FLAG_NONE;
    module_desc.bytecode.bytecode_length = bytecode.size();
    module_desc.bytecode.shader_bytecode = bytecode.data();

    buma3d::IShaderModule* result{};
    auto bmr = dr->GetDevice()->CreateShaderModule(module_desc, &result);
    BMR_ASSERT(bmr);

    auto&& s = std::filesystem::path(_path).filename().string();
    result->SetName(s.c_str());
    BUMA_LOGI("Created shader module {}", s.c_str());

    return result;
}

void SampleBase::DestroySampleBaseObjects()
{
    command_lists.clear();
    command_allocators.clear();

    shader_modules.clear();
    pipeline.Reset();

    framebuffers.clear();
    render_pass.Reset();

    pipeline_layout.Reset();
    set_layouts.clear();

    descriptor_update.Reset();
    descriptor_sets.clear();
    descriptor_pool.Reset();
    descriptor_heap.Reset();

    submission_fence_values.clear();
    submission_fence.Reset();
    acquire_fence.Reset();
    acquire_fence_to_cpu.Reset();
    present_fence.Reset();

    null_layout.Reset();
    sampler_point.Reset();
    sampler_linear.Reset();
    sampler_aniso.Reset();

    if (swapchain)
    {
        dr->DestroySwapChain(swapchain);
        swapchain = nullptr;
    }
    swapchain = nullptr;

    present_queue = nullptr;
    device.Reset();
    adapter.Reset();
    dr.reset();

    if (window)
    {
        platform.DestroyWindow(window);
        window = nullptr;
    }
}


}// namespace buma
