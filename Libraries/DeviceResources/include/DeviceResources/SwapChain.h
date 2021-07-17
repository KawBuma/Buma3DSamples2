#pragma once

#include <DeviceResources/DeviceResources.h>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <memory>

namespace buma
{

class Texture;
class CommandQueue;

class SwapChain
{
public:
    struct SWAP_CHAIN_BUFFER
    {
        Texture*                        tex;
        buma3d::IRenderTargetView*      rtv;
        buma3d::IShaderResourceView*    srv;
    };

public:
    SwapChain(DeviceResources&                          _dr
              , const buma3d::SWAP_CHAIN_BUFFER_DESC&   _buffer
              , buma3d::SWAP_CHAIN_FLAGS                _flags
              , buma3d::ISurface*                       _surface
              , CommandQueue*                           _present_queue);
    ~SwapChain();

    bool                SetName(const char* _name);

    void                SetAcquireInfo(buma3d::IFence* _signal_fence, buma3d::IFence* _signal_fence_to_cpu);
    void                SetPresentInfo(const buma3d::SWAP_CHAIN_PRESENT_INFO& _present_info);
    void                SetPresentInfo(buma3d::IFence* _wait_fence, std::vector<buma3d::SCISSOR_RECT>&& _present_regions);

    buma3d::BMRESULT    AcquireNextBuffer(uint32_t _timeout_millisec, uint32_t* _dst_back_buffer_index);
    buma3d::BMRESULT    Present();

    void                Resize(uint32_t _width, uint32_t _height);
    void                Recreate(const buma3d::SWAP_CHAIN_BUFFER_DESC& _buffer, buma3d::SWAP_CHAIN_FLAGS _flags);

    const buma3d::SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO&  GetAcquireInfo()            const { return acquire_info; }
    const buma3d::SWAP_CHAIN_PRESENT_INFO&              GetPresentInfo()            const { return present_info; }
    const buma3d::SWAP_CHAIN_BUFFER_DESC&               GetBufferDesc()             const { return swapchain_desc.buffer; }
    buma3d::SWAP_CHAIN_FLAGS                            GetFlags()                  const { return swapchain_desc.flags; }

    CommandQueue*                                       GetPresentableQueue()       const { return present_queue; }
    const buma3d::util::Ptr<buma3d::ISurface>&          GetSurface()                const { return surface; }
    const buma3d::util::Ptr<buma3d::ISwapChain>&        GetSwapChain()              const { return swapchain; }
    const buma3d::SWAP_CHAIN_DESC&                      GetSwapChainDesc()          const { return swapchain_desc; }
    const std::vector<SWAP_CHAIN_BUFFER>&               GetBuffers()                const { return back_buffers; }
    const SWAP_CHAIN_BUFFER&                            GetCurrentBuffer()          const { return back_buffers[back_buffer_index]; }
    uint32_t                                            GetCurrentBufferIndex()     const { return back_buffer_index; }

protected:
    void Create();
    void CreateBackBuffers();
    void DestroyBackBuffers();
    void CreateViews();

protected:
    DeviceResources&                            dr;
    CommandQueue*                               present_queue;
    buma3d::ICommandQueue*                      b3d_present_queue;
    buma3d::SWAP_CHAIN_DESC                     swapchain_desc;
    buma3d::util::Ptr<buma3d::ISurface>         surface;
    buma3d::util::Ptr<buma3d::ISwapChain>       swapchain;
    uint32_t                                    back_buffer_index;

    buma3d::SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO acquire_info;
    buma3d::SWAP_CHAIN_PRESENT_INFO             present_info;
    std::vector<buma3d::SCISSOR_RECT>           present_regions;

    std::vector<SWAP_CHAIN_BUFFER>              back_buffers;

};

        
}// namespace buma
