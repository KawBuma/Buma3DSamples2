#include <MyImgui/MyImgui.h>
#include "./MyImguiRenderer.h"

#include <DeviceResources/DeviceResources.h>
#include <DeviceResources/ResourceBuffer.h>
#include <DeviceResources/ResourceTexture.h>
#include <DeviceResources/CommandQueue.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
//#include <libloaderapi.h>
#include <Windows.h>
#undef CreateWindow

namespace b = buma3d;

namespace buma
{
namespace gui
{

MyImGuiRenderer::MyImGuiRenderer(RENDER_RESOURCE& _rr, bool _is_viewport_renderer, bool _is_primary_renderer)
    : rr                        { _rr }
    , is_viewport_renderer      { _is_viewport_renderer }
    , is_primary_renderer       { _is_primary_renderer }
    , total_vtx_count           {}
    , total_idx_count           {}
    , vertex_buffer             {}
    , index_buffer              {}
    , vertex_buffer_view        {}
    , index_buffer_view         {}
    , submit                    {}
    , update_desc               {}
    , texid_descriptors         {}
    , texid_descriptors_offset  {}
    , current_texid_descriptors {}
    , timeline_fence            {}
    , timeline_fence_val        {}
    , allocator                 {}
    , list                      {}
    , current_list              {}
    , args                      {}
    , progress                  {}
{
    Init();
}


MyImGuiRenderer::~MyImGuiRenderer()
{
    if (vertex_buffer)
    {
        rr.dr->DestroyBuffer(vertex_buffer);
        vertex_buffer = nullptr;
    }
    if (index_buffer)
    {
        rr.dr->DestroyBuffer(index_buffer);
        index_buffer = nullptr;
    }
}

void MyImGuiRenderer::Init()
{
    static const b::CLEAR_VALUE cv{ 0.f, 0.f, 0.f, 1.f };
    args.ca = { b::TEXTURE_ASPECT_FLAG_COLOR, 0, &cv };
    args.cas = { 1, &args.ca };

    auto bmr = rr.device->CreateDescriptorUpdate({}, &descriptor_update);
    assert(util::IsSucceeded(bmr));

    AddNewTexIdDescriptors();

    if (!is_primary_renderer && rr.flags & MYIMGUI_CREATE_FLAG_USE_SINGLE_COMMAND_LIST)
        return;

    //if (!(rr.flags & MYIMGUI_CREATE_FLAG_DESCRIPTOR_POOL_FEEDING))
    submit = std::make_unique<SUBMIT>();

    bmr = rr.device->CreateCommandAllocator(buma3d::init::CommandAllocatorDesc(rr.queue->GetCommandType(), buma3d::COMMAND_LIST_LEVEL_PRIMARY, buma3d::COMMAND_ALLOCATOR_FLAG_TRANSIENT), &allocator);
    assert(util::IsSucceeded(bmr));
    allocator->SetName("MyImGuiRenderer::allocator");

    bmr = rr.device->AllocateCommandList(buma3d::init::CommandListDesc(allocator.Get(), buma3d::B3D_DEFAULT_NODE_MASK), &list);
    assert(util::IsSucceeded(bmr));
    list->SetName("MyImGuiRenderer::list");

    bmr = rr.device->CreateFence(buma3d::init::TimelineFenceDesc(), &timeline_fence);
    assert(util::IsSucceeded(bmr));
    timeline_fence->SetName("MyImGuiRenderer::timeline_fence");
}

void MyImGuiRenderer::ClearViewportRendererRtv(buma3d::ICommandList* _list, ImDrawData* _draw_data)
{
    if (is_viewport_renderer)
    {
        //b::SCISSOR_RECT rect{ { 0,0 }, { (uint32_t)_draw_data->DisplaySize.x, (uint32_t)_draw_data->DisplaySize.y } };
        //args.cas.num_rects = 1;
        //args.cas.rects = &rect;
        //_list->ClearAttachments(args.cas);
    }
}

void MyImGuiRenderer::BeginRecord(buma3d::ICommandList* _list)
{
    if (_list)
    {
        current_list = _list;
    }
    else
    {
        timeline_fence->Wait(timeline_fence_val.wait(), UINT32_MAX);
        allocator->Reset(b::COMMAND_ALLOCATOR_RESET_FLAG_NONE);
        current_list = list.Get();
        current_list->BeginRecord(args.list_begin_desc);
    }
}
void MyImGuiRenderer::Draw(buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data)
{
    if (_draw_data->CmdListsCount == 0)
        return;

    {
        progress.imcmd_list_offset      = 0;
        progress.imcmd_buffer_offset    = 0;
        progress.start_index_location   = 0;
        progress.texid_set_offset       = 0;
        progress.has_done               = false;

        args.draw_indexed.instance_count        = 1;
        args.draw_indexed.start_index_location  = 0;
        args.draw_indexed.base_vertex_location  = 0;

        float L = _draw_data->DisplayPos.x;
        float R = _draw_data->DisplayPos.x + _draw_data->DisplaySize.x;
        float T = _draw_data->DisplayPos.y;
        float B = _draw_data->DisplayPos.y + _draw_data->DisplaySize.y;
        auto Set = [](float _m[4], float _v0, float _v1, float _v2, float _v3) { _m[0] = _v0; _m[1] = _v1; _m[2] = _v2; _m[3] = _v3; };
        Set(args.mvp[0], 2.0f / (R - L)       , 0.0f                    , 0.0f              , 0.0f);
        Set(args.mvp[1], 0.0f                 , 2.0f / (T - B)          , 0.0f              , 0.0f);
        Set(args.mvp[2], 0.0f                 , 0.0f                    , 0.5f              , 0.0f);
        Set(args.mvp[3], (R + L) / (L - R)    , (T + B) / (B - T)       , 0.5f              , 1.0f);

        args.push_constants.index                      = 0;
        args.push_constants.num32_bit_values_to_set    = sizeof(args.mvp) / 4;
        args.push_constants.src_data                   = args.mvp;
        args.push_constants.dst_offset_in_32bit_values = 0;

        args.bind_vbv.num_views = 1;
        args.bind_vbv.views     = vertex_buffer_view.GetAddressOf();

        args.viewport.width      = _draw_data->DisplaySize.x;
        args.viewport.height     = _draw_data->DisplaySize.y;
        args.viewport.min_depth  = 0.0f;
        args.viewport.max_depth  = 1.0f;

        args.render_pass_begin.render_pass      = rr.render_pass_clear.Get();
        args.render_pass_begin.framebuffer      = _framebuffer;
        args.render_pass_begin.num_clear_values = 0;
        args.render_pass_begin.clear_values     = nullptr;
    }

    texid_descriptors_offset = 0;
    ChangeTexIdDescriptors();
    UpdateDescriptorSets(_draw_data);

    PrepareBuffers(_draw_data);
    PrepareDraw             (current_list, _framebuffer, _draw_data);
    ClearViewportRendererRtv(current_list, _draw_data);
    BeginDraw               (current_list, _framebuffer, _draw_data);
    EndDraw                 (current_list);
}
void MyImGuiRenderer::EndRecord(buma3d::ICommandList* _list)
{
    if (_list)
    {
        return;
    }
    else
    {
        auto bmr = current_list->EndRecord();
        assert(util::IsSucceeded(bmr));
    }
}
void MyImGuiRenderer::RecordGuiCommands(buma3d::IFramebuffer* _framebuffer, buma3d::RESOURCE_STATE _current_state, buma3d::RESOURCE_STATE _state_after, ImDrawData* _draw_data)
{
    auto rtv = _framebuffer->GetDesc().attachments[0]->As<buma3d::IRenderTargetView>();
    if (_current_state != b::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE)
    {
        args.barrier_desc.Reset().AddTextureBarrier(rtv, _current_state, b::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE)
                                 .SetPipelineStageFalgs(b::PIPELINE_STAGE_FLAG_TOP_OF_PIPE, b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT);
        current_list->PipelineBarrier(args.barrier_desc.Finalize().Get());
    }

    Draw(_framebuffer, _draw_data);

    if (_state_after != b::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE)
    {
        args.barrier_desc.Reset().AddTextureBarrier(rtv, b::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE, _state_after)
                                 .SetPipelineStageFalgs(b::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT, b::PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE);
        current_list->PipelineBarrier(args.barrier_desc.Get());
    }
}
void MyImGuiRenderer::AddSubmitInfoTo(CommandQueue* _queue)
{
    if (list == nullptr || current_list != list.Get())// 外部から指定されたコマンドリストの場合、送信する操作はありません。
        return;

    auto&& si = submit->info;
    si.wait_fence.num_fences        = 0;
    si.num_command_lists_to_execute = 1;
    si.command_lists_to_execute     = list.GetAddressOf();

    auto signal_val = timeline_fence_val.signal();
    si.signal_fence.num_fences      = 1;
    si.signal_fence.fences          = timeline_fence.GetAddressOf();
    si.signal_fence.fence_values    = &signal_val;

    rr.queue->AddSubmitInfo(1, si);
    ++timeline_fence_val;
    rr.dr->QueueSubmit(*rr.queue);
}
void MyImGuiRenderer::PrepareDraw(buma3d::ICommandList* _list, buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data)
{
    _list->InsertMarker("MyImGuiRenderer::PrepareDraw", nullptr);
    _list->SetPipelineLayout(b::PIPELINE_BIND_POINT_GRAPHICS, rr.pipeline_layout.Get());
    _list->SetPipelineState(rr.pipeline_state_load.Get());

    _list->Push32BitConstants(b::PIPELINE_BIND_POINT_GRAPHICS, args.push_constants);
    b::IDescriptorSet* sets[] = { current_texid_descriptors->sampler_set.Get(), current_texid_descriptors->font_set.Get(), };
    _list->BindDescriptorSets(b::PIPELINE_BIND_POINT_GRAPHICS, b::CMD_BIND_DESCRIPTOR_SETS{ 0, 2, sets });
    progress.has_bound_font_set = true;


    _list->BindVertexBufferViews(args.bind_vbv);
    _list->BindIndexBufferView(index_buffer_view.Get());
    _list->SetViewports(1, &args.viewport);

    if (is_primary_renderer || texid_descriptors_offset > 1)
    {
        args.render_pass_begin.render_pass = rr.render_pass_load.Get();
        _list->BeginRenderPass(args.render_pass_begin, args.subpass_begin);
        args.render_pass_begin.render_pass = rr.render_pass_clear.Get();
    }
    else
    {
        buma3d::CLEAR_VALUE cv{};
        args.render_pass_begin.num_clear_values = 1;
        args.render_pass_begin.clear_values = &cv;
        _list->BeginRenderPass(args.render_pass_begin, args.subpass_begin);
    }

}
void MyImGuiRenderer::PrepareBuffers(ImDrawData* _draw_data)
{
    PrepareVertexBuffer(_draw_data);
    PrepareIndexBuffer(_draw_data);

    uint32_t idx_offset = 0;
    uint32_t vtx_offset = 0;
    for (int i_cmd = 0; i_cmd < _draw_data->CmdListsCount; i_cmd++)
    {
        auto&& draw_list = _draw_data->CmdLists[i_cmd];
        memcpy(index_buffer->GetMppedDataAs<ImDrawIdx>(idx_offset)
               , draw_list->IdxBuffer.Data
               , draw_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_offset += draw_list->IdxBuffer.Size;

        memcpy(vertex_buffer->GetMppedDataAs<ImDrawVert>(vtx_offset)
               , draw_list->VtxBuffer.Data
               , draw_list->VtxBuffer.Size * sizeof(ImDrawVert));
        vtx_offset += draw_list->VtxBuffer.Size;
    }
}
void MyImGuiRenderer::PrepareIndexBuffer(ImDrawData* _draw_data)
{
    if (total_idx_count < (uint32_t)_draw_data->TotalIdxCount)
    {
        total_idx_count = (uint32_t)_draw_data->TotalIdxCount;
        index_buffer_view.Reset();
        if (index_buffer)
            rr.dr->DestroyBuffer(index_buffer);
        index_buffer = rr.dr->CreateBuffer(buma3d::init::BufferResourceDesc(sizeof(ImDrawIdx) * (uint64_t)total_idx_count
                                                                                 , b::BUFFER_USAGE_FLAG_COPY_DST | b::BUFFER_USAGE_FLAG_INDEX_BUFFER)
                           , buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT);

        buma3d::INDEX_BUFFER_VIEW_DESC ibvd{};
        ibvd.buffer_offset  = 0;
        ibvd.size_in_bytes  = sizeof(ImDrawIdx) * total_idx_count;
        ibvd.index_type     = sizeof(ImDrawIdx) == 4 ? b::INDEX_TYPE_UINT32 : b::INDEX_TYPE_UINT16;
        auto bmr = rr.device->CreateIndexBufferView(index_buffer->GetB3DBuffer().Get(), ibvd, &index_buffer_view);
        assert(util::IsSucceeded(bmr));
    }
}
void MyImGuiRenderer::PrepareVertexBuffer(ImDrawData* _draw_data)
{
    if (total_vtx_count < (uint32_t)_draw_data->TotalVtxCount)
    {
        total_vtx_count = (uint32_t)_draw_data->TotalVtxCount;
        vertex_buffer_view.Reset();
        if (vertex_buffer)
            rr.dr->DestroyBuffer(vertex_buffer);
        vertex_buffer = rr.dr->CreateBuffer(buma3d::init::BufferResourceDesc(sizeof(ImDrawVert) * (uint64_t)total_vtx_count
                                                                                  , b::BUFFER_USAGE_FLAG_COPY_DST | b::BUFFER_USAGE_FLAG_VERTEX_BUFFER)
                                            , buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT);

        buma3d::VERTEX_BUFFER_VIEW_DESC vbvd{};
        uint64_t buffer_offset      = 0;
        uint32_t size_in_bytes      = sizeof(ImDrawVert) * total_vtx_count;
        uint32_t stride_in_bytes    = sizeof(ImDrawVert);
        vbvd.num_input_slots  = 1;
        vbvd.buffer_offsets   = &buffer_offset;
        vbvd.sizes_in_bytes   = &size_in_bytes;
        vbvd.strides_in_bytes = &stride_in_bytes;
        auto bmr = rr.device->CreateVertexBufferView(vertex_buffer->GetB3DBuffer().Get(), vbvd, &vertex_buffer_view);
        assert(util::IsSucceeded(bmr));
    }
}
void MyImGuiRenderer::BeginDraw(buma3d::ICommandList* _list, buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data)
{
    // TextureIdを設定するためには、引数の_cmd_listに直接記録するのではなく、
    // 追加でコマンドリストを作成し、複数のディスクリプタを割り当てられるようにし、その追加のコマンドリストで描画したものを引数のコマンドリストに指定する必要がある。

    texid_descriptors_offset = 0;
    ChangeTexIdDescriptors();
    auto texid_sets_data            = current_texid_descriptors->texid_sets.data();
    auto max_sets_allocation_count  = current_texid_descriptors->descriptor_pool->GetDesc().max_sets_allocation_count;

    auto&& display_pos = _draw_data->DisplayPos;
    for (int i_cmd = progress.imcmd_list_offset; i_cmd < _draw_data->CmdListsCount; i_cmd++)
    {
        auto&& draw_list = _draw_data->CmdLists[i_cmd];
        for (int i_cmd_buf = progress.imcmd_buffer_offset; i_cmd_buf < draw_list->CmdBuffer.Size; i_cmd_buf++)
        {
            auto&& draw_cmd = draw_list->CmdBuffer.Data[i_cmd_buf];
            auto&& clip_rect = draw_cmd.ClipRect;
            if (!IsScissorValid(display_pos, clip_rect))
            {
                progress.imcmd_buffer_offset++;
                args.draw_indexed.start_index_location += draw_cmd.ElemCount;
                continue;
            }

            args.scissor_rect.offset.x      = static_cast<int32_t>(clip_rect.x - display_pos.x);
            args.scissor_rect.offset.y      = static_cast<int32_t>(clip_rect.y - display_pos.y);
            args.scissor_rect.extent.width  = static_cast<uint32_t>(clip_rect.z - clip_rect.x);
            args.scissor_rect.extent.height = static_cast<uint32_t>(clip_rect.w - clip_rect.y);
            _list->SetScissorRects(1, &args.scissor_rect);

            if (draw_cmd.TextureId)
            {
                _list->BindDescriptorSets(b::PIPELINE_BIND_POINT_GRAPHICS, { 1, 1, texid_sets_data[progress.texid_set_offset++].GetAddressOf() });
                progress.has_bound_font_set = false;
            }
            else if (!progress.has_bound_font_set)
            {
                _list->BindDescriptorSets(b::PIPELINE_BIND_POINT_GRAPHICS, { 1, 1, current_texid_descriptors->font_set.GetAddressOf() });
                progress.has_bound_font_set = true;
            }

            args.draw_indexed.index_count_per_instance = draw_cmd.ElemCount;
            _list->DrawIndexed(args.draw_indexed);

            progress.imcmd_buffer_offset++;
            if (progress.texid_set_offset == max_sets_allocation_count)
            {
                FlushDraw(_list, _framebuffer, _draw_data);
                texid_sets_data           = current_texid_descriptors->texid_sets.data();
                max_sets_allocation_count = current_texid_descriptors->descriptor_pool->GetDesc().max_sets_allocation_count;
            }
            args.draw_indexed.start_index_location += draw_cmd.ElemCount;
        }
        progress.imcmd_buffer_offset = 0;
        progress.imcmd_list_offset++;
        args.draw_indexed.base_vertex_location += draw_list->VtxBuffer.Size;
    }

    progress.has_done = true;
}
void MyImGuiRenderer::FlushDraw(buma3d::ICommandList* _list, buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data)
{
    if (rr.flags & MYIMGUI_CREATE_FLAG_DESCRIPTOR_POOL_FEEDING)
    {
        ChangeTexIdDescriptors();
    }
    else
    {
        assert(_list == list.Get());
        // 予約したディスクリプタセットの上限値に達したため、一旦コマンドを実行します。
        EndDraw(_list);
        EndRecord();
        Submit(_list);
        BeginRecord();
        UpdateDescriptorSets(_draw_data);
        PrepareDraw(_list, _framebuffer, _draw_data);
    }
}
void MyImGuiRenderer::Submit(buma3d::ICommandList* _list)
{
    auto&& si = submit->info;
    si.wait_fence.num_fences        = 0;
    si.num_command_lists_to_execute = 1;
    si.command_lists_to_execute     = &_list;

    auto signal_val = timeline_fence_val.signal();
    si.signal_fence.num_fences      = 1;
    si.signal_fence.fences          = timeline_fence.GetAddressOf();
    si.signal_fence.fence_values    = &signal_val;

    // TODO: 一貫性のため、コマンドの送信を行う関数はMyImguiクラスと統一すべき。

    rr.queue->GetCommandQueue()->InsertMarker("MyImGuiRenderer::Submit", nullptr);

    auto&& sd = submit->desc;
    sd.num_submit_infos    = 1;
    sd.submit_infos        = &si;
    sd.signal_fence_to_cpu = nullptr;
    auto bmr = rr.queue->GetCommandQueue()->Submit(sd);
    assert(util::IsSucceeded(bmr));

    ++timeline_fence_val;
}
void MyImGuiRenderer::UpdateDescriptorSets(ImDrawData* _draw_data)
{
    auto texid_sets_data            = current_texid_descriptors->texid_sets.data();
    auto max_sets_allocation_count  = current_texid_descriptors->descriptor_pool->GetDesc().max_sets_allocation_count;

    update_desc.Reset();
    uint32_t num_texid_sets = 0;
    auto Update = [&]()
    {
        update_desc.Finalize();
        auto&& ud = update_desc.Get();
        if (ud.num_write_descriptor_sets != 0)
        {
            auto bmr = descriptor_update->UpdateDescriptorSets(update_desc.Get());
            assert(util::IsSucceeded(bmr));
        }
        update_desc.Reset();
        num_texid_sets = 0;
    };

    auto&& display_pos = _draw_data->DisplayPos;
    for (int i_cmd = progress.imcmd_list_offset; i_cmd < _draw_data->CmdListsCount; i_cmd++)
    {
        auto&& draw_list = _draw_data->CmdLists[i_cmd];
        for (int i_cmd_buf = progress.imcmd_buffer_offset; i_cmd_buf < draw_list->CmdBuffer.Size; i_cmd_buf++)
        {
            auto&& draw_cmd = draw_list->CmdBuffer.Data[i_cmd_buf];
            if (draw_cmd.UserCallback)
                continue;

            auto&& clip_rect = draw_cmd.ClipRect;
            if (!IsScissorValid(display_pos, clip_rect))
                continue;

            if (draw_cmd.TextureId)
            {
                update_desc
                    .AddNewWriteDescriptorSet()
                    .SetDst(texid_sets_data[num_texid_sets++].Get())
                        .AddNewWriteDescriptorBinding()
                        .SetNumDescriptors(1)
                        .SetDstBinding(0, 0)
                        .SetSrcView(0, reinterpret_cast<b::IShaderResourceView*>(draw_cmd.TextureId))
                        .Finalize()
                    .Finalize();
            }

            if (num_texid_sets == max_sets_allocation_count)
            {
                if (rr.flags & MYIMGUI_CREATE_FLAG_DESCRIPTOR_POOL_FEEDING)
                {
                    Update();
                    AddNewTexIdDescriptors();
                }
                else
                {
                    goto fulled;
                }
            }
        }
    }

fulled:
    Update();

}
void MyImGuiRenderer::EndDraw(buma3d::ICommandList* _list)
{
    _list->EndRenderPass({});
}

bool MyImGuiRenderer::TEXID_DESCRIPTORS::Init(RENDER_RESOURCE& _rr, MyImGuiRenderer& _parent)
{
    static const uint32_t DEFAULT_DESCRIPTOR_POOL_SIZE = 128;

    auto sampler_layout = _rr.sampler_layout.Get();
    auto texture_layout = _rr.texture_layout.Get();

    // ヒープ/プールの作成
    util::DescriptorSizes sizes;
    sizes.IncrementSizes(sampler_layout, 1) // サンプラーは現在RENDER_RESOURCE::samplerのみ使用しているため1で十分です。
         .IncrementSizes(texture_layout, 1/*font*/ + DEFAULT_DESCRIPTOR_POOL_SIZE)
         .Finalize();

    b::BMRESULT bmr;
    bmr = _rr.device->CreateDescriptorHeap(sizes.GetAsHeapDesc(b::DESCRIPTOR_HEAP_FLAG_NONE, b::B3D_DEFAULT_NODE_MASK), &descriptor_heap);
    BMR_RET_IF_FAILED(bmr);

    bmr = _rr.device->CreateDescriptorPool(sizes.GetAsPoolDesc(descriptor_heap.Get(), sizes.GetMaxSetsByTotalMultiplyCount(), b::DESCRIPTOR_POOL_FLAG_NONE), &descriptor_pool);
    BMR_RET_IF_FAILED(bmr);

    descriptor_heap->SetName("MyImGuiRenderer::descriptor_heap");
    descriptor_pool->SetName("MyImGuiRenderer::descriptor_pool");


    // 割り当て
    util::DescriptorSetAllocateDesc allocate_desc(sizes.GetMaxSetsByTotalMultiplyCount());
    allocate_desc
        .SetNumDescriptorSets(sizes.GetMaxSetsByTotalMultiplyCount())
        .SetDescriptorSetLayout(0, sampler_layout)
        .SetDescriptorSetLayout(1, texture_layout);
    for (uint32_t i = 0; i < DEFAULT_DESCRIPTOR_POOL_SIZE; i++)
        allocate_desc.SetDescriptorSetLayout(2 + i, texture_layout);
    allocate_desc.Finalize();

    auto&& [num_sets, sets] = allocate_desc.GetDst();
    bmr = descriptor_pool->AllocateDescriptorSets(allocate_desc.Get(), sets);
    BMR_RET_IF_FAILED(bmr);

    sampler_set = sets[0];
    font_set    = sets[1];
    texid_sets.resize(DEFAULT_DESCRIPTOR_POOL_SIZE);
    auto texid_sets_data = texid_sets.data();
    for (uint32_t i = 0; i < DEFAULT_DESCRIPTOR_POOL_SIZE; i++)
        texid_sets_data[i] = sets[2 + i];


    // デフォルトサンプラ、フォントテクスチャを書き込む
    b::WRITE_DESCRIPTOR_BINDING sampler_binding{ 0, 0, 1, reinterpret_cast<b::IView* const*>(_rr.sampler.GetAddressOf()) };
    b::WRITE_DESCRIPTOR_BINDING font_binding   { 0, 0, 1, reinterpret_cast<b::IView* const*>(_rr.font_srv.GetAddressOf()) };
    b::WRITE_DESCRIPTOR_SET     write_sets[2]{ { sampler_set.Get(), 1, &sampler_binding }, { font_set.Get(), 1, &font_binding } };
    bmr = _parent.descriptor_update->UpdateDescriptorSets(b::UPDATE_DESCRIPTOR_SET_DESC{ 2, write_sets });
    BMR_RET_IF_FAILED(bmr);

    return true;
}


MyImGuiViewportRenderer::MyImGuiViewportRenderer(RENDER_RESOURCE& _rr)
    : rr                    { _rr }
    , renderer              {}
    , swapchain             {}
    , present_wait_fence    {}
    , acquire_fence         {}
    , back_buffers          {}
    , framebuffers          {}
    , back_buffer_index     {}
{

}

MyImGuiViewportRenderer::~MyImGuiViewportRenderer()
{
    for (auto& i : framebuffers)
        i.Reset();

    if (swapchain)
    {
        rr.dr->DestroySwapChain(swapchain);
        swapchain = nullptr;
    }

    assert(!renderer);
}

bool MyImGuiViewportRenderer::CreateWindow(ImGuiViewport* _vp)
{
    renderer = new MyImGuiRenderer(rr, /*_is_viewport_renderer =*/true);

    // スワップチェインを作成
    {
        back_buffer_index = 0;

        // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level handle (e.g. GLFWWindow*, SDL_Window*).
        // Some backends will leave PlatformHandleRaw NULL, in which case we assume PlatformHandle will contain the HWND.
        auto hwnd = _vp->PlatformHandleRaw ? _vp->PlatformHandleRaw : _vp->PlatformHandle;

        b::SURFACE_PLATFORM_DATA_WINDOWS spd{ GetModuleHandle(NULL), hwnd };
        auto buffer_desc = b::init::SwapChainBufferDesc(
              (uint32_t)_vp->Size.x, (uint32_t)_vp->Size.y, BACK_BUFFER_COUNT
            , b::TEXTURE_FORMAT_DESC{ rr.render_pass_clear->GetDesc().attachments[0].format }
            , b::SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT | b::SWAP_CHAIN_BUFFER_FLAG_COPY_DST
        );
        swapchain = rr.dr->CreateSwapChain(b::SURFACE_DESC{ b::SURFACE_PLATFORM_DATA_TYPE_WINDOWS, &spd }, buffer_desc
                                           , b::SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT | b::SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC);
        BUMA_ASSERT(swapchain);

        back_buffers = swapchain->GetBuffers().data();

        swapchain->SetName(("MyImGuiViewportRenderer::swapchain ID: " + std::to_string(_vp->ID)).c_str());
    }

    // フェンスを作成
    {
        rr.dr->GetDevice()->CreateFence(buma3d::init::BinaryFenceDesc(), &acquire_fence);
        rr.dr->GetDevice()->CreateFence(buma3d::init::BinaryFenceDesc(), &present_wait_fence);
        acquire_fence->SetName(("MyImGuiViewportRenderer::acquire_fence ID: " + std::to_string(_vp->ID)).c_str());
        present_wait_fence->SetName(("MyImGuiViewportRenderer::present_wait_fence ID: " + std::to_string(_vp->ID)).c_str());
        swapchain->SetAcquireInfo(acquire_fence.Get(), nullptr);
        swapchain->SetPresentInfo(b::SWAP_CHAIN_PRESENT_INFO{ present_wait_fence.Get() });
    }

    auto result = CreateFramebuffers();
    RET_IF_FAILED(result);

    return true;
}
bool MyImGuiViewportRenderer::CreateFramebuffers()
{
    b::FRAMEBUFFER_DESC fd{ b::FRAMEBUFFER_FLAG_NONE, rr.render_pass_load.Get(), 1 };
    for (uint32_t i = 0; i < BACK_BUFFER_COUNT; i++)
    {
        fd.attachments = (b::IView* const*)&back_buffers[i].rtv;
        auto bmr = rr.device->CreateFramebuffer(fd, &framebuffers[i]);
        RET_IF_FAILED(util::IsSucceeded(bmr));
    }

    return true;
}


bool MyImGuiViewportRenderer::DestroyWindow(ImGuiViewport* _vp)
{
    rr.device->WaitIdle();
    for (auto& i : framebuffers)
        i.Reset();

    back_buffers = nullptr;
    present_wait_fence.Reset();
    acquire_fence.Reset();
    rr.dr->DestroySwapChain(swapchain);
    swapchain = nullptr;

    delete renderer;
    renderer = nullptr;

    return true;
}
bool MyImGuiViewportRenderer::SetWindowSize(ImGuiViewport* _vp, const ImVec2& _size)
{
    rr.device->WaitIdle();
    for (auto& i : framebuffers)
        i.Reset();

    swapchain->Resize((uint32_t)_size.x, (uint32_t)_size.y);
    back_buffers = swapchain->GetBuffers().data();
    auto result = CreateFramebuffers();
    RET_IF_FAILED(result);

    return true;
}
void MyImGuiViewportRenderer::MoveToNextFrame()
{
    uint32_t next_buffer_index = 0;
    auto bmr = swapchain->AcquireNextBuffer(0, &next_buffer_index);
    BMR_ASSERT(bmr);
    back_buffer_index = next_buffer_index;
}
bool MyImGuiViewportRenderer::RenderWindow(ImGuiViewport* _vp, void* _render_arg)
{
    MoveToNextFrame();
    if (_render_arg)
    {
        auto l = static_cast<b::ICommandList*>(_render_arg);
        renderer->BeginRecord(l);
        renderer->RecordGuiCommands(framebuffers[back_buffer_index].Get(), b::RESOURCE_STATE_UNDEFINED, b::RESOURCE_STATE_PRESENT, _vp->DrawData);
        renderer->EndRecord(l);
    }
    else
    {
        renderer->BeginRecord();
        renderer->RecordGuiCommands(framebuffers[back_buffer_index].Get(), b::RESOURCE_STATE_UNDEFINED, b::RESOURCE_STATE_PRESENT, _vp->DrawData);
        renderer->EndRecord();
    }

    return true;
}
bool MyImGuiViewportRenderer::SwapBuffers(ImGuiViewport* _vp, void* _render_arg)
{
    rr.queue->GetCommandQueue()->InsertMarker("MyImGuiViewportRenderer::SwapBuffers", nullptr);
    auto bmr = swapchain->Present();
    BMR_RET_IF_FAILED(bmr);

    return true;
}

void MyImGuiViewportRenderer::AddSubmitInfoTo(CommandQueue* _queue)
{
    // AcquireNextBuffer()のacquire_fenceのシグナルをSubmitで待機します。
    //auto&& si = _submit_info->AddNewSubmitInfo();
    //si.AddWaitFence(acquire_fence.Get()).Finalize();
    // TODO:
    _queue->AddWaitFence(1, acquire_fence.Get());
    _queue->AddSignalFence(1, present_wait_fence.Get());
    renderer->AddSubmitInfoTo(_queue);
}


}// namespace gui
}// namespace buma
