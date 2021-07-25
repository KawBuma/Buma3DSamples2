#pragma once
#include "./Utils.h"

#include <Buma3D/Buma3D.h>

#include <Utils/Utils.h>

#include <Buma3DHelpers/B3DDescHelpers.h>

#include <DeviceResources/ResourceBuffer.h>
#include <DeviceResources/SwapChain.h>

#include <imgui.h>


namespace buma
{

class CommandQueue;

namespace gui
{


class MyImGuiViewportRenderer;
class MyImGuiRenderer
{
    friend class MyImGuiViewportRenderer;
public:
    MyImGuiRenderer(RENDER_RESOURCE& _rr, bool _is_viewport_renderer = false, bool _is_primary_renderer = false);
    ~MyImGuiRenderer();

    void BeginRecord(buma3d::ICommandList* _list = nullptr);
    void Draw       (buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data);
    void EndRecord  (buma3d::ICommandList* _list = nullptr);

    void RecordGuiCommands(buma3d::IFramebuffer* _framebuffer, buma3d::RESOURCE_STATE _current_state, buma3d::RESOURCE_STATE _state_after, ImDrawData* _draw_data);
    void AddSubmitInfoTo(CommandQueue* _queue);

    buma3d::ICommandList* GetCommandList() const { return list.Get(); }

private:
    void Init();

private:
    void ClearViewportRendererRtv(buma3d::ICommandList* _list, ImDrawData* _draw_data);

private:
    void PrepareDraw            (buma3d::ICommandList* _list, buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data);
    void PrepareBuffers         (ImDrawData* _draw_data);
    void PrepareIndexBuffer     (ImDrawData* _draw_data);
    void PrepareVertexBuffer    (ImDrawData* _draw_data);
    void BeginDraw              (buma3d::ICommandList* _list, buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data);
    void FlushDraw              (buma3d::ICommandList* _list, buma3d::IFramebuffer* _framebuffer, ImDrawData* _draw_data);
    void Submit                 (buma3d::ICommandList* _list);
    void UpdateDescriptorSets   (ImDrawData* _draw_data);
    void EndDraw                (buma3d::ICommandList* _list);

private:
    bool IsScissorValid(const ImVec2& _disp_pos, const ImVec4& _clip_rect) const
    {
        return  (_clip_rect.x - _disp_pos.x) >= 0 && (_clip_rect.y - _disp_pos.y) >= 0 &&
                (_clip_rect.z - _disp_pos.x) > (_clip_rect.x - _disp_pos.x) &&
                (_clip_rect.w - _disp_pos.y) > (_clip_rect.y - _disp_pos.y);
    }
    void ChangeTexIdDescriptors()
    {
        current_texid_descriptors = texid_descriptors.data()[texid_descriptors_offset++].get();
    }
    void AddNewTexIdDescriptors()
    {
        if (texid_descriptors_offset + 1 > texid_descriptors.size())
        {
            texid_descriptors.emplace_back(std::make_unique<TEXID_DESCRIPTORS>())
                ->Init(rr, *this);
        }
        ChangeTexIdDescriptors();
    }

private:
    struct B3D_CMD_ARGS
    {
        buma3d::COMMAND_LIST_BEGIN_DESC         list_begin_desc     { buma3d::COMMAND_LIST_BEGIN_FLAG_ONE_TIME_SUBMIT, };
        buma3d::CMD_PUSH_32BIT_CONSTANTS        push_constants      {};
        float                                   mvp[4][4]           {};
        buma3d::CMD_BIND_VERTEX_BUFFER_VIEWS    bind_vbv            {};
        buma3d::VIEWPORT                        viewport            {};
        buma3d::SCISSOR_RECT                    scissor_rect        {};
        buma3d::SUBPASS_BEGIN_DESC              subpass_begin       { buma3d::SUBPASS_CONTENTS_INLINE };
        buma3d::RENDER_PASS_BEGIN_DESC          render_pass_begin   {};
        buma3d::DRAW_INDEXED_ARGUMENTS          draw_indexed        {};
        buma3d::CLEAR_ATTACHMENT                ca                  {};
        buma3d::CMD_CLEAR_ATTACHMENTS           cas                 {};
        util::PipelineBarrierDesc               barrier_desc        {};
    };
    struct IMGUI_DRAW_PROGRESS
    {
        int                                     imcmd_list_offset;
        int                                     imcmd_buffer_offset;
        uint32_t                                start_index_location;
        uint32_t                                texid_set_offset;
        bool                                    has_bound_font_set;
        bool                                    has_done;
    };

    struct SUBMIT
    {
        buma3d::SUBMIT_INFO info;
        buma3d::SUBMIT_DESC desc;
    };

    struct TEXID_DESCRIPTORS
    {
        bool Init(RENDER_RESOURCE& _rr, MyImGuiRenderer& _parent);

        buma3d::util::Ptr<buma3d::IDescriptorHeap>              descriptor_heap;
        buma3d::util::Ptr<buma3d::IDescriptorPool>              descriptor_pool;
        buma3d::util::Ptr<buma3d::IDescriptorSet>               sampler_set;
        buma3d::util::Ptr<buma3d::IDescriptorSet>               font_set;
        std::vector<buma3d::util::Ptr<buma3d::IDescriptorSet>>  texid_sets;
    };

private:
    RENDER_RESOURCE&                                        rr;
    bool                                                    is_viewport_renderer;
    bool                                                    is_primary_renderer;

    uint32_t                                                total_vtx_count;
    uint32_t                                                total_idx_count;
    Buffer*                                                 vertex_buffer;
    Buffer*                                                 index_buffer;
    buma3d::util::Ptr<buma3d::IVertexBufferView>            vertex_buffer_view;
    buma3d::util::Ptr<buma3d::IIndexBufferView>             index_buffer_view;

    std::unique_ptr<SUBMIT>                                 submit;

    buma3d::util::Ptr<buma3d::IDescriptorUpdate>            descriptor_update;
    util::UpdateDescriptorSetDesc                           update_desc;
    std::vector<std::unique_ptr<TEXID_DESCRIPTORS>>         texid_descriptors;
    size_t                                                  texid_descriptors_offset;
    TEXID_DESCRIPTORS*                                      current_texid_descriptors;

    buma3d::util::Ptr<buma3d::IFence>                       timeline_fence;
    util::FENCE_VALUES                                      timeline_fence_val;
    buma3d::util::Ptr<buma3d::ICommandAllocator>            allocator;
    buma3d::util::Ptr<buma3d::ICommandList>                 list;
    buma3d::ICommandList*                                   current_list;

    B3D_CMD_ARGS                                            args;
    IMGUI_DRAW_PROGRESS                                     progress;

};

class MyImGuiViewportRenderer
{
public:
    static constexpr uint32_t BACK_BUFFER_COUNT = 3;

public:
    MyImGuiViewportRenderer(RENDER_RESOURCE& _rr);
    ~MyImGuiViewportRenderer();

    bool CreateWindow(ImGuiViewport* _vp);
    bool CreateFramebuffers();
    bool DestroyWindow(ImGuiViewport* _vp);
    bool SetWindowSize(ImGuiViewport* _vp, const ImVec2& _size);
    void MoveToNextFrame();
    bool RenderWindow(ImGuiViewport* _vp, void* _render_arg);
    bool SwapBuffers(ImGuiViewport* _vp, void* _render_arg);

    void AddSubmitInfoTo(CommandQueue* _queue);

private:
    RENDER_RESOURCE&                            rr;
    MyImGuiRenderer*                            renderer;
    SwapChain*                                  swapchain;
    buma3d::util::Ptr<buma3d::IFence>           present_wait_fence; // バッファのSubmitでの使用完了までPresentで待機させるフェンス
    buma3d::util::Ptr<buma3d::IFence>           acquire_fence;      // バッファのacquireが通知されるフェンス
    const SwapChain::SWAP_CHAIN_BUFFER*         back_buffers;
    buma3d::util::Ptr<buma3d::IFramebuffer>     framebuffers[BACK_BUFFER_COUNT];
    uint32_t                                    back_buffer_index;

};

}// namespace gui
}// namespace buma
