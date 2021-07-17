#pragma once

#include <Buma3D/Buma3D.h>

#include <Utils/Utils.h>
#include <Utils/NonCopyable.h>

#include <Buma3DHelpers/Buma3DHelpers.h>
#include <Buma3DHelpers/B3DDescHash.h>

#include <memory>
#include <unordered_map>
#include <mutex>

namespace buma
{

class SamplerViewCache
{
public:
    SamplerViewCache(buma3d::IDevice* _device)
        : device{ _device }
        , views_cache{}
    {
    }

    ~SamplerViewCache()
    {
        ClearCache();
    }

    DECLARE_NON_COPYABLE(SamplerViewCache);

    buma3d::ISamplerView* GetOrCreate(const buma3d::SAMPLER_DESC& _desc)
    {
        std::lock_guard lock(mutex);
        auto&& result = views_cache[_desc];
        if (result)
            return result;

        auto bmr = device->CreateSampler(_desc, &result);
        BMR_ASSERT(bmr);
        return result;
    }
    void ClearCache()
    {
        std::lock_guard lock(mutex);
        for (auto& [key, i] : views_cache)
        {
            BUMA_ASSERT(i->GetRefCount() == 1);
            util::SafeRelease(i);
        }
        views_cache.clear();
    }

private:
    std::mutex mutex; // FIXME: SamplerViewCache::mutex
    buma3d::IDevice* device;
    std::unordered_map<buma3d::SAMPLER_DESC, buma3d::ISamplerView*> views_cache;

};

template<typename ResourceT, typename DescT, typename ViewT, auto Method>
class ResourceViewCache
{
public:
    ResourceViewCache()
        : mutex{}
        , device{}
        , resource{}
        , views_cache{}
    {
    }

    ~ResourceViewCache()
    {
        ClearCache();
    }

    DECLARE_NON_COPYABLE(ResourceViewCache);

    void Init(ResourceT* _resource)
    {
        ClearCache();
        resource = _resource;
        device = resource->GetDevice();
    }
    
    ViewT* GetOrCreate(const DescT& _desc)
    {
        std::lock_guard lock(mutex);
        auto&& result = views_cache[_desc];
        if (result)
            return result;

        buma3d::BMRESULT bmr;
        if constexpr (std::is_same_v<decltype(Method), decltype(&buma3d::IDevice::CreateUnorderedAccessView)>)
            bmr = (device->*Method)(resource, nullptr, _desc, &result);
        else
            bmr = (device->*Method)(resource, _desc, &result);
        BMR_ASSERT(bmr);
        return result;
    }

    void ClearCache()
    {
        std::lock_guard lock(mutex);
        for (auto& [key,i] : views_cache)
        {
            BUMA_ASSERT(i->GetRefCount() == 1);
            util::SafeRelease(i);
        }
        views_cache.clear();
    }

private:
    std::mutex mutex; // FIXME: ResourceViewCache::mutex
    buma3d::IDevice* device;
    ResourceT* resource;
    std::unordered_map<DescT, ViewT*> views_cache;

};

template <typename ResourceT>
using ShaderResourceViewCache = ResourceViewCache<ResourceT, buma3d::SHADER_RESOURCE_VIEW_DESC, buma3d::IShaderResourceView, &buma3d::IDevice::CreateShaderResourceView>;

template <typename ResourceT>
using UnorderedAccessViewCache = ResourceViewCache<ResourceT, buma3d::UNORDERED_ACCESS_VIEW_DESC, buma3d::IUnorderedAccessView, &buma3d::IDevice::CreateUnorderedAccessView>;

using RenderTargetViewCache = ResourceViewCache<buma3d::ITexture, buma3d::RENDER_TARGET_VIEW_DESC, buma3d::IRenderTargetView, &buma3d::IDevice::CreateRenderTargetView>;

using DepthStencilViewCache = ResourceViewCache<buma3d::ITexture, buma3d::DEPTH_STENCIL_VIEW_DESC, buma3d::IDepthStencilView, &buma3d::IDevice::CreateDepthStencilView>;


} // namespace buma
