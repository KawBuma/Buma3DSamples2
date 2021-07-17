#pragma once

#include <Buma3DHelpers/Buma3DHelpers.h>

namespace buma
{

class CompatibleResourceHeapProperties
{
    friend class ResourceHeapProperties;
    void Init(const std::vector<buma3d::RESOURCE_HEAP_PROPERTIES>& _heap_properties, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _flags)
    {
        flags = _flags;
        for (auto& i : _heap_properties)
        {
            if (i.flags & _flags)
                props.push_back(&i);
        }
    }

public:
    CompatibleResourceHeapProperties()
        : props {}
        , flags{}
    {
    }

    ~CompatibleResourceHeapProperties()
    {
    }

    operator bool() const { return props.empty(); }

    const std::vector<const buma3d::RESOURCE_HEAP_PROPERTIES*>& Get() const { return props; }


    const buma3d::RESOURCE_HEAP_PROPERTIES* Find(const std::vector<buma3d::IResource*>& _resources, std::vector<buma3d::RESOURCE_ALLOCATION_INFO>* _resource_info = nullptr, buma3d::RESOURCE_HEAP_ALLOCATION_INFO* _heap_info = nullptr) const
    {
        std::vector<buma3d::RESOURCE_ALLOCATION_INFO>   resource_infos(_resources.size());
        buma3d::RESOURCE_HEAP_ALLOCATION_INFO           heap_info{};
        auto d = _resources[0]->GetDevice();
        d->GetResourceAllocationInfo((uint32_t)_resources.size(), _resources.data(), resource_infos.data(), &heap_info);

        if (_resource_info)
            _resource_info->swap(resource_infos);
        if (_heap_info)
            *_heap_info = heap_info;

        for (auto& i : props)
        {
            if (heap_info.heap_type_bits & (1 << i->heap_index))
                return i;
        }
        return nullptr;
    }
    const buma3d::RESOURCE_HEAP_PROPERTIES* Find(buma3d::IResource* _resource, buma3d::RESOURCE_ALLOCATION_INFO* _resource_info = nullptr, buma3d::RESOURCE_HEAP_ALLOCATION_INFO* _heap_info = nullptr) const
    {
        buma3d::RESOURCE_ALLOCATION_INFO        resource_info{};
        buma3d::RESOURCE_HEAP_ALLOCATION_INFO   heap_info{};
        _resource->GetDevice()->GetResourceAllocationInfo(1, &_resource, &resource_info, &heap_info);

        if (_resource_info)
            *_resource_info = resource_info;
        if (_heap_info)
            *_heap_info = heap_info;

        for (auto& i : props)
        {
            if (heap_info.heap_type_bits & (1 << i->heap_index))
                return i;
        }
        return nullptr;
    }
    const buma3d::RESOURCE_HEAP_PROPERTIES* Find(buma3d::IDevice* _device, const buma3d::RESOURCE_DESC& _resource_desc, buma3d::RESOURCE_ALLOCATION_INFO* _resource_info = nullptr, buma3d::RESOURCE_HEAP_ALLOCATION_INFO* _heap_info = nullptr) const
    {
        buma3d::util::Ptr<buma3d::IResource> res;
        auto bmr = _device->CreatePlacedResource(_resource_desc, &res);
        if (util::IsFailed(bmr))
            return nullptr;

        return Find(res.Get(), _resource_info, _heap_info);
    }

    CompatibleResourceHeapProperties Filter(
          buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _flags = buma3d::RESOURCE_HEAP_FLAG_NONE
        , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_flags = buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_COPY_DST_FIXED |
                                                             buma3d::RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED ) const
    {
        if (_flags == buma3d::RESOURCE_HEAP_FLAG_NONE)
            _flags = flags;

        CompatibleResourceHeapProperties result{};
        for (auto& i : props)
        {
            if ((i->flags & _deny_flags) == 0 && (i->flags & _flags))
                result.props.emplace_back(i);
        }
        return result;
    }

private:
    std::vector<const buma3d::RESOURCE_HEAP_PROPERTIES*>    props;
    buma3d::RESOURCE_HEAP_PROPERTY_FLAGS                    flags;

};

class ResourceHeapProperties
{
public:
    ResourceHeapProperties(buma3d::IDevice* _device)
        : heap_properties{}
    {
        heap_properties.resize(_device->GetResourceHeapProperties(nullptr));
        _device->GetResourceHeapProperties(heap_properties.data());

        device_local_heaps   .Init(heap_properties, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL);
        host_readable_heaps  .Init(heap_properties, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE);
        host_writable_heaps  .Init(heap_properties, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE);
        host_read_write_heaps.Init(heap_properties, buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | buma3d::RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE);
    }

    ~ResourceHeapProperties()
    {
    }

    const std::vector<buma3d::RESOURCE_HEAP_PROPERTIES>& Get() const { return heap_properties; }
    const CompatibleResourceHeapProperties& GetDeviceLocalHeaps() const { return device_local_heaps; }
    const CompatibleResourceHeapProperties& GetHostReadableHeaps() const { return host_readable_heaps; }
    const CompatibleResourceHeapProperties& GetHostWritableHeaps() const { return host_writable_heaps; }
    const CompatibleResourceHeapProperties& GetHostReadWriteHeaps() const { return host_read_write_heaps; }

    void FindCompatibleHeaps(std::vector<const buma3d::RESOURCE_HEAP_PROPERTIES*>*  _dst
                             , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS                 _flags
                             , buma3d::RESOURCE_HEAP_PROPERTY_FLAGS                 _deny_flags)
    {
        for (auto& i : heap_properties)
        {
            if (!(i.flags & _deny_flags) && i.flags & _flags)
                _dst->push_back(&i);
        }
    }

    uint32_t/*heap_type_bits*/ FindCompatibleHeaps(buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_flags)
    {
        uint32_t result = 0;
        for (auto& i : heap_properties)
        {
            if (!(i.flags & _deny_flags) && i.flags & _flags)
                result |= 1 << i.heap_index;
        }
        return result;
    }

private:
    std::vector<buma3d::RESOURCE_HEAP_PROPERTIES> heap_properties;
    CompatibleResourceHeapProperties device_local_heaps;
    CompatibleResourceHeapProperties host_readable_heaps;
    CompatibleResourceHeapProperties host_writable_heaps;
    CompatibleResourceHeapProperties host_read_write_heaps;

};


}// namespace buma
