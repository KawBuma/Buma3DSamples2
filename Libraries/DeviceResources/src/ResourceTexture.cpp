#include <DeviceResources/ResourceTexture.h>

#include <Utils/Utils.h>

namespace buma
{

Texture::Texture(DeviceResources& _dr, RESOURCE_CREATE_TYPE _create_type
                                 , const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags)
    : ResourceBase(_dr, _create_type)
    , srvs{}
    , uavs{}
    , rtvs{}
    , dsvs{}
{
    CreateResource(_desc, _heap_flags, _deny_heap_flags);
    srvs.Init(GetB3DTexture().Get());
    uavs.Init(GetB3DTexture().Get());
    rtvs.Init(GetB3DTexture().Get());
    dsvs.Init(GetB3DTexture().Get());
}

Texture::Texture(DeviceResources& _dr, buma3d::ITexture* _swapchain_texture)
    : ResourceBase(_dr, RESOURCE_CREATE_TYPE_SWAP_CHAIN)
    , srvs{}
    , uavs{}
    , rtvs{}
    , dsvs{}
{
    resource = _swapchain_texture;
    srvs.Init(GetB3DTexture().Get());
    uavs.Init(GetB3DTexture().Get());
    rtvs.Init(GetB3DTexture().Get());
    dsvs.Init(GetB3DTexture().Get());
}

Texture::~Texture()
{

}

buma3d::IShaderResourceView* Texture::GetSRV(const buma3d::SHADER_RESOURCE_VIEW_DESC& _desc)
{
    return srvs.GetOrCreate(_desc);
}

buma3d::IUnorderedAccessView* Texture::GetUAV(const buma3d::UNORDERED_ACCESS_VIEW_DESC& _desc)
{
    return uavs.GetOrCreate(_desc);
}

buma3d::IRenderTargetView* Texture::GetRTV(const buma3d::RENDER_TARGET_VIEW_DESC& _desc)
{
    return rtvs.GetOrCreate(_desc);
}

buma3d::IDepthStencilView* Texture::GetDSV(const buma3d::DEPTH_STENCIL_VIEW_DESC& _desc)
{
    return dsvs.GetOrCreate(_desc);
}


}// namespace buma
