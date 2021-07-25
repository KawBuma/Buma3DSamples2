#include <HelloTexture/HelloTexture.h>

#include <TextureLoads/TextureLoads.h>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <Buma3DHelpers/Buma3DHelpers.h>
#include <Buma3DHelpers/B3DDescHelpers.h>
#include <Buma3DHelpers/B3DInit.h>

#include <DeviceResources/DeviceResources.h>
#include <DeviceResources/CommandQueue.h>
#include <DeviceResources/ResourceBuffer.h>
#include <DeviceResources/ResourceTexture.h>
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

namespace buma::tex
{
buma3d::RESOURCE_FORMAT GetDefaultFormat(const tex::TEXTURE_DESC& _tex_desc)
{
    static const buma3d::RESOURCE_FORMAT FORMAT_TABLE[3][4] = {
        { buma3d::RESOURCE_FORMAT_R8_SNORM  , buma3d::RESOURCE_FORMAT_R8G8_SNORM    , buma3d::RESOURCE_FORMAT_R8G8B8A8_SNORM     , buma3d::RESOURCE_FORMAT_R8G8B8A8_SNORM },
        { buma3d::RESOURCE_FORMAT_R8_UNORM  , buma3d::RESOURCE_FORMAT_R8G8_UNORM    , buma3d::RESOURCE_FORMAT_R8G8B8A8_UNORM     , buma3d::RESOURCE_FORMAT_R8G8B8A8_UNORM },
        { buma3d::RESOURCE_FORMAT_R16_FLOAT , buma3d::RESOURCE_FORMAT_R16G16_FLOAT  , buma3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT , buma3d::RESOURCE_FORMAT_R16G16B16A16_FLOAT }, };

    switch (_tex_desc.format)
    {
    case TEXTURE_FORMAT_SINT   : return FORMAT_TABLE[0][_tex_desc.component_count-1];
    case TEXTURE_FORMAT_UINT   : return FORMAT_TABLE[1][_tex_desc.component_count-1];
    case TEXTURE_FORMAT_SFLOAT : return FORMAT_TABLE[2][_tex_desc.component_count-1];

    default:
        return buma3d::RESOURCE_FORMAT_UNKNOWN;
    }
}
}// namespace buma::tex

namespace buma
{

HelloTexture::HelloTexture(PlatformBase& _platform)
    : SampleBase(_platform)
    , quad                      {}
    , cb_model                  {}
    , cb_scene                  {}
    , frame_cbs                 {}
    , vertex_buffer             {}
    , vertex_buffer_view        {}
    , texture                   {}
    , copyable_descriptor_pool  {}
    , buffer_descriptor_sets    {}
    , texture_descriptor_sets   {}
    , is_reuse_commands         {}
{
    g_fpss = new std::remove_pointer_t<decltype(g_fpss)>;
    platform.AddHelpMessage(
R"(========== HelloTexture Options ==========
--use-host-writable
    ホスト可視ヒープに定数を書き込みます。

)");
}

HelloTexture::~HelloTexture()
{
    delete g_fpss;
    g_fpss = nullptr;
}

#pragma region events

void HelloTexture::OnKeyDown(KeyDownEventArgs& _args)
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
void HelloTexture::OnKeyUp(KeyUpEventArgs& _args)
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
void HelloTexture::OnMouseMove(MouseMoveEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    auto&& mouse = *_args.mouse;
    g_cam.mouse.delta.x = (float)mouse.xrel;
    g_cam.mouse.delta.y = (float)mouse.yrel;
}
void HelloTexture::OnMouseButtonDown(MouseButtonDownEventArgs& _args)
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
void HelloTexture::OnMouseButtonUp(MouseButtonUpEventArgs& _args)
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
void HelloTexture::OnMouseWheel(MouseWheelEventArgs& _args)
{
    if (!window->IsWindowActive())
        return;

    g_cam.mouse.wheel = static_cast<float>(_args.wheel->y);
}

void HelloTexture::OnProcessWindow(ProcessWindowEventArgs& _args)
{
    if (_args.process_flags & WINDOW_PROCESS_FLAG_RESIZED)
    {
        uint32_t w, h;
        _args.window->GetWindowedSize(&w, &h);
        OnResize(w, h);
    }
}

bool HelloTexture::OnInit()
{
    PrepareSettings();

    CreateWindow(1280, 720, "HelloTexture - Does your driver perform dithered Alpha-To-Coverage?");

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

    CreateDescriptorHeapAndPool();
    AllocateDescriptorSets();

    CreateBuffers();
    CreateBufferViews();
    CreateConstantBuffer();
    CreateConstantBufferView();

    CreateDefaultSamplers();
    CreateTextureResource();
    CopyDataToTexture();
    CreateShaderResourceView();

    CreateDescriptorUpdate();
    UpdateDescriptorSet();

    // 描画コマンドを記録
    for (uint32_t i = 0; i < buffer_count; i++)
        PrepareFrame(i, 0.f);

    return true;
}

void HelloTexture::Tick(const util::StepTimer& _timer)
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

void HelloTexture::OnDestroy()
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
    dr->DestroyBuffer(vertex_buffer);
    vertex_buffer = nullptr;

    texture.data.reset();
    texture.srv.Reset();
    dr->DestroyTexture(texture.texture);
    texture.texture = nullptr;

    buffer_descriptor_sets.clear();
    texture_descriptor_sets.clear();
    copyable_descriptor_pool.Reset();

    DestroySampleBaseObjects();
}

#pragma endregion events

#pragma region preparing Buma3D objects

bool HelloTexture::LoadAssets()
{
    auto aspect_ratio = window->GetAspectRatio();
    quad = {
          { { -1.0f , 1.0f, 0.0f, 1.f }, { 0.f, 0.f } }
        , { {  1.0f,  1.0f, 0.0f, 1.f }, { 1.f, 0.f } }
        , { { -1.0f, -1.0f, 0.0f, 1.f }, { 0.f, 1.f } }
        , { {  1.0f, -1.0f, 0.0f, 1.f }, { 1.f, 1.f } }
    };

    cb_model.model = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
    cb_model.model = glm::rotate   (cb_model.model, 0.f, glm::vec3(0, 1, 0));
    cb_model.model = glm::scale    (cb_model.model, glm::vec3(1, 1, 1));

    cb_scene.view_proj = g_cam.matrices.perspective * g_cam.matrices.view;

    LoadTextureData();

    return true;
}
bool HelloTexture::LoadTextureData()
{
    tex::TEXTURE_CREATE_DESC texdesc{};
    auto path = AssetPath("Assets/texture/UV_Grid_Sm.jpg");
    //auto path = AssetPath("HelloTexture/UV_Grid_Sm.jpg");
    texdesc.filename    = path.c_str();
    texdesc.mip_count   = 0;
    texdesc.row_pitch_alignment   = dr->GetDeviceAdapterLimits().buffer_copy_row_pitch_alignment;
    texdesc.slice_pitch_alignment = dr->GetDeviceAdapterLimits().buffer_copy_offset_alignment;
    texture.data = tex::CreateTexturesFromFile(texdesc);
    RET_IF_FAILED(texture.data);

    return true;
}
bool HelloTexture::CreateDescriptorSetLayouts()
{
    set_layouts.resize(2);
    util::DescriptorSetLayoutDesc layout_desc(2);
    // 定数バッファ用レイアウト。 space0 に設定します。
    layout_desc
        //            (descriptor_type       , base_shader_register, num_descriptors, visibility                 , flags                  )
        .AddNewBinding(b::DESCRIPTOR_TYPE_CBV, 0                   , 1              , b::SHADER_VISIBILITY_VERTEX, b::DESCRIPTOR_FLAG_NONE) // model
        .AddNewBinding(b::DESCRIPTOR_TYPE_CBV, 1                   , 1              , b::SHADER_VISIBILITY_VERTEX, b::DESCRIPTOR_FLAG_NONE) // scene
        .SetFlags(b::DESCRIPTOR_SET_LAYOUT_FLAG_NONE)
        .Finalize();
    auto bmr = device->CreateDescriptorSetLayout(layout_desc.Get(), &set_layouts[BUF]);
    BMR_RET_IF_FAILED(bmr);

    // テクスチャ用レイアウト。 space1 に設定します。
    layout_desc
        .Reset()
        .AddNewBinding(b::DESCRIPTOR_TYPE_SRV_TEXTURE, 0, 1, b::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE, b::DESCRIPTOR_FLAG_NONE)
        .AddNewBinding(b::DESCRIPTOR_TYPE_SAMPLER    , 1, 1, b::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE, b::DESCRIPTOR_FLAG_NONE)
        .SetFlags(b::DESCRIPTOR_SET_LAYOUT_FLAG_NONE)
        .Finalize();
    bmr = device->CreateDescriptorSetLayout(layout_desc.Get(), &set_layouts[TEX]);
    BMR_RET_IF_FAILED(bmr);

    set_layouts[BUF]->SetName("HelloTexture::set_layouts[BUF]");
    set_layouts[TEX]->SetName("HelloTexture::set_layouts[TEX]");
    return true;
}
bool HelloTexture::CreatePipelineLayout()
{
    util::PipelineLayoutDesc desc(2, 0);
    desc.SetNumLayouts(2)
        .SetLayout(0, set_layouts[BUF].Get()) // space0 モデル定数, シーン定数
        .SetLayout(1, set_layouts[TEX].Get()) // space1 テクスチャ
        .SetFlags(b::PIPELINE_LAYOUT_FLAG_NONE)
        .Finalize();

    auto bmr = device->CreatePipelineLayout(desc.Get(), &pipeline_layout);
    BMR_RET_IF_FAILED(bmr);
    pipeline_layout->SetName("HelloTexture::pipeline_layout");

    return true;
}
bool HelloTexture::CreateShaderModules()
{
    shader_modules["VS"].Attach(CreateShaderModule("HelloTexture/shader/VertexShader.hlsl", buma3d::SHADER_STAGE_FLAG_VERTEX, "main"));
    shader_modules["PS"].Attach(CreateShaderModule("HelloTexture/shader/PixelShader.hlsl", buma3d::SHADER_STAGE_FLAG_PIXEL, "main"));

    return true;
}
bool HelloTexture::CreatePipeline()
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
            .SetStrideInBytes(sizeof(VERTEX))
            .SetClassification(buma3d::INPUT_CLASSIFICATION_PER_VERTEX_DATA)
            .AddNewInputElement("POSITION", 0, buma3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT)
            .AddNewInputElement("TEXCOORD", 0, buma3d::RESOURCE_FORMAT_R32G32_FLOAT)
            .Finalize()
        .Finalize();
    pso_desc.InputAssemblyState().topology = buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pso_desc.RasterizationState()
        .FillMode(buma3d::FILL_MODE_SOLID).CullMode(buma3d::CULL_MODE_NONE)
        .DepthClipEnabled(false).DepthBias(false);
    pso_desc.MultisampleState().AlphaToCoverage(true).Finalize();
    pso_desc.ViewportState().SetCounts(1, 1).Finalize(true, true);
    pso_desc.DepthStencilState().DepthStencilDisable();
    pso_desc.BlendState().Reset().SetNumAttachmemns(1).Finalize();
    pso_desc.Finalize();

    auto bmr = device->CreateGraphicsPipelineState(pso_desc.GetAsGraphics(), &pipeline);
    BMR_RET_IF_FAILED(bmr);
    pipeline->SetName("HelloTexture::pipeline");

    return true;
}
bool HelloTexture::CreateDescriptorHeapAndPool()
{
    util::DescriptorSizes sizes;
    sizes.IncrementSizes(set_layouts[BUF].Get(), buffer_count)
         .IncrementSizes(set_layouts[TEX].Get(), buffer_count)
         .Finalize();

    auto bmr = device->CreateDescriptorHeap(sizes.GetAsHeapDesc(b::DESCRIPTOR_HEAP_FLAG_NONE, b::B3D_DEFAULT_NODE_MASK), &descriptor_heap);
    BMR_RET_IF_FAILED(bmr);

    sizes.Reset();
    sizes.IncrementSizes(set_layouts[BUF].Get(), buffer_count)
         .IncrementSizes(set_layouts[TEX].Get(), buffer_count - 1)
         .Finalize();

    bmr = device->CreateDescriptorPool(sizes.GetAsPoolDesc(descriptor_heap.Get(), sizes.GetMaxSetsByTotalMultiplyCount(), b::DESCRIPTOR_POOL_FLAG_NONE), &descriptor_pool);
    BMR_RET_IF_FAILED(bmr);

    // DESCRIPTOR_POOL_FLAG_COPY_SRCフラグで作成されたプールから割り当てたディスクリプタセットはコピーソースとして使用可能です。
    // デモンストレーションのため、今回はフレーム0番目のテクスチャディスクリプタセットをコピー可能プールから割り当てます。
    // (このサンプルにおいてテクスチャとサンプラーは可変でないため、実際にはフレーム毎にディスクリプタセットを割り当てる必要はありません。)
    sizes.Reset()
         .IncrementSizes(set_layouts[TEX].Get(), 1)
         .Finalize();
    bmr = device->CreateDescriptorPool(sizes.GetAsPoolDesc(descriptor_heap.Get(), sizes.GetMaxSetsByTotalMultiplyCount(), b::DESCRIPTOR_POOL_FLAG_COPY_SRC), &copyable_descriptor_pool);
    BMR_RET_IF_FAILED(bmr);

    return true;
}
bool HelloTexture::AllocateDescriptorSets()
{
    util::DescriptorSetAllocateDesc allocate_desc(buffer_count + buffer_count - 1);

    // buffer_descriptor_sets,texture_descriptor_setsをbuffer_count数分割り当てます。
    buffer_descriptor_sets.resize(buffer_count);
    texture_descriptor_sets.resize(buffer_count);

    {
        allocate_desc.SetNumDescriptorSets(buffer_count + buffer_count - 1);
        for (uint32_t i = 0; i < buffer_count; i++)
            allocate_desc.SetDescriptorSetLayout(i, set_layouts[BUF].Get());
        allocate_desc.SetDescriptorSetLayout(buffer_count    , set_layouts[TEX].Get());
        allocate_desc.SetDescriptorSetLayout(buffer_count + 1, set_layouts[TEX].Get());
        allocate_desc.Finalize();

        auto&& [num_sets, dst_sets] = allocate_desc.GetDst();
        auto bmr = descriptor_pool->AllocateDescriptorSets(allocate_desc.Get(), dst_sets);
        BMR_RET_IF_FAILED(bmr);

        for (uint32_t i = 0; i < buffer_count; i++)
            buffer_descriptor_sets[i] = dst_sets[i];

        texture_descriptor_sets[1] = dst_sets[buffer_count];
        texture_descriptor_sets[2] = dst_sets[buffer_count + 1];
    }

    // コピー用プールから割り当て
    {
        allocate_desc.Reset()
            .SetNumDescriptorSets(1)
            .SetDescriptorSetLayout(0, set_layouts[TEX].Get())
            .Finalize();

        auto&& [num_sets, dst_sets] = allocate_desc.GetDst();
        auto bmr = copyable_descriptor_pool->AllocateDescriptorSets(allocate_desc.Get(), dst_sets);
        BMR_RET_IF_FAILED(bmr);

        texture_descriptor_sets[0] = dst_sets[0];
    }

    return true;
}
bool HelloTexture::CreateConstantBuffer()
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
bool HelloTexture::CreateConstantBufferView()
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
bool HelloTexture::CreateTextureResource()
{
    auto&& data_desc = texture.data->GetDesc();
    texture.texture = dr->CreateTexture(init::Tex2DResourceDesc({ (uint32_t)data_desc.width, (uint32_t)data_desc.height }
                                                                , tex::GetDefaultFormat(data_desc), init::TEX_STATIC_SRV_FLAGS, data_desc.num_mips, data_desc.depth)
                                        , b::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL);
    RET_IF_FAILED(texture.texture);

    return true;
}
bool HelloTexture::CopyDataToTexture()
{
    util::PipelineBarrierDesc bd{};
    util::TextureBarrierRange tex(&bd);
    tex .SetTexture(texture.texture->GetB3DTexture().Get())
        .AddSubresRange(b::TEXTURE_ASPECT_FLAG_COLOR, 0, 0, 1, texture.texture->GetDesc().texture.mip_levels)
        .Finalize();

    auto&& ctx = dr->GetCopyContext();
    bd.AddTextureBarrierRange(&tex.Get(), b::RESOURCE_STATE_UNDEFINED, b::RESOURCE_STATE_COPY_DST_WRITE);
    ctx.PipelineBarrier(bd.SetPipelineStageFalgs(b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE, b::PIPELINE_STAGE_FLAG_COPY_RESOLVE).Finalize().Get());

    auto&& data_desc = texture.data->GetDesc();
    for (size_t i = 0; i < data_desc.num_mips; i++)
    {
        auto&& tex_data = texture.data->Get(i);
        ctx.CopyDataToTexture(texture.texture->GetB3DTexture().Get(), (uint32_t)i, 0
                                   , tex_data->layout.row_pitch
                                   , tex_data->extent.h
                                   , tex_data->total_size, tex_data->data);
    }

    bd.Reset();
    if (ctx.GetCommandType() == b::COMMAND_TYPE_COPY_ONLY)
    {
        // 所有権をコピーキューから開放します。
        bd.AddTextureBarrierRange(&tex.Get(), b::RESOURCE_STATE_COPY_DST_WRITE, b::RESOURCE_STATE_SHADER_READ
                                  , b::RESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFER, b::COMMAND_TYPE_COPY_ONLY, b::COMMAND_TYPE_DIRECT);
        ctx.PipelineBarrier(bd.SetPipelineStageFalgs(b::PIPELINE_STAGE_FLAG_COPY_RESOLVE, b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE).Finalize().Get());
    }
    else
    {
        bd.AddTextureBarrierRange(&tex.Get(), b::RESOURCE_STATE_COPY_DST_WRITE, b::RESOURCE_STATE_SHADER_READ);
        ctx.PipelineBarrier(bd.SetPipelineStageFalgs(b::PIPELINE_STAGE_FLAG_COPY_RESOLVE, b::PIPELINE_STAGE_FLAG_ALL_GRAPHICS).Finalize().Get());
    }

    // コピーキューから開放された所有権をグラフィックキューで取得します。
    if (ctx.GetCommandType() == b::COMMAND_TYPE_COPY_ONLY)
    {
        bd.Reset();
        bd.AddTextureBarrierRange(&tex.Get(), b::RESOURCE_STATE_COPY_DST_WRITE, b::RESOURCE_STATE_SHADER_READ
                                  , b::RESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFER, b::COMMAND_TYPE_COPY_ONLY, b::COMMAND_TYPE_DIRECT);
        ctx.PipelineBarrier(bd.SetPipelineStageFalgs(b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE, b::PIPELINE_STAGE_FLAG_ALL_GRAPHICS).Finalize().Get());
    }

    return true;
}
bool HelloTexture::CreateShaderResourceView()
{
    buma3d::SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.view.type          = b::VIEW_TYPE_SHADER_RESOURCE;
    srvd.view.format        = texture.texture->GetDesc().texture.format_desc.format;
    srvd.view.dimension     = b::VIEW_DIMENSION_TEXTURE_2D;
    srvd.texture.components                             = { b::COMPONENT_SWIZZLE_IDENTITY, b::COMPONENT_SWIZZLE_IDENTITY, b::COMPONENT_SWIZZLE_IDENTITY, b::COMPONENT_SWIZZLE_IDENTITY };
    srvd.texture.subresource_range.offset.aspect        = b::TEXTURE_ASPECT_FLAG_COLOR;
    srvd.texture.subresource_range.offset.mip_slice     = 0;
    srvd.texture.subresource_range.offset.array_slice   = 0;
    srvd.texture.subresource_range.mip_levels           = b::B3D_USE_REMAINING_MIP_LEVELS;
    srvd.texture.subresource_range.array_size           = 1;
    srvd.flags                                          = b::SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT;

    auto bmr = device->CreateShaderResourceView(texture.texture->GetB3DTexture().Get(), srvd, &texture.srv);
    BMR_RET_IF_FAILED(bmr);
    
    return true;
}
bool HelloTexture::CreateBuffers()
{
    auto vbdesc = init::BufferResourceDesc(sizeof(VERTEX) * quad.size(), init::BUF_COPYABLE_FLAGS | b::BUFFER_USAGE_FLAG_VERTEX_BUFFER);
    vertex_buffer = dr->CreateBuffer(vbdesc, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL);
    vertex_buffer->SetName("HelloTexture::vertex_buffer");
    BUMA_ASSERT(vertex_buffer);

    // 頂点、インデックスバッファデータをデバイスローカルバッファへコピー
    auto&& ctx = dr->GetCopyContext();
    ctx.CopyDataToBuffer(vertex_buffer->GetB3DBuffer().Get(), 0, vertex_buffer->GetDesc().buffer.size_in_bytes, quad.data());

    return true;
}
bool HelloTexture::CreateBufferViews()
{
    // 頂点バッファビューを作成
    util::VertexBufferViewDesc vbvdesc(1);
    vbvdesc.SetView(0, 0, (uint32_t)vertex_buffer->GetDesc().buffer.size_in_bytes, sizeof(VERTEX));
    auto bmr = device->CreateVertexBufferView(vertex_buffer->GetB3DBuffer().Get(), vbvdesc.Finalize().Get(), &vertex_buffer_view);
    BMR_RET_IF_FAILED(bmr);

    return true;
}
bool HelloTexture::UpdateDescriptorSet()
{
    util::UpdateDescriptorSetDesc update_desc{};
    for (uint32_t i_frame = 0; i_frame < buffer_count; i_frame++)
    {
        update_desc.AddNewWriteDescriptorSet()
            .SetDst(buffer_descriptor_sets[i_frame].Get())
            .AddNewWriteDescriptorBinding().SetNumDescriptors(1).SetDstBinding(0, 0).SetSrcView(0, frame_cbs[i_frame].model_cbv.Get()).Finalize()
            .AddNewWriteDescriptorBinding().SetNumDescriptors(1).SetDstBinding(1, 0).SetSrcView(0, frame_cbs[i_frame].scene_cbv.Get()).Finalize()
            .Finalize();
    
        if (i_frame == 0)// 最初のセットのみに書き込みます。 
        {
            update_desc.AddNewWriteDescriptorSet()
                .SetDst(texture_descriptor_sets[0].Get())
                .AddNewWriteDescriptorBinding().SetNumDescriptors(1).SetDstBinding(0, 0).SetSrcView(0, texture.srv.Get()).Finalize()
                .AddNewWriteDescriptorBinding().SetNumDescriptors(1).SetDstBinding(1, 0).SetSrcView(0, sampler_aniso.Get()).Finalize()
                .Finalize();
        }
        else
        {
            // テクスチャ、サンプラをコピー
            update_desc.AddNewCopyDescriptorSet()
                .SetDst(texture_descriptor_sets[i_frame].Get()).SetSrc(texture_descriptor_sets[0].Get())
                .AddNewCopyBinding().SetNumDescriptors(1).SetDstBinding(0).SetSrcBinding(0).Finalize() // テクスチャ
                .AddNewCopyBinding().SetNumDescriptors(1).SetDstBinding(1).SetSrcBinding(1).Finalize() // サンプラー
                .Finalize();
        }
    }
    update_desc.Finalize();
    
    auto bmr = descriptor_update->UpdateDescriptorSets(update_desc.Get());
    BMR_RET_IF_FAILED(bmr);
    
    return true;
}

#pragma endregion preparing Buma3D objects

void HelloTexture::MoveToNextFrame()
{
    // プレゼント可能となるバッファのインデックスを取得します。
    // 加えてフェンスにシグナル操作を行います。取得されたインデックスのバッファが利用可能となるタイミングを通知します。
    swapchain->SetAcquireInfo(acquire_fence.Get(), nullptr);
    auto bmr = swapchain->AcquireNextBuffer(0, &buffer_index);
    BUMA_ASSERT(bmr == b::BMRESULT_SUCCEED || bmr == b::BMRESULT_SUCCEED_NOT_READY);
}

void HelloTexture::Update(float _deltatime)
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
        cb_model.model = glm::rotate(cb_model.model, angle /*+= 0.3f * _deltatime*/, glm::vec3(0, 1, 0));
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

void HelloTexture::Render(float _deltatime)
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

void HelloTexture::OnResize(uint32_t _w, uint32_t _h)
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

void HelloTexture::PrepareFrame(uint32_t _buffer_index, float _deltatime)
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

        b::IDescriptorSet* sets[2] = { buffer_descriptor_sets[_buffer_index].Get(), texture_descriptor_sets[_buffer_index].Get() };
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

            //      { vertex_count_per_instance, instance_count, start_vertex_location, start_instance_location }
            l->Draw({ 4                        , 1             , 0                    , 0                       });
        }
        l->EndRenderPass({});
    }
    bmr = l->EndRecord();
    BMR_ASSERT(bmr);
}

HelloTexture* HelloTexture::Create(PlatformBase& _platform)
{
    return new HelloTexture(_platform);
}

void HelloTexture::Destroy(HelloTexture* _app)
{
    delete _app;
}

BUMA_APPMODULE_API ApplicationBase* BUMA_APP_APIENTRY CreateApplication(PlatformBase& _platform)
{
    return HelloTexture::Create(_platform);
}

BUMA_APPMODULE_API void BUMA_APP_APIENTRY DestroyApplication(ApplicationBase* _app)
{
    HelloTexture::Destroy(static_cast<HelloTexture*>(_app));
}


}// namespace buma
