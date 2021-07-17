#pragma once

#include <DeviceResources/Resource.h>

namespace buma
{

class Buffer : public ResourceBase
{
public:
    Buffer(DeviceResources& _dr, RESOURCE_CREATE_TYPE _create_type
           , const buma3d::RESOURCE_DESC& _desc, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS _heap_flags, buma3d::RESOURCE_HEAP_PROPERTY_FLAGS  _deny_heap_flags);
    ~Buffer();

    buma3d::util::Ptr<buma3d::IBuffer> GetB3DBuffer() const { return GetB3DResource().As<buma3d::IBuffer>(); }

    /**
     * @brief ホスト可視ヒープでサブ割り当てされた場合、マップされたポインタを返します。
     * @return このリソースへオフセットされたマップされたポインタを返します。  マップ不可リソースの場合nullptrを返します。
    */
    void*                               GetMppedData();
    const buma3d::MAPPED_RANGE*         GetMppedRange() const;
    void                                Flush(const buma3d::MAPPED_RANGE* _range = nullptr);
    void                                Invalidate(const buma3d::MAPPED_RANGE* _range = nullptr);

    template<typename T>
    T* GetMppedDataAs(size_t _index = 0) { return static_cast<T*>(GetMppedData()) + _index; }

    buma3d::IShaderResourceView* GetSRV(const buma3d::SHADER_RESOURCE_VIEW_DESC& _desc);
    buma3d::IUnorderedAccessView* GetUAV(const buma3d::UNORDERED_ACCESS_VIEW_DESC& _desc);

private:
    void*                                       mapped_data;
    buma3d::MAPPED_RANGE                        mapped_range;
    ShaderResourceViewCache<buma3d::IBuffer>    srvs;
    UnorderedAccessViewCache<buma3d::IBuffer>   uavs;

};


}// namespace buma
