#pragma once

#include <DeviceResources/Resource.h>

namespace buma
{

class Texture : public ResourceBase
{
public:
    Texture(DeviceResources& _dr, RESOURCE_CREATE_TYPE _create_type
            , const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags);

    // SwapChain クラス用テクスチャを構築します
    Texture(DeviceResources& _dr, buma3d::ITexture* _swapchain_texture);

    ~Texture();

    buma3d::util::Ptr<buma3d::ITexture> GetB3DTexture() const { return GetB3DResource().As<buma3d::ITexture>(); }

    buma3d::IShaderResourceView*    GetSRV(const buma3d::SHADER_RESOURCE_VIEW_DESC& _desc);
    buma3d::IUnorderedAccessView*   GetUAV(const buma3d::UNORDERED_ACCESS_VIEW_DESC& _desc);
    buma3d::IRenderTargetView*      GetRTV(const buma3d::RENDER_TARGET_VIEW_DESC& _desc);
    buma3d::IDepthStencilView*      GetDSV(const buma3d::DEPTH_STENCIL_VIEW_DESC& _desc);

public:
    ShaderResourceViewCache<buma3d::ITexture>   srvs;
    UnorderedAccessViewCache<buma3d::ITexture>  uavs;
    RenderTargetViewCache                       rtvs;
    DepthStencilViewCache                       dsvs;

};


}// namespace buma
