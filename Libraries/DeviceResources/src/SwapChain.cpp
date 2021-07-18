#include <DeviceResources/SwapChain.h>
#include <DeviceResources/ResourceTexture.h>
#include <DeviceResources/CommandQueue.h>

#include <Buma3DHelpers/B3DInit.h>
#include <Buma3DHelpers/B3DDescHelpers.h>
#include <Buma3DHelpers/Buma3DHelpers.h>

namespace buma
{

SwapChain::SwapChain(DeviceResources&                          _dr
                     , const buma3d::SWAP_CHAIN_BUFFER_DESC&   _buffer
                     , buma3d::SWAP_CHAIN_FLAGS                _flags
                     , buma3d::ISurface*                       _surface
                     , CommandQueue*                           _present_queue)
    : dr                      { _dr }
    , swapchain_desc          {}
    , present_queue           { _present_queue }
    , b3d_present_queue       { _present_queue->GetCommandQueue() }
    , surface                 { _surface }
    , swapchain               {}
    , back_buffer_index       {}
    , acquire_info            {}
    , present_info            {}
    , present_regions         {}
    , back_buffers            {}
{
    swapchain_desc.surface            = surface.Get();
    swapchain_desc.color_space        = buma3d::COLOR_SPACE_SRGB_NONLINEAR; // TODO: swapchain_desc.color_space の指定
    swapchain_desc.pre_roration       = buma3d::ROTATION_MODE_IDENTITY;
    swapchain_desc.buffer             = _buffer;
    swapchain_desc.alpha_mode         = buma3d::SWAP_CHAIN_ALPHA_MODE_DEFAULT;
    swapchain_desc.flags              = _flags;
    swapchain_desc.num_present_queues = 1;
    swapchain_desc.present_queues     = &b3d_present_queue;
    Create();
}


SwapChain::~SwapChain()
{
    dr.GetDevice()->WaitIdle();

    DestroyBackBuffers();
    swapchain.Reset();
    surface.Reset();
}

buma3d::BMRESULT SwapChain::AcquireNextBuffer(uint32_t _timeout_millisec, uint32_t* _dst_back_buffer_index)
{
    uint32_t next_buffer_index = 0;
    auto bmr = swapchain->AcquireNextBuffer(acquire_info, &next_buffer_index);

    back_buffer_index = next_buffer_index;
    *_dst_back_buffer_index = next_buffer_index;
    return bmr;
}

buma3d::BMRESULT SwapChain::Present()
{
    return swapchain->Present(present_info);
}

void SwapChain::Resize(uint32_t _width, uint32_t _height)
{
    auto tmp = swapchain_desc.buffer;
    tmp.width  = _width;
    tmp.height = _height;
    Recreate(tmp, swapchain_desc.flags);
}

void SwapChain::Recreate(const buma3d::SWAP_CHAIN_BUFFER_DESC& _buffer, buma3d::SWAP_CHAIN_FLAGS _flags)
{
    swapchain_desc.buffer = _buffer;
    swapchain_desc.flags = _flags;

    DestroyBackBuffers();
    back_buffer_index = 0;

    auto bmr = swapchain->Recreate(swapchain_desc);
    BMR_ASSERT(bmr);

    CreateBackBuffers();
    CreateViews();
}

bool SwapChain::SetName(const char* _name)
{
    swapchain->SetName(_name);
    for (uint32_t i = 0, size = (uint32_t)back_buffers.size(); i < size; i++)
    {
        back_buffers[i].tex->SetName((std::string(_name) + " buffer " + std::to_string(i)).c_str());
    }

    return true;
}

void SwapChain::SetAcquireInfo(buma3d::IFence* _signal_fence, buma3d::IFence* _signal_fence_to_cpu)
{
    acquire_info.signal_fence        = _signal_fence;
    acquire_info.signal_fence_to_cpu = _signal_fence_to_cpu;
}

void SwapChain::SetPresentInfo(const buma3d::SWAP_CHAIN_PRESENT_INFO& _present_info)
{
    present_info.wait_fence          = _present_info.wait_fence;
    present_info.num_present_regions = _present_info.num_present_regions;
    present_info.present_regions     = nullptr;
    if (_present_info.num_present_regions != 0)
    {
        present_regions.resize(_present_info.num_present_regions);
        present_info.present_regions = present_regions.data();
        size_t cnt = 0;
        for (auto& i : present_regions)
            i = _present_info.present_regions[cnt++];
    }
}

void SwapChain::SetPresentInfo(buma3d::IFence* _wait_fence, std::vector<buma3d::SCISSOR_RECT>&& _present_regions)
{
    present_regions                  = std::move(_present_regions);
    present_info.wait_fence          = _wait_fence;
    present_info.num_present_regions = (uint32_t)present_regions.size();
    present_info.present_regions     = present_regions.data();
}

void SwapChain::Create()
{
    // 作成時に指定されたフォーマットが見つからなければデフォルトのフォーマットを設定
    std::vector<buma3d::SURFACE_FORMAT> supported_formats;
    supported_formats.resize(surface->GetSupportedSurfaceFormats(nullptr));
    surface->GetSupportedSurfaceFormats(supported_formats.data());

    auto it_find = std::find_if(supported_formats.begin(), supported_formats.end(), [&](const buma3d::SURFACE_FORMAT& _f) {
        return (_f.format == swapchain_desc.buffer.format_desc.format && _f.color_space == swapchain_desc.color_space);
    });
    if (it_find == supported_formats.end())
    {
        swapchain_desc.buffer.format_desc.format = supported_formats.front().format;
        swapchain_desc.color_space               = supported_formats.front().color_space;
    }

    auto bmr = dr.GetDevice()->CreateSwapChain(swapchain_desc, &swapchain);
    BMR_ASSERT(bmr);
    swapchain->SetName("SwapChain");

    CreateBackBuffers();
    CreateViews();
}

void SwapChain::CreateBackBuffers()
{
    BUMA_ASSERT(back_buffers.empty());
    back_buffers.resize(swapchain_desc.buffer.count);
    for (uint32_t i = 0; i < swapchain_desc.buffer.count; i++)
    {
        buma3d::util::Ptr<buma3d::ITexture> tex;
        auto bmr = swapchain->GetBuffer(i, &tex);
        BMR_ASSERT(bmr);

        auto name = swapchain->GetName();
        auto&& bb = back_buffers[i];
        bb.tex = new Texture(dr, tex.Get());
        bb.tex->SetName((name ? name : "Swapchain" + std::to_string(i)).c_str());
    }
}

void SwapChain::DestroyBackBuffers()
{
    for (auto& i : back_buffers)
    {
        i.rtv = nullptr;
        i.srv = nullptr;
        util::SafeDelete(i.tex);
    }
    back_buffers.clear();
}

void SwapChain::CreateViews()
{
    auto&& device = dr.GetDevice();
    for (auto& i : back_buffers)
    {
        auto&& tdesc = i.tex->GetDesc();
        if (tdesc.texture.usage & buma3d::TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT)
        {
            buma3d::RENDER_TARGET_VIEW_DESC rtvdesc = buma3d::init::RenderTargetViewDescTex2D(tdesc.texture.format_desc.format);
            i.rtv = i.tex->GetRTV(rtvdesc);
        }
        if (tdesc.texture.usage & buma3d::TEXTURE_USAGE_FLAG_SHADER_RESOURCE)
        {
            buma3d::SHADER_RESOURCE_VIEW_DESC srvdesc = buma3d::init::ShaderResourceViewDescDescTex2D(tdesc.texture.format_desc.format);
            if (!(tdesc.texture.usage & buma3d::TEXTURE_USAGE_FLAG_INPUT_ATTACHMENT))
                srvdesc.flags |= buma3d::SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT;
            i.srv = i.tex->GetSRV(srvdesc);
        }
    }
}


}// namespace buma
