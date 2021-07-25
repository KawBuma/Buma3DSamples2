#include <HelloTriangle/HelloTriangle.h>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <Buma3DHelpers/Buma3DHelpers.h>
#include <Buma3DHelpers/B3DDescHelpers.h>
#include <Buma3DHelpers/B3DInit.h>

#include <DeviceResources/DeviceResources.h>
#include <DeviceResources/CommandQueue.h>

#include <Utils/Definitions.h>
#include <Utils/Utils.h>
#ifdef BUMA_DEBUG
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_ALL
#else
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_INFO
#endif // BUMA_DEBUG
#include <Utils/Logger.h>

#include <ShaderTools/ShaderLoader.h>
#include <DeviceResources/ShaderToolsConv.h>

#include <type_traits>
#include <vector>
#include <filesystem>

#include <SDL.h>
#include <SDL_syswm.h>
#undef CreateWindow

#define RET_IF_FAILED(x) { if (!x) return false; }
#define BMR_RET_IF_FAILED(x) if (buma::util::IsFailed(x)) { BUMA_ASSERT(false && #x); return false; }

//#define BUMA_ASSERT(x)

namespace /*anonymous*/
{

std::vector<float>* g_fpss  = nullptr;
bool                g_first = true;

}// namespace /*anonymous*/


namespace init = buma3d::init;
namespace b = buma3d;

template<typename T>
using Ptr = buma3d::util::Ptr<T>;


namespace buma
{

const buma3d::RESOURCE_HEAP_PROPERTIES* FindMappableHeap(const std::vector<buma3d::RESOURCE_HEAP_PROPERTIES>& _heap_props)
{
    for (auto& i : _heap_props) {
        if (i.flags & buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE &&
            i.flags & buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT &&
            !(i.flags & buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED))
            return &i;
    }
    return nullptr;
}

const buma3d::RESOURCE_HEAP_PROPERTIES* FindDeviceLocalHeap(const std::vector<buma3d::RESOURCE_HEAP_PROPERTIES>& _heap_props)
{
    for (auto& i : _heap_props) {
        if (i.flags & buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL)
            return &i;
    }
    return nullptr;
}

HelloTriangle::HelloTriangle(PlatformBase& _platform)
    : ApplicationBase(_platform)
    , window                {}
    , dr                    {}
    , adapter               {}
    , device                {}
    , command_queue         {}
    , surface               {}
    , swapchain             {}
    , back_buffers          {}
    , back_buffer_rtvs      {}
    , back_buffer_format    {}
    , back_buffer_index     {}
    , cmd_fences            {}
    , acquire_fence         {}
    , render_complete_fence {}
    , render_pass           {}
    , pipeline_layout       {}
    , framebuffers          {}
    , shader_modules        {}
    , pipeline              {}
    , cmd_allocator         {}
    , cmd_lists             {}
    , heap_props            {}
    , resource_heap         {}
    , vertex_buffer         {}
    , index_buffer          {}
    , vertex_buffer_src     {}
    , index_buffer_src      {}
    , vertex_buffer_view    {}
    , index_buffer_view     {}
    , is_reuse_commands     {}
{
    g_fpss = new std::remove_pointer_t<decltype(g_fpss)>;
}

HelloTriangle::~HelloTriangle()
{
    delete g_fpss;
    g_fpss = nullptr;
}

void HelloTriangle::OnKeyUp(KeyUpEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    if (_args.key->keysym.scancode == SDL_SCANCODE_Z)
    {
        is_reuse_commands = !is_reuse_commands;
        BUMA_LOGI(is_reuse_commands ? "Reuse recorded commands" : "Re-record commands every frame");
    }
}

void HelloTriangle::OnProcessWindow(ProcessWindowEventArgs& _args)
{
    if (_args.process_flags & WINDOW_PROCESS_FLAG_RESIZED)
    {
        uint32_t w, h;
        _args.window->GetWindowedSize(&w, &h);
        OnResize(w, h);
    }
}

bool HelloTriangle::OnInit()
{
    PrepareSettings();

    CreateWindow(1280, 720, "HelloTriangle - Press the Z key to toggle whether to reuse command lists");

    if (!CreateDeviceResources())   return false;
    if (!InitSwapChain())           return false;
    if (!CreateRenderTargetViews()) return false;
    if (!LoadAssets())              return false;
    if (!CreatePipelineLayout())    return false;
    if (!CreateRenderPass())        return false;
    if (!CreateFramebuffers())      return false;
    if (!CreateShaderModules())     return false;
    if (!CreateGraphicsPipelines()) return false;
    if (!CreateCommandAllocator())  return false;
    if (!CreateCommandLists())      return false;
    if (!CreateFences())            return false;

    b::RESOURCE_HEAP_ALLOCATION_INFO         heap_alloc_info{};
    std::vector<b::RESOURCE_ALLOCATION_INFO> alloc_infos;
    if (!CreateBuffers())                                   return false;
    if (!CreateHeaps(&heap_alloc_info, &alloc_infos))       return false;
    if (!BindResourceHeaps(&heap_alloc_info, &alloc_infos)) return false;
    if (!CreateBuffersForCopy())                            return false;
    if (!CopyBuffers())                                     return false;
    if (!CreateBufferViews())                               return false;

    // 初期化時のフレームはレンダリングが完了している状態と同等のため、各フェンスを事前にシグナルします。
    for (auto& i : cmd_fences)
    {
        buma3d::SUBMIT_SIGNAL_DESC sd{};
        sd.signal_fence_to_cpu = i.Get();
        command_queue->SubmitSignal(sd);
    }

    // 描画コマンドを記録
    for (uint32_t i = 0; i < BUFFER_COUNT; i++)
        PrepareFrame(i, 0.f);

    return true;
}

void HelloTriangle::Tick(const util::StepTimer& _timer)
{
    if (_timer.IsOneSecElapsed())
    {
        if (!g_first)
            g_fpss->emplace_back(static_cast<float>(_timer.GetFramesPerSecond()));
        g_first = false;

        BUMA_LOGI(("fps: " + std::to_string(_timer.GetFramesPerSecond())).c_str());
    }

    float delta = static_cast<float>(_timer.GetElapsedSeconds());
    Update(delta);
    Render(delta);
}

void HelloTriangle::OnDestroy()
{
    dr->WaitForGpu();

    // result
    if (!g_first)
    {
        g_first = true;
        float res = 0.f;
        float size = static_cast<float>(g_fpss->size());
        for (auto& i : *g_fpss)
            res += i;
        BUMA_LOGI("prof result: average fps {}", res / size);
    }

    // オブジェクトの解放
    cmd_lists.clear();
    cmd_allocator.clear();

    vertex_buffer_view.Reset();
    index_buffer_view.Reset();
    vertex_buffer_src.Reset();
    index_buffer_src.Reset();
    vertex_buffer.Reset();
    index_buffer.Reset();
    resource_heap.Reset();
    heap_props.clear();

    cmd_fences.clear();
    acquire_fence.Reset();
    render_complete_fence.Reset();

    shader_modules.clear();
    pipeline.Reset();
    pipeline_layout.Reset();

    render_pass.Reset();
    framebuffers.clear();

    back_buffer_rtvs.clear();
    back_buffers.clear();
    swapchain.Reset();
    surface.Reset();

    command_queue.Reset();
    device.Reset();
    adapter.Reset();

    dr.reset();

    if (window)
    {
        platform.DestroyWindow(window);
        window = nullptr;
    }
}

void HelloTriangle::CreateWindow(uint32_t _w, uint32_t _h, const char* _title)
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
}

bool HelloTriangle::CreateDeviceResources()
{
    BUMA_ASSERT(dr == nullptr);

    DEVICE_RESOURCE_DESC desc{ INTERNAL_API_TYPE_VULKAN, false, 0, IS_DEBUG };

    // 使用するAPIとデバッグの有効化をコマンドラインから取得します。
    if (platform.HasArgument("--api"))
    {
        auto api = platform.FindArgument("--api");
        ++api;
             if (*api == "d3d12")  desc.type = INTERNAL_API_TYPE_D3D12;
        else if (*api == "vulkan") desc.type = INTERNAL_API_TYPE_VULKAN;
    }
    if (platform.HasArgument("--adapter"))
    {
        auto adp = ++(platform.FindArgument("--adapter"));
        if (*adp == "performance")
            desc.use_performance_adapter = true;
        else
            desc.adapter_index = std::stoul(*adp);
    }
    if (platform.HasArgument("--debug-b3d"))
    {
        desc.is_enabled_debug = true;
    }

    dr = std::make_unique<DeviceResources>(desc, nullptr);
    adapter = dr->GetAdapter();
    device  = dr->GetDevice();

    return true;
}

bool HelloTriangle::InitSwapChain()
{
    // サーフェスを作成
    SDL_SysWMinfo info{};
    SDL_GetWindowWMInfo((SDL_Window*)window->GetNativeHandle(), &info);
    buma3d::SURFACE_PLATFORM_DATA_WINDOWS data{ info.info.win.hinstance, info.info.win.window };
    buma3d::SURFACE_DESC sd{ b::SURFACE_PLATFORM_DATA_TYPE_WINDOWS, &data };
    auto bmr = adapter->CreateSurface(sd, &surface);
    BMR_ASSERT(bmr);

    // プレゼントを実行するコマンドキューを取得
    for (uint32_t i = 0; i < (uint32_t)buma3d::COMMAND_TYPE_NUM_TYPES; i++)
    {
        if (adapter->QueryPresentationSupport(buma3d::COMMAND_TYPE(i), surface.Get()) == b::BMRESULT_SUCCEED)
        {
            device->GetCommandQueue(buma3d::COMMAND_TYPE(i), 0, &command_queue);
            break;
        }
    }

    // 利用可能なサーフェスフォーマットを取得
    std::vector<b::SURFACE_FORMAT> surface_formats(surface->GetSupportedSurfaceFormats(nullptr));
    surface->GetSupportedSurfaceFormats(surface_formats.data());
    auto&& sf = surface_formats.front();

    // スワップチェインを作成
    auto scd = init::SwapChainDesc(surface.Get(), sf.color_space,
                                   init::SwapChainBufferDesc(0, 0, BUFFER_COUNT, { sf.format }, b::SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT),
                                   command_queue.GetAddressOf());
    window->GetWindowedSize(&scd.buffer.width, &scd.buffer.height);
    scd.flags |= b::SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT;
    scd.flags |= b::SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC;

    back_buffer_format = sf.format;

    bmr = device->CreateSwapChain(scd, &swapchain);
    BMR_ASSERT(bmr);

    return true;
}

bool HelloTriangle::CreateRenderTargetViews()
{
    // バックバッファRTVを作成
    back_buffers.resize(BUFFER_COUNT);
    back_buffer_rtvs.resize(BUFFER_COUNT);
    for (uint32_t i = 0; i < BUFFER_COUNT; i++)
    {
        swapchain->GetBuffer(i, &back_buffers[i]);

        util::RenderTargetViewDesc desc(back_buffer_format);
        desc.AsTextureView().ArraySize(0, 1);
        auto bmr = device->CreateRenderTargetView(back_buffers[i].Get(), desc.Get(), &back_buffer_rtvs[i]);
        BMR_ASSERT(bmr);
    }
    return true;
}

bool HelloTriangle::CreateFramebuffers()
{
    // フレームバッファを作成します
    // フレームバッファの作成はレンダーパスに依存します
    framebuffers.resize(BUFFER_COUNT);
    b::FRAMEBUFFER_DESC fb_desc{};
    for (uint32_t i = 0; i < BUFFER_COUNT; i++)
    {
        b::IView* attachment    = back_buffer_rtvs[i].Get();
        fb_desc.flags           = b::FRAMEBUFFER_FLAG_NONE;
        fb_desc.render_pass     = render_pass.Get();
        fb_desc.num_attachments = 1;
        fb_desc.attachments     = &attachment;

        auto bmr = device->CreateFramebuffer(fb_desc, &framebuffers[i]);
        BMR_RET_IF_FAILED(bmr);
    }
    return true;
}

bool HelloTriangle::LoadAssets()
{
    auto aspect_ratio = window->GetAspectRatio();
    triangle = {
          { {  0.0f ,  0.25f * aspect_ratio, 0.0f, 1.f }, { 1.f, 0.f, 0.f, 1.f} }
        , { {  0.25f, -0.25f * aspect_ratio, 0.0f, 1.f }, { 0.f, 1.f, 0.f, 1.f} }
        , { { -0.25f, -0.25f * aspect_ratio, 0.0f, 1.f }, { 0.f, 0.f, 1.f, 1.f} }
    };
    index = { 0,1,2 };

    return true;
}

#pragma region preparing Buma3D objects

bool HelloTriangle::CreatePipelineLayout()
{
    // このサンプルで使用するリソースはありません。
    b::PIPELINE_LAYOUT_DESC pld{};
    pld.flags               = b::PIPELINE_LAYOUT_FLAG_NONE;
    pld.num_set_layouts     = 0;
    pld.set_layouts         = nullptr;
    pld.num_push_constants  = 0;
    pld.push_constants      = nullptr;

    auto bmr = device->CreatePipelineLayout(pld, &pipeline_layout);
    BMR_ASSERT(bmr);
    return bmr == b::BMRESULT_SUCCEED;
}
bool HelloTriangle::CreateRenderPass()
{
    b::RENDER_PASS_DESC render_pass_desc{};

    // このサンプルで使用するアタッチメントはスワップチェインのバックバッファのみです。
    b::ATTACHMENT_DESC attachment{};
    attachment.flags        = b::ATTACHMENT_FLAG_NONE;
    attachment.format       = back_buffer_rtvs[0]->GetDesc().view.format;
    attachment.sample_count = 1;
    attachment.load_op      = b::ATTACHMENT_LOAD_OP_CLEAR;
    attachment.store_op     = b::ATTACHMENT_STORE_OP_STORE;
    attachment.begin_state  = b::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE;
    attachment.end_state    = b::RESOURCE_STATE_PRESENT;

    // 所定のサブパス内でアタッチメントがどのような状態となるかについて記述します。
    b::ATTACHMENT_REFERENCE color_attachment_ref{};
    color_attachment_ref.attachment_index             = 0;
    color_attachment_ref.state_at_pass                = b::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE;
    color_attachment_ref.stencil_state_at_pass        = {};
    color_attachment_ref.input_attachment_aspect_mask = b::TEXTURE_ASPECT_FLAG_COLOR;

    // サブパスで使用するアタッチメントと追加の情報を記述します。
    b::SUBPASS_DESC subpass_desc{};
    subpass_desc.flags                          = b::SUBPASS_FLAG_NONE;
    subpass_desc.pipeline_bind_point            = b::PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.view_mask                      = 0x0;
    subpass_desc.num_color_attachments          = 1;
    subpass_desc.color_attachments              = &color_attachment_ref;
    subpass_desc.resolve_attachments            = nullptr;
    subpass_desc.depth_stencil_attachment       = nullptr;

    // 実行とメモリアクセスの依存関係を記述します。
    b::SUBPASS_DEPENDENCY dependencies[] = { {},{} };
    dependencies[0].src_subpass                 = b::B3D_SUBPASS_EXTERNAL;  // レンダーパス開始前->サブパス0 間の依存関係
    dependencies[0].dst_subpass                 = 0;
    dependencies[0].src_stage_mask              = b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE;
    dependencies[0].dst_stage_mask              = b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT;
    dependencies[0].src_access                  = b::RESOURCE_ACCESS_FLAG_NONE;
    dependencies[0].dst_access                  = b::RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE;
    dependencies[0].dependency_flags            = b::DEPENDENCY_FLAG_NONE;
    dependencies[0].view_offset                 = 0;

    dependencies[1].src_subpass                 = 0;                        // サブパス0->レンダーパス終了後 間の依存関係
    dependencies[1].dst_subpass                 = b::B3D_SUBPASS_EXTERNAL;
    dependencies[1].src_stage_mask              = b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT;
    dependencies[1].dst_stage_mask              = b::PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE;
    dependencies[1].src_access                  = b::RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE;
    dependencies[1].dst_access                  = b::RESOURCE_ACCESS_FLAG_NONE;
    dependencies[1].dependency_flags            = b::DEPENDENCY_FLAG_NONE;
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

    auto bmr = device->CreateRenderPass(render_pass_desc, &render_pass);
    BMR_ASSERT(bmr);
    return bmr == b::BMRESULT_SUCCEED;
}
bool HelloTriangle::CreateShaderModules()
{
    shader_modules.emplace_back().Attach(CreateShaderModule("HelloTriangle/shader/VertexShader.hlsl", buma3d::SHADER_STAGE_FLAG_VERTEX, "main"));
    shader_modules.emplace_back().Attach(CreateShaderModule("HelloTriangle/shader/PixelShader.hlsl", buma3d::SHADER_STAGE_FLAG_PIXEL, "main"));

    return true;
}
buma3d::IShaderModule* HelloTriangle::CreateShaderModule(const char* _path, buma3d::SHADER_STAGE_FLAG _stage, const char* _entry_point)
{
    // シェーダをコンパイル
    shader::LOAD_SHADER_DESC desc{};
    desc.filename    = _path;
    desc.entry_point = _entry_point;
    desc.defines     = {};
    desc.stage       = shader::ConvertShaderStage(_stage);

    desc.options.pack_matrices_in_row_major = false;
    desc.options.enable_16bit_types         = false;
    desc.options.enable_debug_info          = true;
    desc.options.disable_optimizations      = true;
    desc.options.optimization_level         = 0; // 0 to 3, no optimization to most optimization
    desc.options.shader_model               = { 6, 2 };

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
bool HelloTriangle::CreateGraphicsPipelines()
{
    b::BMRESULT bmr{};
    // グラフィックスパイプラインの作成
    {
        b::GRAPHICS_PIPELINE_STATE_DESC pso_desc{};

        pso_desc.pipeline_layout      = pipeline_layout.Get();
        pso_desc.render_pass          = render_pass.Get();
        pso_desc.subpass              = 0;
        pso_desc.node_mask            = b::B3D_DEFAULT_NODE_MASK;
        pso_desc.flags                = b::PIPELINE_STATE_FLAG_NONE;

        b::PIPELINE_SHADER_STAGE_DESC shader_stages[2]{};
        {
            shader_stages[0].stage            = b::SHADER_STAGE_FLAG_VERTEX;
            shader_stages[0].entry_point_name = "main";
            shader_stages[0].flags            = b::PIPELINE_SHADER_STAGE_FLAG_NONE;
            shader_stages[0].module           = shader_modules[0].Get();

            shader_stages[1].stage            = b::SHADER_STAGE_FLAG_PIXEL;
            shader_stages[1].entry_point_name = "main";
            shader_stages[1].flags            = b::PIPELINE_SHADER_STAGE_FLAG_NONE;
            shader_stages[1].module           = shader_modules[1].Get();

            pso_desc.num_shader_stages    = 2;
            pso_desc.shader_stages        = shader_stages;
        }

        // 入力レイアウト
        b::INPUT_LAYOUT_DESC  input_layout{};
        b::INPUT_SLOT_DESC    input_slot{};
        b::INPUT_ELEMENT_DESC input_elements[2]{};
        {
            //                  { semantic_name, semantic_index, format                               , aligned_byte_offset           }
            input_elements[0] = { "POSITION"   , 0             , b::RESOURCE_FORMAT_R32G32B32A32_FLOAT, b::B3D_APPEND_ALIGNED_ELEMENT };
            input_elements[1] = { "COLOR"      , 0             , b::RESOURCE_FORMAT_R32G32B32A32_FLOAT, b::B3D_APPEND_ALIGNED_ELEMENT };

            input_slot.slot_number              = 0;
            input_slot.stride_in_bytes          = sizeof(float) * 8;// == RESOURCE_FORMAT_R32G32B32A32_FLOAT * 2
            input_slot.classification           = b::INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            input_slot.instance_data_step_rate  = 0;
            input_slot.num_elements             = _countof(input_elements);
            input_slot.elements                 = input_elements;

            input_layout.num_input_slots  = 1;
            input_layout.input_slots      = &input_slot;

            pso_desc.input_layout = &input_layout;
        }

        b::INPUT_ASSEMBLY_STATE_DESC ia{};
        {
            ia.topology = b::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            pso_desc.input_assembly_state = &ia;
        }

        pso_desc.tessellation_state   = nullptr;

        b::RASTERIZATION_STATE_DESC rs{};
        {
            rs.fill_mode                        = b::FILL_MODE_SOLID;
            rs.cull_mode                        = b::CULL_MODE_BACK;
            rs.is_front_counter_clockwise       = false;
            rs.is_enabled_depth_clip            = false;
            rs.is_enabled_depth_bias            = false;
            rs.depth_bias_scale                 = 0;
            rs.depth_bias_clamp                 = 0.f;
            rs.depth_bias_slope_scale           = 0.f;
            rs.is_enabled_conservative_raster   = false;
            rs.line_rasterization_mode          = b::LINE_RASTERIZATION_MODE_DEFAULT;
            rs.line_width                       = 1.f;

            pso_desc.rasterization_state  = &rs;
        }

        pso_desc.stream_output = nullptr;

        b::MULTISAMPLE_STATE_DESC ms{};
        {
            ms.is_enabled_alpha_to_coverage     = false;
            ms.is_enabled_sample_rate_shading   = false;
            ms.rasterization_samples            = 1;
            ms.sample_masks                     = b::B3D_DEFAULT_SAMPLE_MASK;
            ms.sample_position_state.is_enabled = false;
            ms.sample_position_state.desc       = nullptr;

            pso_desc.multisample_state = &ms;
        }

        b::DEPTH_STENCIL_STATE_DESC ds{};
        {
            ds.is_enabled_depth_test        = false;
            ds.is_enabled_depth_write       = false;
            ds.depth_comparison_func        = b::COMPARISON_FUNC_NEVER;
            ds.is_enabled_depth_bounds_test = false;
            ds.min_depth_bounds             = 0;
            ds.max_depth_bounds             = 1;
            ds.is_enabled_stencil_test      = false;

            ds.stencil_front_face.fail_op         = b::STENCIL_OP_KEEP;
            ds.stencil_front_face.depth_fail_op   = b::STENCIL_OP_KEEP;
            ds.stencil_front_face.pass_op         = b::STENCIL_OP_KEEP;
            ds.stencil_front_face.comparison_func = b::COMPARISON_FUNC_NEVER;
            ds.stencil_front_face.compare_mask    = b::B3D_DEFAULT_STENCIL_COMPARE_MASK;
            ds.stencil_front_face.write_mask      = b::B3D_DEFAULT_STENCIL_WRITE_MASK;
            ds.stencil_front_face.reference       = b::B3D_DEFAULT_STENCIL_REFERENCE;

            ds.stencil_back_face.fail_op         = b::STENCIL_OP_KEEP;
            ds.stencil_back_face.depth_fail_op   = b::STENCIL_OP_KEEP;
            ds.stencil_back_face.pass_op         = b::STENCIL_OP_KEEP;
            ds.stencil_back_face.comparison_func = b::COMPARISON_FUNC_NEVER;
            ds.stencil_back_face.compare_mask    = b::B3D_DEFAULT_STENCIL_COMPARE_MASK;
            ds.stencil_back_face.write_mask      = b::B3D_DEFAULT_STENCIL_WRITE_MASK;
            ds.stencil_back_face.reference       = b::B3D_DEFAULT_STENCIL_REFERENCE;

            pso_desc.depth_stencil_state = &ds;
        }

        b::BLEND_STATE_DESC bs{};
        b::RENDER_TARGET_BLEND_DESC attachments{};
        {
            attachments.is_enabled_blend    = false;
            attachments.src_blend           = b::BLEND_FACTOR_ONE;
            attachments.dst_blend           = b::BLEND_FACTOR_ONE;
            attachments.blend_op            = b::BLEND_OP_ADD;
            attachments.src_blend_alpha     = b::BLEND_FACTOR_ONE;
            attachments.dst_blend_alpha     = b::BLEND_FACTOR_ONE;
            attachments.blend_op_alpha      = b::BLEND_OP_ADD;
            attachments.color_write_mask    = b::COLOR_WRITE_FLAG_ALL;

            bs.is_enabled_independent_blend = false;
            bs.is_enabled_logic_op          = false;
            bs.logic_op                     = b::LOGIC_OP_SET;
            bs.num_attachments              = 1;
            bs.attachments                  = &attachments;
            bs.blend_constants              = { 1.f, 1.f, 1.f, 1.f };

            pso_desc.blend_state = &bs;
        }

        b::VIEWPORT_STATE_DESC vp{};
        {
            vp.num_viewports        = 1;
            vp.num_scissor_rects    = 1;
            vp.viewports            = nullptr;
            vp.scissor_rects        = nullptr;

            pso_desc.viewport_state = &vp;
        }

        b::DYNAMIC_STATE_DESC   dynamic_state_desc{};
        b::DYNAMIC_STATE        dynamic_states[] = { b::DYNAMIC_STATE_VIEWPORT, b::DYNAMIC_STATE_SCISSOR };
        {
            pso_desc.dynamic_state = &dynamic_state_desc;
            dynamic_state_desc.num_dynamic_states = _countof(dynamic_states);

            dynamic_state_desc.dynamic_states = dynamic_states;
        }

        bmr = device->CreateGraphicsPipelineState(pso_desc, &pipeline);
        BMR_RET_IF_FAILED(bmr);
    }

    return true;
}
bool HelloTriangle::CreateCommandAllocator()
{
    cmd_allocator.resize(BUFFER_COUNT);
    for (auto& i : cmd_allocator)
    {
        b::COMMAND_ALLOCATOR_DESC cad{};
        cad.type    = b::COMMAND_TYPE_DIRECT;
        cad.level   = b::COMMAND_LIST_LEVEL_PRIMARY;
        cad.flags   = b::COMMAND_ALLOCATOR_FLAG_NONE;

        auto bmr = device->CreateCommandAllocator(cad, &i);
        BMR_RET_IF_FAILED(bmr);
    }

    return true;
}
bool HelloTriangle::CreateCommandLists()
{
    cmd_lists.resize(BUFFER_COUNT);
    b::COMMAND_LIST_DESC cld{};
    cld.type      = b::COMMAND_TYPE_DIRECT;
    cld.level     = b::COMMAND_LIST_LEVEL_PRIMARY;
    cld.node_mask = b::B3D_DEFAULT_NODE_MASK;
    uint32_t cnt = 0;
    for (auto& i : cmd_lists)
    {
        cld.allocator = cmd_allocator[cnt].Get();
        auto bmr = device->AllocateCommandList(cld, &i);
        BMR_RET_IF_FAILED(bmr);
        i->SetName(std::string("CommandList " + std::to_string(cnt++)).c_str());
    }

    return true;
}
bool HelloTriangle::CreateFences()
{
    b::BMRESULT bmr;

    // 各コマンドリストの実行完了をCPUで待機する GPU_TO_CPU フェンス
    cmd_fences.resize(BUFFER_COUNT);
    for (uint32_t i = 0; i < BUFFER_COUNT; i++)
    {
        bmr = device->CreateFence(init::BinaryCpuFenceDesc(), &cmd_fences[i]);
        BMR_ASSERT(bmr);
    }

    // バックバッファの利用可能をコマンドキューへ通知する GPU_TO_GPU フェンス
    bmr = device->CreateFence(init::BinaryFenceDesc(), &acquire_fence);
    BMR_ASSERT(bmr);

    // 描画完了をスワップチェインへ通知する GPU_TO_GPU フェンス
    bmr = device->CreateFence(init::BinaryFenceDesc(), &render_complete_fence);
    BMR_ASSERT(bmr);

    for (uint32_t i = 0; i < BUFFER_COUNT; i++)
        cmd_fences[i]->SetName(("cmd_fences" + std::to_string(i)).c_str());
    acquire_fence->SetName("acquire_fence");
    render_complete_fence->SetName("render_complete_fence");

    return true;
}
bool HelloTriangle::CreateBuffers()
{
    b::BMRESULT bmr{};
    // 頂点バッファリソースの器を作成
    {
        auto vbdesc = init::BufferResourceDesc(sizeof(VERTEX) * triangle.size(), init::BUF_COPYABLE_FLAGS | b::BUFFER_USAGE_FLAG_VERTEX_BUFFER);
        bmr = device->CreatePlacedResource(vbdesc, &vertex_buffer);
        BMR_RET_IF_FAILED(bmr);
        vertex_buffer->SetName("Vertex buffer");
    }

    // インデックスバッファリソースの器を作成
    {
        auto ibdesc = init::BufferResourceDesc(sizeof(decltype(index)::value_type) * index.size(), init::BUF_COPYABLE_FLAGS | b::BUFFER_USAGE_FLAG_INDEX_BUFFER);
        bmr = device->CreatePlacedResource(ibdesc, &index_buffer);
        BMR_RET_IF_FAILED(bmr);
        index_buffer->SetName("Index buffer");
    }

    return true;
}
bool HelloTriangle::CreateHeaps(b::RESOURCE_HEAP_ALLOCATION_INFO* _heap_alloc_info, std::vector<b::RESOURCE_ALLOCATION_INFO>* _alloc_infos)
{
    // バッファのサイズ要件を取得。
    {
        _alloc_infos->resize(2);
        b::IResource* resources[] = { vertex_buffer.Get(), index_buffer.Get() };
        auto bmr = device->GetResourceAllocationInfo(2, resources, _alloc_infos->data(), _heap_alloc_info);
        BMR_RET_IF_FAILED(bmr);
    }

    // ヒーププロパティを取得
    {
        heap_props.resize(device->GetResourceHeapProperties(nullptr));
        device->GetResourceHeapProperties(heap_props.data());
    }

    // 頂点、インデックスバッファ用リソースヒープを作成
    {
        auto heap_prop = FindDeviceLocalHeap(heap_props);
        RET_IF_FAILED(heap_prop);
        RET_IF_FAILED(_heap_alloc_info->heap_type_bits & (1 << heap_prop->heap_index));

        b::RESOURCE_HEAP_DESC heap_desc{};
        heap_desc.heap_index         = heap_prop->heap_index;
        heap_desc.size_in_bytes      = _heap_alloc_info->total_size_in_bytes;
        heap_desc.alignment          = _heap_alloc_info->required_alignment;
        heap_desc.flags              = b::RESOURCE_HEAP_FLAG_NONE;
        heap_desc.creation_node_mask = b::B3D_DEFAULT_NODE_MASK;
        heap_desc.visible_node_mask  = b::B3D_DEFAULT_NODE_MASK;

        auto bmr = device->CreateResourceHeap(heap_desc, &resource_heap);
        BMR_RET_IF_FAILED(bmr);
        resource_heap->SetName("Device local heap");
    }

    return true;
}
bool HelloTriangle::BindResourceHeaps(b::RESOURCE_HEAP_ALLOCATION_INFO* _heap_alloc_info, std::vector<b::RESOURCE_ALLOCATION_INFO>* _alloc_infos)
{
    b::BMRESULT bmr{};
    // 頂点、インデックスバッファをバインド
    b::BIND_RESOURCE_HEAP_INFO info{};
    info.src_heap            = resource_heap.Get();
    info.num_bind_node_masks = 0;
    info.bind_node_masks     = nullptr;

    // 頂点バッファ
    info.src_heap_offset = (*_alloc_infos)[0].heap_offset;
    info.dst_resource    = vertex_buffer.Get();
    bmr = device->BindResourceHeaps(1, &info);
    BMR_RET_IF_FAILED(bmr);

    // インデックスバッファ
    info.src_heap_offset = (*_alloc_infos)[1].heap_offset;
    info.dst_resource    = index_buffer.Get();
    bmr = device->BindResourceHeaps(1, &info);
    BMR_RET_IF_FAILED(bmr);

    return true;
}
bool HelloTriangle::CreateBuffersForCopy()
{
    // コピー用頂点、インデックスバッファを作成
    auto heap_prop = FindMappableHeap(heap_props);
    RET_IF_FAILED(heap_prop);

    b::COMMITTED_RESOURCE_DESC comitted_desc = init::CommittedResourceDesc(heap_prop->heap_index, b::RESOURCE_HEAP_FLAG_NONE, {});
    comitted_desc.resource_desc.dimension       = b::RESOURCE_DIMENSION_BUFFER;
    comitted_desc.resource_desc.flags           = b::RESOURCE_FLAG_NONE;
    comitted_desc.resource_desc.buffer.flags    = b::BUFFER_CREATE_FLAG_NONE;

    // コピー用頂点バッファリソースを作成
    {
        auto&& vertex_buffer_desc = comitted_desc.resource_desc;
        vertex_buffer_desc.buffer.usage         = b::BUFFER_USAGE_FLAG_COPY_SRC | b::BUFFER_USAGE_FLAG_COPY_DST;
        vertex_buffer_desc.buffer.size_in_bytes = sizeof(VERTEX) * triangle.size();

        auto bmr = device->CreateCommittedResource(comitted_desc, &vertex_buffer_src);
        BMR_RET_IF_FAILED(bmr);
        vertex_buffer_src->SetName("Vertex buffer for copy");
        vertex_buffer_src->GetHeap()->SetName("Vertex buffer heap for copy");

        // データを書き込む
        {
            util::Mapper map(vertex_buffer_src->GetHeap());
            memcpy_s(map.As<VERTEX>().GetData(), vertex_buffer_desc.buffer.size_in_bytes, triangle.data(), vertex_buffer_desc.buffer.size_in_bytes);
        }
    }

    // コピー用インデックスバッファリソースを作成
    {
        auto&& index_buffer_desc = comitted_desc.resource_desc;
        index_buffer_desc.buffer.usage         = b::BUFFER_USAGE_FLAG_COPY_SRC | b::BUFFER_USAGE_FLAG_COPY_DST;
        index_buffer_desc.buffer.size_in_bytes = sizeof(uint16_t) * index.size();

        auto bmr = device->CreateCommittedResource(comitted_desc, &index_buffer_src);
        BMR_RET_IF_FAILED(bmr);
        index_buffer_src->SetName("Index buffer for copy");
        index_buffer_src->GetHeap()->SetName("Index buffer heap for copy");

        // データを書き込む
        {
            util::Mapper map(index_buffer_src->GetHeap());
            memcpy_s(map.As<uint16_t>().GetData(), index_buffer_desc.buffer.size_in_bytes, index.data(), sizeof(uint16_t) * index.size());
        }
    }

    return true;
}
bool HelloTriangle::CopyBuffers()
{
    // 頂点、インデックスバッファデータをデバイスローカルバッファへコピー
    b::BMRESULT bmr{};
    auto&& l = cmd_lists[0];

    b::COMMAND_LIST_BEGIN_DESC begin{};
    begin.flags = b::COMMAND_LIST_BEGIN_FLAG_NONE;
    begin.inheritance_desc = nullptr;

    // 記録を開始
    bmr = l->BeginRecord(begin);
    BMR_RET_IF_FAILED(bmr);

    // コピー宛先、ソースそれぞれにバリアを張る
    // NOTE: リソースはコマンドリストで初回使用される際に、適切な状態への遷移が暗黙的に行われます。
    //       ここで記録しているコマンドはICommandQueue::Submit()で送信する際の初回の要素(SUBMIT_INFO::command_lists_to_execute[0])のため、実際はこのようなバリアをスキップすることは有効です。
    b::CMD_PIPELINE_BARRIER barreir{};
    b::BUFFER_BARRIER_DESC buffer_barreirs[4]{};
    {
        buffer_barreirs[0].src_queue_type = b::COMMAND_TYPE_DIRECT;
        buffer_barreirs[0].dst_queue_type = b::COMMAND_TYPE_DIRECT;
        buffer_barreirs[0].barrier_flags  = b::RESOURCE_BARRIER_FLAG_NONE;

        buffer_barreirs[0].buffer         = vertex_buffer.Get();
        buffer_barreirs[0].src_state      = b::RESOURCE_STATE_UNDEFINED;
        buffer_barreirs[0].dst_state      = b::RESOURCE_STATE_COPY_DST_WRITE;

        buffer_barreirs[1] = buffer_barreirs[0];
        buffer_barreirs[1].buffer         = vertex_buffer_src.Get();
        buffer_barreirs[1].src_state      = b::RESOURCE_STATE_HOST_READ_WRITE;
        buffer_barreirs[1].dst_state      = b::RESOURCE_STATE_COPY_SRC_READ;

        buffer_barreirs[2] = buffer_barreirs[0];
        buffer_barreirs[3] = buffer_barreirs[1];
        buffer_barreirs[2].buffer         = index_buffer.Get();
        buffer_barreirs[3].buffer         = index_buffer_src.Get();

        barreir.src_stages           = b::PIPELINE_STAGE_FLAG_HOST;
        barreir.dst_stages           = b::PIPELINE_STAGE_FLAG_COPY_RESOLVE;
        barreir.dependency_flags     = b::DEPENDENCY_FLAG_NONE;
        barreir.num_buffer_barriers  = _countof(buffer_barreirs);
        barreir.buffer_barriers      = buffer_barreirs;
        barreir.num_texture_barriers = 0;
        barreir.texture_barriers     = nullptr;

        l->PipelineBarrier(barreir);
    }

    // バッファをコピー
    b::CMD_COPY_BUFFER_REGION copy_buffer{};
    b::BUFFER_COPY_REGION copy_region{};
    {
        copy_buffer.num_regions = 1;
        copy_buffer.regions = &copy_region;

        copy_region.src_offset      = 0;
        copy_region.dst_offset      = 0;
        copy_region.size_in_bytes   = vertex_buffer->GetDesc().buffer.size_in_bytes;
        copy_buffer.src_buffer      = vertex_buffer_src.Get();
        copy_buffer.dst_buffer      = vertex_buffer.Get();
        l->CopyBufferRegion(copy_buffer);

        copy_region.src_offset      = 0;
        copy_region.dst_offset      = 0;
        copy_region.size_in_bytes   = index_buffer->GetDesc().buffer.size_in_bytes;
        copy_buffer.src_buffer      = index_buffer_src.Get();
        copy_buffer.dst_buffer      = index_buffer.Get();
        l->CopyBufferRegion(copy_buffer);
    }


    // NOTE: D3D12ではコマンド実行完了後に、リソースが暗黙的にCOMMON状態へ遷移します。 また、Vulkanにはバッファには明確な「状態」がありません。 そのためD3D12の仕様に従う限りはバリアをスキップできます。
    //       次回コマンドリストで初回使用される際に、インデックスと頂点バッファ状態への遷移が暗黙的に行われます: (COPY_DST -> COMMON) -> VERTEX/INDEX_READ への遷移


    // 記録を終了
    bmr = l->EndRecord();
    BMR_RET_IF_FAILED(bmr);

    // キューへ送信
    {
        b::SUBMIT_INFO submit_info{};
        submit_info.wait_fence.num_fences        = 0;
        submit_info.wait_fence.fences            = nullptr;
        submit_info.wait_fence.fence_values      = nullptr;
        submit_info.num_command_lists_to_execute = 1;
        submit_info.command_lists_to_execute     = l.GetAddressOf();
        submit_info.signal_fence.num_fences      = 0;
        submit_info.signal_fence.fences          = nullptr;
        submit_info.signal_fence.fence_values    = nullptr;

        b::SUBMIT_DESC submit{};
        submit.num_submit_infos    = 1;
        submit.submit_infos        = &submit_info;
        submit.signal_fence_to_cpu = cmd_fences[0].Get();
        bmr = command_queue->Submit(submit);
        BMR_RET_IF_FAILED(bmr);
    }

    // 待機
    bmr = cmd_fences[0]->Wait(0, UINT32_MAX);
    BMR_RET_IF_FAILED(bmr);

    // GPU_TO_CPUフェンスをリセット
    bmr = cmd_fences[0]->Reset();
    BMR_RET_IF_FAILED(bmr);

    return true;
}
bool HelloTriangle::CreateBufferViews()
{
    b::BMRESULT bmr{};
    // 頂点バッファビューを作成
    {
        b::VERTEX_BUFFER_VIEW_DESC vbvdesc{};
        uint64_t buffer_offset      = 0;
        uint32_t sizes_in_bytes     = (uint32_t)vertex_buffer->GetDesc().buffer.size_in_bytes;
        uint32_t strides_in_bytes   = sizeof(VERTEX);
        vbvdesc.num_input_slots     = 1;
        vbvdesc.buffer_offsets      = &buffer_offset;
        vbvdesc.sizes_in_bytes      = &sizes_in_bytes;
        vbvdesc.strides_in_bytes    = &strides_in_bytes;

        bmr = device->CreateVertexBufferView(vertex_buffer.Get(), vbvdesc, &vertex_buffer_view);
        BMR_RET_IF_FAILED(bmr);
    }

    // インデックスバッファビューを作成
    {
        b::INDEX_BUFFER_VIEW_DESC ibvdesc{};
        ibvdesc.buffer_offset   = 0;
        ibvdesc.size_in_bytes   = index_buffer->GetDesc().buffer.size_in_bytes;
        ibvdesc.index_type      = b::INDEX_TYPE_UINT16;

        bmr = device->CreateIndexBufferView(index_buffer.Get(), ibvdesc, &index_buffer_view);
        BMR_RET_IF_FAILED(bmr);
    }

    return true;
}

#pragma endregion preparing Buma3D objects

void HelloTriangle::PrepareFrame(uint32_t _buffer_index, float _deltatime)
{
    auto reset_flags = b::COMMAND_ALLOCATOR_RESET_FLAG_RELEASE_RESOURCES;
    cmd_allocator[_buffer_index]->Reset(reset_flags);

    auto&& l = cmd_lists[_buffer_index];
    b::COMMAND_LIST_BEGIN_DESC begin{};
    begin.flags            = b::COMMAND_LIST_BEGIN_FLAG_NONE;
    begin.inheritance_desc = nullptr;

    auto bmr = l->BeginRecord(begin);
    BMR_ASSERT(bmr);
    {
        b::CMD_PIPELINE_BARRIER barrier{};
        b::TEXTURE_BARRIER_DESC tb{};
        tb.type           = b::TEXTURE_BARRIER_TYPE_VIEW;
        tb.view           = back_buffer_rtvs[_buffer_index].Get();
        tb.src_state      = b::RESOURCE_STATE_UNDEFINED;
        tb.dst_state      = b::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE;
        tb.src_queue_type = b::COMMAND_TYPE_DIRECT;
        tb.dst_queue_type = b::COMMAND_TYPE_DIRECT;
        tb.barrier_flags  = b::RESOURCE_BARRIER_FLAG_NONE;
        barrier.num_buffer_barriers  = 0;
        barrier.buffer_barriers      = nullptr;
        barrier.num_texture_barriers = 1;
        barrier.texture_barriers     = &tb;
        barrier.src_stages           = b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE;              // パイプラインの可能な限り早い段階で遷移を行います。
        barrier.dst_stages           = b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT;  // 書き込みアクセス発生前までにはカラーアタッチメントへの遷移が行われます。
        l->PipelineBarrier(barrier);

        l->SetPipelineState(pipeline.Get());
        l->SetPipelineLayout(b::PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.Get());

        static float sc = 0.f;
        static float sx = 0.f;
        sc = sc + 0.34f * _deltatime;
        sx = fabsf(sinf(sc));
        b::CLEAR_VALUE            clear_val{ b::CLEAR_RENDER_TARGET_VALUE{0.9f * sx ,0.28f,0.13f,1.f} };
        b::RENDER_PASS_BEGIN_DESC rpbd{ render_pass.Get(), framebuffers[_buffer_index].Get(), 1, &clear_val };
        b::SUBPASS_BEGIN_DESC     spbd{ b::SUBPASS_CONTENTS_INLINE };
        l->BeginRenderPass(rpbd, spbd);
        {
            auto&& resolution = swapchain->GetDesc().buffer;
            b::VIEWPORT     vpiewport    = {   0, 0  ,   (float)resolution.width, (float)resolution.height, b::B3D_VIEWPORT_MIN_DEPTH, b::B3D_VIEWPORT_MAX_DEPTH };
            b::SCISSOR_RECT scissor_rect = { { 0, 0 }, {        resolution.width,        resolution.height} };
            l->SetViewports(1, &vpiewport);
            l->SetScissorRects(1, &scissor_rect);

            l->BindVertexBufferViews({ 0, 1, vertex_buffer_view.GetAddressOf() });
            l->BindIndexBufferView(index_buffer_view.Get());

            //             { index_count_per_instance, instance_count, start_index_location, base_vertex_location, start_instance_location }
            l->DrawIndexed({ 3                       , 1             , 0                   , 0                   , 0                       });
        }
        l->EndRenderPass({});
    }
    bmr = l->EndRecord();
    BMR_ASSERT(bmr);
}

void HelloTriangle::Update(float _deltatime)
{
    if (window->GetWindowState() == WINDOW_STATE_MINIMIZE)
        return std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // 次のバックバッファを取得
    MoveToNextFrame();
}

void HelloTriangle::MoveToNextFrame()
{
    // プレゼント可能となるバッファのインデックスを取得します。
    // 加えてフェンスにシグナル操作を行います。取得されたインデックスのバッファが利用可能となるタイミングを通知します。
    b::SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO ai{};
    ai.signal_fence     = acquire_fence.Get();
    ai.timeout_millisec = 0;
    auto bmr = swapchain->AcquireNextBuffer(ai, &back_buffer_index);
    BUMA_ASSERT(bmr == b::BMRESULT_SUCCEED || bmr == b::BMRESULT_SUCCEED_NOT_READY);
}

void HelloTriangle::OnResize(uint32_t _w, uint32_t _h)
{
    command_queue->WaitIdle();

    // 予めリサイズ前のバックバッファを参照するオブジェクト(フレームバッファ、RTV)を解放する必要があります。
    framebuffers.clear();
    back_buffer_rtvs.clear();
    back_buffers.clear();

    buma3d::SWAP_CHAIN_DESC d = swapchain->GetDesc();
    d.buffer.width  = _w;
    d.buffer.height = _h;
    auto bmr = swapchain->Recreate(d);
    BMR_ASSERT(bmr);

    CreateRenderTargetViews();
    CreateFramebuffers();
    for (uint32_t i = 0; i < BUFFER_COUNT; i++)
        PrepareFrame(i, 0.f);

    Update(0.f);
    Render(0.f);
}

void HelloTriangle::Render(float _deltatime)
{
    if (window->GetWindowState() == WINDOW_STATE_MINIMIZE)
        return;

    // 送信情報を準備
    uint64_t dummy{};
    b::SUBMIT_INFO submit_info{
        b::FENCE_SUBMISSION{ 1, acquire_fence.GetAddressOf(), &dummy },           // MoveToNextFrame()でacquireしたバッファが利用可能となるまでGPUで待機します。
        1, cmd_lists[back_buffer_index].GetAddressOf(),                           // コマンドリスト
        b::FENCE_SUBMISSION{ 1, render_complete_fence.GetAddressOf(), &dummy } }; // コマンドリストの実行完了を通知するフェンスと、スワップチェイン用のrender_complete_fenceをGPUでシグナルします。
    b::SUBMIT_DESC submit_desc{ 1, &submit_info, cmd_fences[back_buffer_index].Get() };

    b::BMRESULT bmr{};
    // コマンドリストとフェンスを送信
    {
        // acquireしたバックバッファを利用するコマンドリストの実行完了をCPUで待機します。
        bmr = cmd_fences[back_buffer_index]->Wait(0, UINT32_MAX);
        BMR_ASSERT(bmr);
        bmr = cmd_fences[back_buffer_index]->Reset();
        BMR_ASSERT(bmr);

        if (!is_reuse_commands)
            PrepareFrame(back_buffer_index, _deltatime);

        // 記録したコマンドを実行
        bmr = command_queue->Submit(submit_desc);
        BMR_ASSERT(bmr);
    }

    // バックバッファをプレゼント
    {
        // render_complete_fenceのシグナルをGPUで待機します。
        b::SWAP_CHAIN_PRESENT_INFO pi{};
        pi.wait_fence = render_complete_fence.Get();
        bmr = swapchain->Present(pi);
        BMR_ASSERT(bmr);
    }
}

HelloTriangle* HelloTriangle::Create(PlatformBase& _platform)
{
    return new HelloTriangle(_platform);
}

void HelloTriangle::Destroy(HelloTriangle* _app)
{
    delete _app;
}

BUMA_APPMODULE_API ApplicationBase* BUMA_APP_APIENTRY CreateApplication(PlatformBase& _platform)
{
    return HelloTriangle::Create(_platform);
}

BUMA_APPMODULE_API void BUMA_APP_APIENTRY DestroyApplication(ApplicationBase* _app)
{
    HelloTriangle::Destroy(static_cast<HelloTriangle*>(_app));
}


}// namespace buma
