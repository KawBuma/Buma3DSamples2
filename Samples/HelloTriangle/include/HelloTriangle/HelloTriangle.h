#pragma once

#include <AppFramework/Framework.h>

#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>

#include <vector>
#include <memory>

namespace buma
{

class DeviceResources;

class HelloTriangle : public ApplicationBase
{
public:
    static constexpr uint32_t BUFFER_COUNT = 3;

public:
    HelloTriangle(PlatformBase& _platform);
    virtual ~HelloTriangle();

    static HelloTriangle* Create(PlatformBase& _platform);
    static void Destroy(HelloTriangle* _app);

public:
    void OnKeyUp(KeyUpEventArgs& _args) override;
    void OnProcessWindow(ProcessWindowEventArgs& _args) override;
    bool OnInit() override;

    void Tick(const util::StepTimer& _timer) override;
    void OnDestroy() override;

    const char* GetName() const { return "HelloTriangle"; }

private:
    void CreateWindow(uint32_t _w, uint32_t _h, const char* _title);
    bool CreateDeviceResources();
    bool InitSwapChain();
    bool CreateRenderTargetViews();
    bool CreateFramebuffers();

    bool LoadAssets();
    bool CreatePipelineLayout();
    bool CreateRenderPass();
    bool CreateShaderModules();
    buma3d::IShaderModule* CreateShaderModule(const char* _path, buma3d::SHADER_STAGE_FLAG _stage, const char* _entry_point = "main");
    bool CreateGraphicsPipelines();
    bool CreateCommandAllocator();
    bool CreateCommandLists();
    bool CreateFences();
    bool CreateBuffers();
    bool CreateHeaps(buma3d::RESOURCE_HEAP_ALLOCATION_INFO* _heap_alloc_info, std::vector<buma3d::RESOURCE_ALLOCATION_INFO>* _alloc_infos);
    bool BindResourceHeaps(buma3d::RESOURCE_HEAP_ALLOCATION_INFO* _heap_alloc_info, std::vector<buma3d::RESOURCE_ALLOCATION_INFO>* _alloc_infos);
    bool CreateBuffersForCopy();
    bool CopyBuffers();
    bool CreateBufferViews();

    void Update(float _deltatime);
    void Render(float _deltatime);
    void MoveToNextFrame();
    void OnResize(uint32_t _w, uint32_t _h);
    void PrepareFrame(uint32_t _buffer_index, float _deltatime);

private:
    struct VERTEX {
        buma3d::FLOAT4 position;
        buma3d::FLOAT4 color;
    };
    std::vector<VERTEX> triangle;
    std::vector<uint16_t> index;

private:
    WindowBase*                                                 window;
    std::unique_ptr<DeviceResources>                            dr;

    buma3d::util::Ptr<buma3d::IDeviceAdapter>                   adapter;
    buma3d::util::Ptr<buma3d::IDevice>                          device;
    buma3d::util::Ptr<buma3d::ICommandQueue>                    command_queue;

    buma3d::util::Ptr<buma3d::ISurface>                         surface;
    buma3d::util::Ptr<buma3d::ISwapChain>                       swapchain;
    std::vector<buma3d::util::Ptr<buma3d::ITexture>>            back_buffers;
    std::vector<buma3d::util::Ptr<buma3d::IRenderTargetView>>   back_buffer_rtvs;
    buma3d::RESOURCE_FORMAT                                     back_buffer_format;
    uint32_t                                                    back_buffer_index;

    std::vector<buma3d::util::Ptr<buma3d::IFence>>              cmd_fences;
    buma3d::util::Ptr<buma3d::IFence>                           acquire_fence;
    buma3d::util::Ptr<buma3d::IFence>                           render_complete_fence;

    buma3d::util::Ptr<buma3d::IRenderPass>                      render_pass;
    buma3d::util::Ptr<buma3d::IPipelineLayout>                  pipeline_layout;
    std::vector<buma3d::util::Ptr<buma3d::IFramebuffer>>        framebuffers;

    std::vector<buma3d::util::Ptr<buma3d::IShaderModule>>       shader_modules;
    buma3d::util::Ptr<buma3d::IPipelineState>                   pipeline;
    std::vector<buma3d::util::Ptr<buma3d::ICommandAllocator>>   cmd_allocator;
    std::vector<buma3d::util::Ptr<buma3d::ICommandList>>        cmd_lists;

    std::vector<buma3d::RESOURCE_HEAP_PROPERTIES>               heap_props;
    buma3d::util::Ptr<buma3d::IResourceHeap>                    resource_heap;
    buma3d::util::Ptr<buma3d::IBuffer>                          vertex_buffer;
    buma3d::util::Ptr<buma3d::IBuffer>                          index_buffer;
    buma3d::util::Ptr<buma3d::IBuffer>                          vertex_buffer_src;
    buma3d::util::Ptr<buma3d::IBuffer>                          index_buffer_src;
    buma3d::util::Ptr<buma3d::IVertexBufferView>                vertex_buffer_view;
    buma3d::util::Ptr<buma3d::IIndexBufferView>                 index_buffer_view;

private:
    bool is_reuse_commands;

};

BUMA_APPMODULE_API ApplicationBase* BUMA_APP_APIENTRY CreateApplication(PlatformBase& _platform);
BUMA_APPMODULE_API void BUMA_APP_APIENTRY DestroyApplication(ApplicationBase* _app);

}// namespace buma
