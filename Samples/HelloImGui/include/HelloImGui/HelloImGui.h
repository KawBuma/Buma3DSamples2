#pragma once
#include <SampleBase/SampleBase.h>

namespace buma
{

namespace gui { class MyImGui; }
namespace tex { struct ITextures; }

class DeviceResources;
class Texture;
class Buffer;

class HelloImGui : public SampleBase
{
public:
    HelloImGui(PlatformBase& _platform);
    virtual ~HelloImGui();

    static HelloImGui* Create(PlatformBase& _platform);
    static void Destroy(HelloImGui* _app);

public:
    void OnProcessMessage (ProcessMessageEventArgs&  _args) override;
    void OnProcessWindow  (ProcessWindowEventArgs&   _args) override;
    void OnKeyDown        (KeyDownEventArgs&         _args) override;
    void OnKeyUp          (KeyUpEventArgs&           _args) override;
    void OnMouseMove      (MouseMoveEventArgs&       _args) override;
    void OnMouseButtonDown(MouseButtonDownEventArgs& _args) override;
    void OnMouseButtonUp  (MouseButtonUpEventArgs&   _args) override;
    void OnMouseWheel     (MouseWheelEventArgs&      _args) override;

    bool OnInit() override;
    void Tick(const util::StepTimer& _timer) override;
    void OnDestroy() override;

    const char* GetName() const override { return "HelloImGui"; }

private:
    bool LoadAssets();
    bool LoadTextureData();
    bool CreateDescriptorSetLayouts() override;
    bool CreateBorderSampler();
    bool CreatePipelineLayout() override;
    bool CreateDescriptorHeapAndPool() override;
    bool AllocateDescriptorSets() override;
    bool CreateShaderModules();
    bool CreatePipeline() override;
    bool CreateBuffers();
    bool CreateBufferViews();
    bool CreateConstantBuffer();
    bool CreateConstantBufferView();
    bool CreateTextureResource();
    bool CopyDataToTexture();
    bool CreateShaderResourceView();
    bool UpdateDescriptorSet();
    bool InitMyImGui();

private:
    void Update(float _deltatime);
    void Render(float _deltatime);
    void MoveToNextFrame();
    void OnResize(uint32_t _w, uint32_t _h);
    void PrepareFrame(uint32_t _buffer_index, float _deltatime);

private:
    struct VERTEX {
        buma3d::FLOAT4 position;
        buma3d::FLOAT2 uv;
    };
    struct CB_MODEL { // register(b0, space0);
        glm::mat4 model;
    };
    struct CB_SCENE { // register(b0, space1);
        glm::mat4 view_proj;
    };
    struct FRAME_CB {
        void*                                           mapped_data[2/*model,scene*/];
        Buffer*                                         buffer;
        buma3d::util::Ptr<buma3d::IConstantBufferView>  scene_cbv;
        buma3d::util::Ptr<buma3d::IConstantBufferView>  model_cbv;
    };
    struct TEXTURE {
        std::unique_ptr<tex::ITextures>                 data;
        Texture*                                        texture;
        buma3d::util::Ptr<buma3d::IShaderResourceView>  srv;
    };
    enum SET_LAYOUT { BUF = 0, TEX = 1 };

private:
    std::vector<VERTEX>                                         quad;
    CB_MODEL                                                    cb_model;
    CB_SCENE                                                    cb_scene;

    std::vector<FRAME_CB>                                       frame_cbs;

    Buffer*                                                     vertex_buffer;
    buma3d::util::Ptr<buma3d::IVertexBufferView>                vertex_buffer_view;

    TEXTURE                                                     texture;

    buma3d::util::Ptr<buma3d::IDescriptorPool>                  copyable_descriptor_pool;
    std::vector<buma3d::util::Ptr<buma3d::IDescriptorSet>>      buffer_descriptor_sets;
    buma3d::util::Ptr<buma3d::IDescriptorSet>                   texture_descriptor_set;

    buma3d::util::Ptr<buma3d::ISamplerView>                     border_sampler;

private:
    bool show_gui;
    std::unique_ptr<gui::MyImGui> myimgui;

private:
    bool is_reuse_commands;

};

BUMA_APPMODULE_API ApplicationBase* BUMA_APP_APIENTRY CreateApplication(PlatformBase& _platform);
BUMA_APPMODULE_API void BUMA_APP_APIENTRY DestroyApplication(ApplicationBase* _app);

}// namespace buma
