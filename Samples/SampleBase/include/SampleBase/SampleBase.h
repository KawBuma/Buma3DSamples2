#pragma once

#include <AppFramework/Application.h>

#include <DeviceResources/DeviceResources.h>

#include <Buma3DHelpers/B3DDescHelpers.h>

#include <vector>
#include <map>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace buma::util
{

/*
* Basic camera class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
class Camera
{
private:
    float fov;
    float znear, zfar;

    void updateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        glm::mat4 transM;

        rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = position;
        if (flipY)
        {
            translation.y *= -1.0f;
        }
        transM = glm::translate(glm::mat4(1.0f), translation);

        if (type == CameraType::firstperson)
        {
            matrices.view = rotM * transM;
        }
        else
        {
            matrices.view = transM * rotM;
        }

        viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
        dirty = false;
    }
public:
    enum CameraType { lookat, firstperson };
    CameraType type = CameraType::lookat;

    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();
    glm::vec4 viewPos = glm::vec4();

    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;

    bool dirty = false;
    //bool updated = false;
    bool flipY = false;

    struct
    {
        glm::mat4 perspective;
        glm::mat4 view;
    } matrices;

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keys;

    struct
    {
        glm::vec2 delta;
        bool      left   = false;
        bool      middle = false;
        bool      right  = false;
        float     wheel  = 0.f;
    } mouse;

    bool moving()
    {
        return keys.left || keys.right || keys.up || keys.down ||
               mouse.left || mouse.middle || mouse.right || mouse.wheel != 0.f;
    }

    float getNearClip()
    {
        return znear;
    }

    float getFarClip()
    {
        return zfar;
    }

    void setPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov = fov;
        this->znear = znear;
        this->zfar = zfar;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        if (flipY)
        {
            matrices.perspective[1, 1] *= -1.0f;
        }
        dirty = true;
    };

    void updateAspectRatio(float aspect)
    {
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        if (flipY)
        {
            matrices.perspective[1, 1] *= -1.0f;
        }
        dirty = true;
    }

    void setPosition(glm::vec3 position)
    {
        this->position = position;
        dirty = true;
        updateViewMatrix();
    }

    void setRotation(glm::vec3 rotation)
    {
        this->rotation = rotation;
        dirty = true;
        updateViewMatrix();
    }

    void rotate(glm::vec3 delta)
    {
        this->rotation += delta;
        dirty = true;
        updateViewMatrix();
    }

    void setTranslation(glm::vec3 translation)
    {
        this->position = translation;
        dirty = true;
        updateViewMatrix();
    };

    void translate(glm::vec3 delta)
    {
        this->position += delta;
        dirty = true;
        updateViewMatrix();
    }

    void setRotationSpeed(float rotationSpeed)
    {
        this->rotationSpeed = rotationSpeed;
    }

    void setMovementSpeed(float movementSpeed)
    {
        this->movementSpeed = movementSpeed;
    }

    bool update(float deltaTime)
    {
        if (!moving())
        {
            mouse.delta.x = 0.f;
            mouse.delta.y = 0.f;
            mouse.wheel = 0.f;
            return false;
        }

        if (type == CameraType::firstperson)
        {
            glm::vec3 camFront;
            camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
            camFront.y = sin(glm::radians(rotation.x));
            camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            camFront = glm::normalize(camFront);

            float moveSpeed = deltaTime * movementSpeed;

            if (keys.up)
                position += camFront * moveSpeed;
            if (keys.down)
                position -= camFront * moveSpeed;
            if (keys.left)
                position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            if (keys.right)
                position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            updateViewMatrix();
        }
        else
        {
            if (mouse.left)
                rotate(glm::vec3(mouse.delta.y * rotationSpeed, mouse.delta.x * rotationSpeed, 0.0f));
            if (mouse.right)
                translate(glm::vec3(-0.0f, 0.0f, mouse.delta.y * -.005f));
            if (mouse.middle)
                translate(glm::vec3(mouse.delta.x * 0.005f, -mouse.delta.y * 0.005f, 0.0f));
            if (mouse.wheel != 0.f)
                translate(glm::vec3(0.0f, 0.0f, mouse.wheel * 0.05f));
            updateViewMatrix();
        }
        mouse.delta.x = 0.f;
        mouse.delta.y = 0.f;
        mouse.wheel = 0.f;
        return true;
    }

    // Update camera passing separate axis data (gamepad)
    // Returns true if view or position has been changed
    bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
    {
        bool retVal = false;

        if (type == CameraType::firstperson)
        {
            // Use the common console thumbstick layout		
            // Left = view, right = move

            const float deadZone = 0.0015f;
            const float range = 1.0f - deadZone;

            glm::vec3 camFront;
            camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
            camFront.y = sin(glm::radians(rotation.x));
            camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            camFront = glm::normalize(camFront);

            float moveSpeed = deltaTime * movementSpeed * 2.0f;
            float rotSpeed = deltaTime * rotationSpeed * 50.0f;

            // Move
            if (fabsf(axisLeft.y) > deadZone)
            {
                float pos = (fabsf(axisLeft.y) - deadZone) / range;
                position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
                retVal = true;
            }
            if (fabsf(axisLeft.x) > deadZone)
            {
                float pos = (fabsf(axisLeft.x) - deadZone) / range;
                position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
                retVal = true;
            }

            // Rotate
            if (fabsf(axisRight.x) > deadZone)
            {
                float pos = (fabsf(axisRight.x) - deadZone) / range;
                rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
                retVal = true;
            }
            if (fabsf(axisRight.y) > deadZone)
            {
                float pos = (fabsf(axisRight.y) - deadZone) / range;
                rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
                retVal = true;
            }
        }
        else
        {
        }

        if (retVal)
        {
            updateViewMatrix();
        }

        return retVal;
    }

};


} // namespace buma::util

namespace buma
{

class SampleBase : public ApplicationBase
{
public:
    SampleBase(PlatformBase& _platform);
    virtual ~SampleBase();

protected:
    virtual bool CreateWindow(uint32_t _w, uint32_t _h, const char* _title = "SampleBase");
    virtual bool CreateDeviceResources(const char* _library_dir = nullptr);
    virtual bool CreateSwapChain(const buma3d::SWAP_CHAIN_BUFFER_DESC& _buffer_desc, buma3d::SWAP_CHAIN_FLAGS _flags);

    void CreateNullDescriptorSetLayout();
    void CreateDefaultSamplers();
    void CreateFencesForSwapChain();
    void CreateDescriptorUpdate();

    virtual bool CreateDescriptorSetLayouts() = 0;
    virtual bool CreateDescriptorHeapAndPool();
    virtual bool AllocateDescriptorSets();
    virtual bool CreateRenderPass();
    virtual bool CreateFramebuffers();
    virtual bool CreatePipelineLayout();
    virtual bool CreatePipeline() = 0;
    virtual bool CreateCommandAllocator();
    virtual bool CreateCommandLists();

    buma3d::IShaderModule* CreateShaderModule(const char* _path, buma3d::SHADER_STAGE_FLAG _stage, const char* _entry_point = "main");

    void DestroySampleBaseObjects();

protected:
    WindowBase*                                                     window;

    std::unique_ptr<DeviceResources>                                dr;
    buma3d::util::Ptr<buma3d::IDeviceAdapter>                       adapter;
    buma3d::util::Ptr<buma3d::IDevice>                              device;

    SwapChain*                                                      swapchain;
    CommandQueue*                                                   present_queue;
    uint32_t                                                        buffer_count;
    buma3d::RESOURCE_FORMAT                                         buffer_format;
    uint32_t                                                        buffer_index;

    uint64_t                                                        current_fence_value;
    std::vector<uint64_t>                                           submission_fence_values;
    buma3d::util::Ptr<buma3d::IFence>                               submission_fence; // コマンド完了のシグナル、待機を意図したフェンスです。

    buma3d::util::Ptr<buma3d::IFence>                               acquire_fence;
    buma3d::util::Ptr<buma3d::IFence>                               acquire_fence_to_cpu;
    buma3d::util::Ptr<buma3d::IFence>                               present_fence;           // スワップチェインのプレセントを待機させる事を意図したフェンスです(Submitでシグナルし、Presentで待機します)

    buma3d::util::Ptr<buma3d::IDescriptorSetLayout>                 null_layout;
    buma3d::util::Ptr<buma3d::ISamplerView>                         sampler_point;
    buma3d::util::Ptr<buma3d::ISamplerView>                         sampler_linear;
    buma3d::util::Ptr<buma3d::ISamplerView>                         sampler_aniso;

    buma3d::util::Ptr<buma3d::IRenderPass>                          render_pass;
    std::vector<buma3d::util::Ptr<buma3d::IFramebuffer>>            framebuffers;

    std::vector<buma3d::util::Ptr<buma3d::IDescriptorSetLayout>>    set_layouts;
    buma3d::util::Ptr<buma3d::IPipelineLayout>                      pipeline_layout;

    buma3d::util::Ptr<buma3d::IDescriptorHeap>                      descriptor_heap;
    buma3d::util::Ptr<buma3d::IDescriptorPool>                      descriptor_pool;
    std::vector<buma3d::util::Ptr<buma3d::IDescriptorSet>>          descriptor_sets;

    std::map<std::string, buma3d::util::Ptr<buma3d::IShaderModule>> shader_modules;
    buma3d::util::Ptr<buma3d::IPipelineState>                       pipeline;

    std::vector<buma3d::util::Ptr<buma3d::ICommandAllocator>>       command_allocators;
    std::vector<buma3d::util::Ptr<buma3d::ICommandList>>            command_lists;

    buma3d::util::Ptr<buma3d::IDescriptorUpdate>                    descriptor_update;

};


} // namespace buma
