#pragma once
#include<SampleBase/SampleBase.h>

#include<DeviceResources/ResourceBuffer.h>

namespace buma
{

class DeviceResources;

class HelloConstantBuffer : public SampleBase
{
public:
    HelloConstantBuffer(PlatformBase& _platform);
    virtual ~HelloConstantBuffer();

    static HelloConstantBuffer* Create(PlatformBase& _platform);
    static void Destroy(HelloConstantBuffer* _app);

public:
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

    const char* GetName() const override { return "HelloConstantBuffer"; }

private:
    bool LoadAssets();
    bool CreateDescriptorSetLayouts() override;
    bool CreatePipelineLayout() override;
    bool CreateDescriptorHeap();
    bool CreateDescriptorPool();
    bool AllocateDescriptorSets() override;
    bool CreatePipeline() override;
    bool CreateShaderModules();
    bool CreateBuffers();
    bool CreateBufferViews();
    bool CreateConstantBuffer();
    bool CreateConstantBufferView();
    bool UpdateDescriptorSet();

private:
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

private:
    std::vector<VERTEX>                                         triangle;
    std::vector<uint16_t>                                       index;
    CB_MODEL                                                    cb_model;
    CB_SCENE                                                    cb_scene;

    std::vector<FRAME_CB>                                       frame_cbs;

    Buffer*                                                     vertex_buffer;
    Buffer*                                                     index_buffer;
    buma3d::util::Ptr<buma3d::IVertexBufferView>                vertex_buffer_view;
    buma3d::util::Ptr<buma3d::IIndexBufferView>                 index_buffer_view;

private:
    bool is_reuse_commands;

};

BUMA_APPMODULE_API ApplicationBase* BUMA_APP_APIENTRY CreateApplication(PlatformBase& _platform);
BUMA_APPMODULE_API void BUMA_APP_APIENTRY DestroyApplication(ApplicationBase* _app);

}// namespace buma
