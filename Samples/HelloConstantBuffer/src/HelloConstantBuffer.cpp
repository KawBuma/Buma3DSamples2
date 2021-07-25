#include <HelloConstantBuffer/HelloConstantBuffer.h>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <Buma3DHelpers/Buma3DHelpers.h>
#include <Buma3DHelpers/B3DDescHelpers.h>
#include <Buma3DHelpers/B3DInit.h>

#include <DeviceResources/DeviceResources.h>
#include <DeviceResources/CommandQueue.h>
#include <DeviceResources/SwapChain.h>
#include <DeviceResources/CopyContext.h>

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
bool                USE_HOST_WRITABLE_HEAP = true;

buma::util::Camera g_cam{};

}// namespace /*anonymous*/

namespace init = buma3d::init;
namespace b = buma3d;

template<typename T>
using Ptr = buma3d::util::Ptr<T>;

namespace buma
{

HelloConstantBuffer::HelloConstantBuffer(PlatformBase& _platform)
    : SampleBase(_platform)
    , triangle           {}
    , index              {}
    , cb_model           {}
    , cb_scene           {}
    , frame_cbs          {}
    , vertex_buffer      {}
    , index_buffer       {}
    , vertex_buffer_view {}
    , index_buffer_view  {}
    , is_reuse_commands  {}
{
    g_fpss = new std::remove_pointer_t<decltype(g_fpss)>;
    platform.AddHelpMessage(
R"(========== HelloConstantBuffer Options ==========
--use-host-writable
    ホスト可視ヒープに定数を書き込みます。

)");
}

HelloConstantBuffer::~HelloConstantBuffer()
{
    delete g_fpss;
    g_fpss = nullptr;
}

#pragma region events

void HelloConstantBuffer::OnKeyDown(KeyDownEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    // カメラを更新
    switch (_args.key->keysym.scancode)
    {
    case SDL_SCANCODE_W: g_cam.keys.up    = true; break;
    case SDL_SCANCODE_A: g_cam.keys.down  = true; break;
    case SDL_SCANCODE_S: g_cam.keys.left  = true; break;
    case SDL_SCANCODE_D: g_cam.keys.right = true; break;
    default: break;
    }
}
void HelloConstantBuffer::OnKeyUp(KeyUpEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    // カメラを更新
    switch (_args.key->keysym.scancode)
    {
    case SDL_SCANCODE_W: g_cam.keys.up    = false; break;
    case SDL_SCANCODE_A: g_cam.keys.down  = false; break;
    case SDL_SCANCODE_S: g_cam.keys.left  = false; break;
    case SDL_SCANCODE_D: g_cam.keys.right = false; break;
    default: break;
    }

    if (_args.key->keysym.scancode == SDL_SCANCODE_Z)
    {
        is_reuse_commands = !is_reuse_commands;
        BUMA_LOGI(is_reuse_commands ? "Reuse recorded commands" : "Re-record commands every frame");
    }
}
void HelloConstantBuffer::OnMouseMove(MouseMoveEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    auto&& mouse = *_args.mouse;
    g_cam.mouse.delta.x = (float)mouse.xrel;
    g_cam.mouse.delta.y = (float)mouse.yrel;
}
void HelloConstantBuffer::OnMouseButtonDown(MouseButtonDownEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    switch (_args.mouse->button)
    {
    case SDL_BUTTON_LEFT:   g_cam.mouse.left   = true; break;
    case SDL_BUTTON_MIDDLE: g_cam.mouse.middle = true; break;
    case SDL_BUTTON_RIGHT:  g_cam.mouse.right  = true; break;
    default: break;
    }
}
void HelloConstantBuffer::OnMouseButtonUp(MouseButtonUpEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    switch (_args.mouse->button)
    {
    case SDL_BUTTON_LEFT:   g_cam.mouse.left   = false; break;
    case SDL_BUTTON_MIDDLE: g_cam.mouse.middle = false; break;
    case SDL_BUTTON_RIGHT:  g_cam.mouse.right  = false; break;
    default: break;
    }
}
void HelloConstantBuffer::OnMouseWheel(MouseWheelEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    g_cam.mouse.wheel = static_cast<float>(_args.wheel->y);
}

void HelloConstantBuffer::OnProcessWindow(ProcessWindowEventArgs& _args)
{
    if (_args.process_flags & WINDOW_PROCESS_FLAG_RESIZED)
    {
        uint32_t w, h;
        _args.window->GetWindowedSize(&w, &h);
        OnResize(w, h);
    }
}

bool HelloConstantBuffer::OnInit()
{
    PrepareSettings();

    CreateWindow(1280, 720, "HelloConstantBuffer - Press the Z key to toggle whether to reuse command lists");

    CreateDeviceResources();

    buffer_count = 3;
    buma3d::SWAP_CHAIN_FLAGS flags = buma3d::SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC
                                   | buma3d::SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT;
    auto buffer_desc = init::SwapChainBufferDesc(1280, 720, buffer_count, { buma3d::RESOURCE_FORMAT_B8G8R8A8_UNORM/*default*/ }, buma3d::SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT);
    CreateSwapChain(buffer_desc, flags);
    CreateFencesForSwapChain();

    LoadAssets();
    g_cam.type = util::Camera::CameraType::lookat;
    g_cam.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    g_cam.setRotation(glm::vec3(0.0f));
    g_cam.setRotationSpeed(0.5f);
    g_cam.setPerspective(60.0f, window->GetAspectRatio(), 1.0f, 256.0f);

    USE_HOST_WRITABLE_HEAP = platform.HasArgument("--use-host-writable");
    BUMA_LOGI("USE_HOST_WRITABLE_HEAP {}", USE_HOST_WRITABLE_HEAP);

    CreateCommandAllocator();
    CreateCommandLists();

    CreateDescriptorSetLayouts();
    CreatePipelineLayout();

    CreateRenderPass();
    CreateFramebuffers();
    CreateShaderModules();
    CreatePipeline();

    CreateDescriptorHeap();
    CreateDescriptorPool();
    AllocateDescriptorSets();

    CreateBuffers();
    CreateBufferViews();
    CreateConstantBuffer();
    CreateConstantBufferView();

    CreateDescriptorUpdate();
    UpdateDescriptorSet();

    // 描画コマンドを記録
    for (uint32_t i = 0; i < buffer_count; i++)
        PrepareFrame(i, 0.f);

    return true;
}

void HelloConstantBuffer::Tick(const util::StepTimer& _timer)
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

void HelloConstantBuffer::OnDestroy()
{
    // result
    if (!g_first)
    {
        g_first = true;
        float res = 0.f;
        float size = static_cast<float>(g_fpss->size());
        for (auto& i : *g_fpss)
            res += i;
        BUMA_LOGI("average fps {}", res / size);
    }

    dr->WaitForGpu();

    // オブジェクトの解放
    for (auto& i : frame_cbs)
    {
        i.model_cbv.Reset();
        i.scene_cbv.Reset();
        dr->DestroyBuffer(i.buffer);
        i.buffer = nullptr;
    }
    frame_cbs.clear();

    vertex_buffer_view.Reset();
    index_buffer_view.Reset();
    dr->DestroyBuffer(vertex_buffer);
    dr->DestroyBuffer(index_buffer);
    vertex_buffer = nullptr;
    index_buffer = nullptr;

    DestroySampleBaseObjects();
}

#pragma endregion events

#pragma region preparing Buma3D objects

bool HelloConstantBuffer::LoadAssets()
{
    auto aspect_ratio = window->GetAspectRatio();
    triangle = {
          { {  0.0f , 1.0f, 0.0f, 1.f }, { 1.f, 0.f, 0.f, 1.f} }
        , { {  1.0f, -1.0f, 0.0f, 1.f }, { 0.f, 1.f, 0.f, 1.f} }
        , { { -1.0f, -1.0f, 0.0f, 1.f }, { 0.f, 0.f, 1.f, 1.f} }
    };
    index = { 0,1,2 };

    cb_model.model = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
    cb_model.model = glm::rotate   (cb_model.model, 0.f, glm::vec3(0, 1, 0));
    cb_model.model = glm::scale    (cb_model.model, glm::vec3(1, 1, 1));

    cb_scene.view_proj = g_cam.matrices.perspective * g_cam.matrices.view;

    return true;
}
bool HelloConstantBuffer::CreatePipeline()
{
    // グラフィックスパイプラインの作成
    util::PipelineStateDesc pso_desc{};
    pso_desc
        .SetPipelineLayout(pipeline_layout.Get())
        .SetRenderPass(render_pass.Get()).SetSubpass(0)
        .SetNodeMask().SetPipelineStateFlags()
        .AddDynamicState(buma3d::DYNAMIC_STATE_VIEWPORT)
        .AddDynamicState(buma3d::DYNAMIC_STATE_SCISSOR);
    pso_desc.ShaderStages()
        .AddVS(shader_modules["VS"].Get(), "main")
        .AddPS(shader_modules["PS"].Get(), "main");
    pso_desc.InputLayout()
        .AddNewInputSlot()
            .SetSlotNumber(0)
            .SetStrideInBytes(sizeof(float) * 8) // == RESOURCE_FORMAT_R32G32B32A32_FLOAT * 2
            .SetClassification(buma3d::INPUT_CLASSIFICATION_PER_VERTEX_DATA)
            .AddNewInputElement("POSITION", 0, buma3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT)
            .AddNewInputElement("COLOR"   , 0, buma3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT)
            .Finalize()
        .Finalize();
    pso_desc.InputAssemblyState().topology = buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pso_desc.RasterizationState()
        .FillMode(buma3d::FILL_MODE_SOLID).CullMode(buma3d::CULL_MODE_NONE)
        .DepthClipEnabled(false).DepthBias(false);
    pso_desc.MultisampleState().AlphaToCoverage(false).Finalize();
    pso_desc.ViewportState().SetCounts(1, 1).Finalize(true, true);
    pso_desc.DepthStencilState().DepthStencilDisable();
    pso_desc.BlendState().Reset().SetNumAttachmemns(1).Finalize();
    pso_desc.Finalize();

    auto bmr = device->CreateGraphicsPipelineState(pso_desc.GetAsGraphics(), &pipeline);
    BMR_RET_IF_FAILED(bmr);
    pipeline->SetName("HelloConstantBuffer::pipeline");

    return true;
}
bool HelloConstantBuffer::CreateDescriptorSetLayouts()
{
    // 頂点シェーダーに可視の register(b0, space*) を割り当てます。 register space はパイプラインレイアウトの作成時に決定されます。
    buma3d::DESCRIPTOR_SET_LAYOUT_BINDING bindings[1]{};
    bindings[0].descriptor_type      = b::DESCRIPTOR_TYPE_CBV;
    bindings[0].flags                = b::DESCRIPTOR_FLAG_NONE;
    bindings[0].base_shader_register = 0;
    bindings[0].num_descriptors      = 1;
    bindings[0].shader_visibility    = b::SHADER_VISIBILITY_VERTEX;
    bindings[0].static_sampler       = nullptr;

    buma3d::DESCRIPTOR_SET_LAYOUT_DESC desc{};
    desc.num_bindings   = _countof(bindings);
    desc.bindings       = bindings;
    desc.flags          = b::DESCRIPTOR_SET_LAYOUT_FLAG_NONE;
    auto bmr = device->CreateDescriptorSetLayout(desc, &set_layouts.emplace_back());
    BMR_RET_IF_FAILED(bmr);
    set_layouts.back()->SetName("HelloConstantBuffer::set_layouts 0");

    return true;
}
bool HelloConstantBuffer::CreatePipelineLayout()
{
    // space0,space1 にそれぞれset_layouts[0]のリソースが定義されます。
    b::IDescriptorSetLayout* layouts[2]{};
    layouts[0] = set_layouts[0].Get(); // モデル定数
    layouts[1] = set_layouts[0].Get(); // シーン定数

    b::PIPELINE_LAYOUT_DESC desc{};
    desc.flags = b::PIPELINE_LAYOUT_FLAG_NONE;
    desc.num_set_layouts    = _countof(layouts);
    desc.set_layouts        = layouts;
    desc.num_push_constants = 0;
    desc.push_constants     = nullptr;

    auto bmr = device->CreatePipelineLayout(desc, &pipeline_layout);
    BMR_RET_IF_FAILED(bmr);
    pipeline_layout->SetName("HelloConstantBuffer::pipeline_layout");

    return true;
}
bool HelloConstantBuffer::CreateDescriptorHeap()
{
    // コマンドリストに現在セットされる各ディスクリプタセットの親ヒープは同一でなければなりません。
    b::DESCRIPTOR_HEAP_SIZE heap_sizes[] = {
         { b::DESCRIPTOR_TYPE_CBV, 2 * buffer_count }
    };

    b::DESCRIPTOR_HEAP_DESC desc{};
    desc.flags          = b::DESCRIPTOR_HEAP_FLAG_NONE;
    desc.num_heap_sizes = _countof(heap_sizes);
    desc.heap_sizes     = heap_sizes;
    desc.node_mask      = b::B3D_DEFAULT_NODE_MASK;
    auto bmr = device->CreateDescriptorHeap(desc, &descriptor_heap);
    BMR_RET_IF_FAILED(bmr);
    descriptor_heap->SetName("HelloConstantBuffer::descriptor_heap");

    return true;
}
bool HelloConstantBuffer::CreateDescriptorPool()
{
    // モデル定数、シーン定数のディスクリプタを各バックバッファにそれぞれ割り当てます。
    b::DESCRIPTOR_POOL_SIZE pool_sizes[] = {
         { b::DESCRIPTOR_TYPE_CBV, 2 * buffer_count }
    };

    b::DESCRIPTOR_POOL_DESC pool_desc{};
    pool_desc.heap                      = descriptor_heap.Get();
    pool_desc.flags                     = b::DESCRIPTOR_POOL_FLAG_NONE;
    pool_desc.max_sets_allocation_count = 2 * buffer_count;
    pool_desc.num_pool_sizes            = _countof(pool_sizes);
    pool_desc.pool_sizes                = pool_sizes;

    auto bmr = device->CreateDescriptorPool(pool_desc, &descriptor_pool);
    BMR_RET_IF_FAILED(bmr);
    descriptor_pool->SetName("HelloConstantBuffer::descriptor_pool");

    return true;
}
bool HelloConstantBuffer::AllocateDescriptorSets()
{
    std::vector<b::IDescriptorSet*>         sets   (buffer_count * 2);
    std::vector<b::IDescriptorSetLayout*>   layouts(buffer_count * 2, set_layouts[0].Get());

    buma3d::DESCRIPTOR_SET_ALLOCATE_DESC allocate_desc{};
    allocate_desc.num_descriptor_sets = buffer_count * 2;
    allocate_desc.set_layouts         = layouts.data();
    auto bmr = descriptor_pool->AllocateDescriptorSets(allocate_desc, sets.data());
    BMR_RET_IF_FAILED(bmr);

    uint32_t cnt = 0;
    descriptor_sets.reserve(buffer_count * 2);
    for (auto& i : sets)
    {
        i->SetName(("HelloConstantBuffer::descriptor_sets " + std::to_string(cnt++)).c_str());
        descriptor_sets.emplace_back().Attach(i);
    }

    return true;
}
bool HelloConstantBuffer::CreateConstantBuffer()
{
    buma3d::RESOURCE_HEAP_PROPERTY_FLAGS heap_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL;

    // WRITABLE_HEAPはCPUで書き込みを行うことができるヒープです。
    // CPUでの書き込みとGPUでのコマンド実行が同時に発生すると、描画結果に悪影響を与える可能性があります。 
    if (USE_HOST_WRITABLE_HEAP)
        heap_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT;

    // 定数バッファビューが参照するバッファのオフセットは以下の値に整列する必要があります
    const auto CBV_ALIGNMENT = dr->GetDeviceAdapterLimits().min_constant_buffer_offset_alignment;
    auto cb_desc = init::BufferResourceDesc(0, init::BUF_CBV_FLAGS);
    cb_desc.buffer.size_in_bytes += util::AlignUp(sizeof(CB_SCENE), CBV_ALIGNMENT);
    cb_desc.buffer.size_in_bytes += util::AlignUp(sizeof(CB_MODEL), CBV_ALIGNMENT);

    // コマンド実行の独立性を高めるために、定数バッファはバックバッファ毎に作成します。
    // ただし、定数構造のサイズが非常に大きい場合ビデオメモリの消費が問題となる可能性があります。
    uint32_t cnt = 0;
    frame_cbs.resize(buffer_count);
    for (auto& i : frame_cbs)
    {
        i.buffer = dr->CreateBuffer(cb_desc, heap_flags);
        i.buffer->SetName(("HelloTriangle::frame_cbs " + std::to_string(cnt++)).c_str());
    }

    // 定数データを送信
    if (USE_HOST_WRITABLE_HEAP)
    {
        for (auto& i : frame_cbs)
        {
            auto data = i.buffer->GetMppedDataAs<uint8_t>();
            i.mapped_data[0/*model*/] = data;
            i.mapped_data[1/*scene*/] = data + util::AlignUp(sizeof(CB_MODEL), CBV_ALIGNMENT);
            memcpy(i.mapped_data[0], &cb_model, sizeof(CB_MODEL));
            memcpy(i.mapped_data[1], &cb_scene, sizeof(CB_SCENE));
        }
    }
    else
    {
        auto&& ctx = dr->GetCopyContext();
        for (auto& i : frame_cbs)
        {
            ctx.CopyDataToBuffer(i.buffer->GetB3DBuffer().Get(), 0
                                 , sizeof(cb_model), &cb_model);

            ctx.CopyDataToBuffer(i.buffer->GetB3DBuffer().Get(), util::AlignUp(sizeof(CB_MODEL), CBV_ALIGNMENT)
                                 , sizeof(cb_scene), &cb_scene);
        }
    }

    return true;
}
bool HelloConstantBuffer::CreateConstantBufferView()
{
    const auto CBV_ALIGNMENT = dr->GetDeviceAdapterLimits().min_constant_buffer_offset_alignment;
    b::CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
    for (auto& i : frame_cbs)
    {
        cbv_desc.buffer_offset = 0;
        cbv_desc.size_in_bytes = util::AlignUp(sizeof(CB_MODEL), CBV_ALIGNMENT);
        auto bmr = device->CreateConstantBufferView(i.buffer->GetB3DBuffer().Get(), cbv_desc, &i.model_cbv);
        BMR_RET_IF_FAILED(bmr);

        cbv_desc.buffer_offset += cbv_desc.size_in_bytes;
        cbv_desc.size_in_bytes = util::AlignUp(sizeof(CB_SCENE), CBV_ALIGNMENT);
        bmr = device->CreateConstantBufferView(i.buffer->GetB3DBuffer().Get(), cbv_desc, &i.scene_cbv);
        BMR_RET_IF_FAILED(bmr);
    }

    return true;
}
bool HelloConstantBuffer::CreateShaderModules()
{
    shader_modules["VS"].Attach(CreateShaderModule("HelloConstantBuffer/shader/VertexShader.hlsl", buma3d::SHADER_STAGE_FLAG_VERTEX, "main"));
    shader_modules["PS"].Attach(CreateShaderModule("HelloConstantBuffer/shader/PixelShader.hlsl", buma3d::SHADER_STAGE_FLAG_PIXEL, "main"));

    return true;
}
bool HelloConstantBuffer::CreateBuffers()
{
    auto vbdesc = init::BufferResourceDesc(sizeof(VERTEX) * triangle.size(), init::BUF_COPYABLE_FLAGS | b::BUFFER_USAGE_FLAG_VERTEX_BUFFER);
    vertex_buffer = dr->CreateBuffer(vbdesc, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL);
    vertex_buffer->SetName("HelloConstantBuffer::vertex_buffer");
    BUMA_ASSERT(vertex_buffer);

    vbdesc = init::BufferResourceDesc(sizeof(decltype(index)::value_type) * index.size(), init::BUF_COPYABLE_FLAGS | b::BUFFER_USAGE_FLAG_INDEX_BUFFER);
    index_buffer = dr->CreateBuffer(vbdesc, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL);
    index_buffer->SetName("HelloConstantBuffer::index_buffer");
    BUMA_ASSERT(index_buffer);

    // 頂点、インデックスバッファデータをデバイスローカルバッファへコピー
    auto&& ctx = dr->GetCopyContext();
    ctx.CopyDataToBuffer(vertex_buffer->GetB3DBuffer().Get(), 0, vertex_buffer->GetDesc().buffer.size_in_bytes, triangle.data());
    ctx.CopyDataToBuffer(index_buffer->GetB3DBuffer().Get(), 0, index_buffer->GetDesc().buffer.size_in_bytes, index.data());

    return true;
}
bool HelloConstantBuffer::CreateBufferViews()
{
    // 頂点バッファビューを作成
    util::VertexBufferViewDesc vbvdesc(1);
    vbvdesc.SetView(0, 0, (uint32_t)vertex_buffer->GetDesc().buffer.size_in_bytes, sizeof(VERTEX));
    auto bmr = device->CreateVertexBufferView(vertex_buffer->GetB3DBuffer().Get(), vbvdesc.Finalize().Get(), &vertex_buffer_view);
    BMR_RET_IF_FAILED(bmr);

    // インデックスバッファビューを作成
    b::INDEX_BUFFER_VIEW_DESC ibvdesc{ 0, index_buffer->GetDesc().buffer.size_in_bytes, buma3d::INDEX_TYPE_UINT16 };
    bmr = device->CreateIndexBufferView(index_buffer->GetB3DBuffer().Get(), ibvdesc, &index_buffer_view);
    BMR_RET_IF_FAILED(bmr);

    return true;
}
bool HelloConstantBuffer::UpdateDescriptorSet()
{
    std::vector<b::WRITE_DESCRIPTOR_SET>     write_sets    (buffer_count * 2);
    std::vector<b::WRITE_DESCRIPTOR_BINDING> write_bindings(buffer_count * 2);
    for (uint32_t i_frame = 0; i_frame < buffer_count; i_frame++)
    {
        auto offset = i_frame * 2;
        for (uint32_t i_set = 0; i_set < 2; i_set++)
        {
            auto&& b = write_bindings[offset + i_set];
            b.dst_binding_index       = 0;
            b.dst_first_array_element = 0;
            b.num_descriptors         = 1;
            b.src_views = i_set == 0
                ? (b::IView**)frame_cbs[i_frame].model_cbv.GetAddressOf()
                : (b::IView**)frame_cbs[i_frame].scene_cbv.GetAddressOf();

            auto&& w = write_sets[offset + i_set];
            w.dst_set               = descriptor_sets[offset + i_set].Get();
            w.num_bindings          = 1;
            w.bindings              = &b;
            w.num_dynamic_bindings  = 0;
            w.dynamic_bindings      = nullptr;
        }
    }

    b::UPDATE_DESCRIPTOR_SET_DESC update_desc{};
    update_desc.num_write_descriptor_sets   = (uint32_t)write_sets.size();
    update_desc.write_descriptor_sets       = write_sets.data();
    update_desc.num_copy_descriptor_sets    = 0;
    update_desc.copy_descriptor_sets        = nullptr;
    auto bmr = descriptor_update->UpdateDescriptorSets(update_desc);
    BMR_RET_IF_FAILED(bmr);

    return true;
}

#pragma endregion preparing Buma3D objects

void HelloConstantBuffer::MoveToNextFrame()
{
    // プレゼント可能となるバッファのインデックスを取得します。
    // 加えてフェンスにシグナル操作を行います。取得されたインデックスのバッファが利用可能となるタイミングを通知します。
    swapchain->SetAcquireInfo(acquire_fence.Get(), nullptr);
    auto bmr = swapchain->AcquireNextBuffer(0, &buffer_index);
    BUMA_ASSERT(bmr == b::BMRESULT_SUCCEED || bmr == b::BMRESULT_SUCCEED_NOT_READY);
}

void HelloConstantBuffer::Update(float _deltatime)
{
    if (window->GetWindowState() == WINDOW_STATE_MINIMIZE)
        return std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // 次のバックバッファを取得
    MoveToNextFrame();
    
    // シーン定数バッファを更新
    {
        g_cam.update(_deltatime);

        static float angle = 0.f;
        cb_model.model = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
        cb_model.model = glm::rotate(cb_model.model, angle += 0.3f * _deltatime, glm::vec3(0, 1, 0));
        cb_model.model = glm::scale(cb_model.model, glm::vec3(1, 1, 1));
        cb_scene.view_proj = g_cam.matrices.perspective * g_cam.matrices.view;

        if (USE_HOST_WRITABLE_HEAP)
        {
            memcpy(frame_cbs[buffer_index].mapped_data[0], &cb_model, sizeof(CB_MODEL));
            memcpy(frame_cbs[buffer_index].mapped_data[1], &cb_scene, sizeof(CB_SCENE));
        }
        else
        {
            auto&& ctx = dr->GetCopyContext();
            const auto CBV_ALIGNMENT = dr->GetDeviceAdapterLimits().min_constant_buffer_offset_alignment;
            ctx.CopyDataToBuffer(frame_cbs[buffer_index].buffer->GetB3DBuffer().Get()
                                 , 0
                                 , util::AlignUp(sizeof(CB_MODEL), CBV_ALIGNMENT)
                                 , &cb_model);
            ctx.CopyDataToBuffer(frame_cbs[buffer_index].buffer->GetB3DBuffer().Get()
                                 , util::AlignUp(sizeof(CB_MODEL), CBV_ALIGNMENT)
                                 , util::AlignUp(sizeof(CB_SCENE), CBV_ALIGNMENT)
                                 , &cb_scene);
        }
    }
}

void HelloConstantBuffer::Render(float _deltatime)
{
    if (window->GetWindowState() == WINDOW_STATE_MINIMIZE)
        return;

    auto&& frame_fence_value = submission_fence_values[buffer_index];

    // 送信情報を準備
    auto&& q = *present_queue;
    q.AddWaitFence(0, acquire_fence.Get());                                 // MoveToNextFrame()でacquireしたバッファが利用可能となるまでGPUで待機します。
    q.AddCommandList(0, command_lists[buffer_index].Get());                 // コマンドリスト
    q.AddSignalFence(0, submission_fence.Get(), current_fence_value + 1);   // コマンドリストの実行完了を通知するフェンスと、
    q.AddSignalFence(0, present_fence.Get());                               // スワップチェイン用のpresent_fenceをGPUでシグナルします。

    b::BMRESULT bmr{};
    // コマンドリストとフェンスを送信
    {
        // acquireしたバックバッファを利用するコマンドリストの実行完了をCPUで待機します。 
        bmr = submission_fence->Wait(frame_fence_value, UINT32_MAX);
        BMR_ASSERT(bmr);

        if (!is_reuse_commands)
            PrepareFrame(buffer_index, _deltatime);

        // 記録したコマンドを実行
        dr->QueueSubmit(*present_queue);
        BMR_ASSERT(bmr);
    }

    // バックバッファをプレゼント
    {
        // present_fenceのシグナルをGPUで待機します。
        b::SWAP_CHAIN_PRESENT_INFO pi{ present_fence.Get() };
        swapchain->SetPresentInfo(pi);
        bmr = swapchain->Present();
        BMR_ASSERT(bmr);
    }

    // シグナル予定のフェンス値へ増加させます
    current_fence_value++;
    frame_fence_value = current_fence_value;
}

void HelloConstantBuffer::OnResize(uint32_t _w, uint32_t _h)
{
    if (_w > 0 && _h > 0)
        g_cam.updateAspectRatio(window->GetAspectRatio());

    present_queue->WaitIdle();

    // 予めリサイズ前のバックバッファを参照するオブジェクト(フレームバッファ、RTV)を解放する必要があります。
    framebuffers.clear();

    swapchain->Resize(_w, _h);

    CreateFramebuffers();
    for (uint32_t i = 0; i < buffer_count; i++)
        PrepareFrame(i, 0.f);

    Update(0.f);
    Render(0.f);
}

void HelloConstantBuffer::PrepareFrame(uint32_t _buffer_index, float _deltatime)
{
    auto reset_flags = b::COMMAND_ALLOCATOR_RESET_FLAG_RELEASE_RESOURCES;
    command_allocators[_buffer_index]->Reset(reset_flags);

    auto&& l = command_lists[_buffer_index];
    b::COMMAND_LIST_BEGIN_DESC begin{};
    begin.flags            = b::COMMAND_LIST_BEGIN_FLAG_NONE;
    begin.inheritance_desc = nullptr;

    auto bmr = l->BeginRecord(begin);
    BMR_ASSERT(bmr);
    {
        b::CMD_PIPELINE_BARRIER barrier{};
        b::TEXTURE_BARRIER_DESC tb{};
        tb.type           = b::TEXTURE_BARRIER_TYPE_VIEW;
        tb.view           = swapchain->GetBuffers()[_buffer_index].rtv;
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

        auto descriptor_sets_data = descriptor_sets.data() + _buffer_index * 2;
        b::IDescriptorSet* sets[2] = { descriptor_sets_data[0].Get(), descriptor_sets_data[1].Get() };
        b::CMD_BIND_DESCRIPTOR_SETS bind_sets{};
        bind_sets.first_set                         = 0;
        bind_sets.num_descriptor_sets               = 2;
        bind_sets.descriptor_sets                   = sets;
        bind_sets.num_dynamic_descriptor_offsets    = 0;
        bind_sets.dynamic_descriptor_offsets        = nullptr;
        l->BindDescriptorSets(b::PIPELINE_BIND_POINT_GRAPHICS, bind_sets);

        static float sc = 0.f;
        static float sx = 0.f;
        sc = sc + 0.34f * _deltatime;
        sx = fabsf(sinf(sc));
        b::CLEAR_VALUE            clear_val{ b::CLEAR_RENDER_TARGET_VALUE{0.9f * sx , 0.28f, 0.13f, 1.f} };
        b::RENDER_PASS_BEGIN_DESC rpbd{ render_pass.Get(), framebuffers[_buffer_index].Get(), 1, &clear_val };
        b::SUBPASS_BEGIN_DESC     spbd{ b::SUBPASS_CONTENTS_INLINE };
        l->BeginRenderPass(rpbd, spbd);
        {
            auto&& resolution = swapchain->GetBufferDesc();
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

HelloConstantBuffer* HelloConstantBuffer::Create(PlatformBase& _platform)
{
    return new HelloConstantBuffer(_platform);
}

void HelloConstantBuffer::Destroy(HelloConstantBuffer* _app)
{
    delete _app;
}

BUMA_APPMODULE_API ApplicationBase* BUMA_APP_APIENTRY CreateApplication(PlatformBase& _platform)
{
    return HelloConstantBuffer::Create(_platform);
}

BUMA_APPMODULE_API void BUMA_APP_APIENTRY DestroyApplication(ApplicationBase* _app)
{
    HelloConstantBuffer::Destroy(static_cast<HelloConstantBuffer*>(_app));
}


}// namespace buma
