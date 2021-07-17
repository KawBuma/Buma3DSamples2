#pragma once

#include <DeviceResources/DeviceResources.h>

#include <DeviceResources/private/ResourceViewCache.h>

namespace buma
{

enum RESOURCE_CREATE_TYPE
{
      RESOURCE_CREATE_TYPE_COMMITTED
    , RESOURCE_CREATE_TYPE_PLACED
    , RESOURCE_CREATE_TYPE_RESERVED

    , RESOURCE_CREATE_TYPE_SWAP_CHAIN
};

struct RESOURCE_HEAP_ALLOCATION;

struct ResourceBase
{
public:
    ResourceBase(DeviceResources& _dr, RESOURCE_CREATE_TYPE _create_type);
    ResourceBase(const ResourceBase&) = delete;

    virtual ~ResourceBase();

    RESOURCE_CREATE_TYPE                        GetResourceCreateType() const { create_type; }
    const buma3d::util::Ptr<buma3d::IResource>& GetB3DResource() const { return resource; }

    buma3d::BMRESULT             SetName(const char* _name) { return resource->SetName(_name); }
    const char*                  GetName()                  { return resource->GetName(); }
    const buma3d::RESOURCE_DESC& GetDesc() const            { return resource->GetDesc(); }

protected:
    bool CreateResource(const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags);
    bool AllocateHeap(buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _deny_heap_flags);
    bool Bind();

protected:
    DeviceResources&                        dr;
    RESOURCE_CREATE_TYPE                    create_type;
    buma3d::util::Ptr<buma3d::IResource>    resource;
    RESOURCE_HEAP_ALLOCATION*               heap_allocation;

};


}// namespace buma
