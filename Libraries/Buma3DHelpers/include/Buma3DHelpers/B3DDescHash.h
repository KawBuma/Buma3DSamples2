#pragma once

#include <Buma3D/Buma3D.h>
#include <Utils/Utils.h>

namespace buma::util
{

template<typename T>
inline void hash_combine(size_t& _seed, T const& _val)
{
    // https://suzulang.com/cpp-64bit-hash-combine/
    _seed ^= std::hash<T>{}(_val)+0x9e3779b97f4a7c15ull + (_seed << 12) + (_seed >> 4);
}

} // namespace buma::util


namespace buma3d
{

inline bool operator==(const VIEW_DESC& a, const VIEW_DESC& b)
{
    return
        a.type      == b.type      &&
        a.format    == b.format    &&
        a.dimension == b.dimension ;
}

inline bool operator==(const BUFFER_VIEW& a, const BUFFER_VIEW& b)
{
    return
        a.first_element         == b.first_element         &&
        a.num_elements          == b.num_elements          &&
        a.structure_byte_stride == b.structure_byte_stride ;
}

inline bool operator==(const COMPONENT_MAPPING& a, const COMPONENT_MAPPING& b)
{
    return
        a.r == b.r &&
        a.g == b.g &&
        a.b == b.b &&
        a.a == b.a ;
}

inline bool operator==(const SUBRESOURCE_OFFSET& a, const SUBRESOURCE_OFFSET& b)
{
    return
        a.aspect      == b.aspect      &&
        a.mip_slice   == b.mip_slice   &&
        a.array_slice == b.array_slice ;
}

inline bool operator==(const SUBRESOURCE_RANGE& a, const SUBRESOURCE_RANGE& b)
{
    return
        a.offset     == b.offset     &&
        a.mip_levels == b.mip_levels &&
        a.array_size == b.array_size ;
}

inline bool operator==(const TEXTURE_VIEW& a, const TEXTURE_VIEW& b)
{
    return a.components == b.components && a.subresource_range == b.subresource_range;
}

inline bool operator==(const SHADER_RESOURCE_VIEW_DESC& a, const SHADER_RESOURCE_VIEW_DESC& b)
{
    auto CompareDimension = [](const SHADER_RESOURCE_VIEW_DESC& a, const SHADER_RESOURCE_VIEW_DESC& b)
    {
        switch (a.view.dimension)
        {
        case buma3d::VIEW_DIMENSION_BUFFER_TYPED:
        case buma3d::VIEW_DIMENSION_BUFFER_STRUCTURED:
        case buma3d::VIEW_DIMENSION_BUFFER_BYTEADDRESS:
            return a.buffer == b.buffer;

        case buma3d::VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE:
            return a.acceleration_structure.location == b.acceleration_structure.location;

        case buma3d::VIEW_DIMENSION_TEXTURE_1D:
        case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
        case buma3d::VIEW_DIMENSION_TEXTURE_2D:
        case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
        case buma3d::VIEW_DIMENSION_TEXTURE_3D:
        case buma3d::VIEW_DIMENSION_TEXTURE_CUBE:
        case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY:
            return a.texture == b.texture;

        default:
            return false;
        }
    };
    return
        a.flags == a.flags &&
        a.view  == b.view  && CompareDimension(a, b);
}

inline bool operator==(const UNORDERED_ACCESS_VIEW_DESC& a, const UNORDERED_ACCESS_VIEW_DESC& b)
{
    auto CompareDimension = [](const UNORDERED_ACCESS_VIEW_DESC& a, const UNORDERED_ACCESS_VIEW_DESC& b)
    {
        switch (a.view.dimension)
        {
        case buma3d::VIEW_DIMENSION_BUFFER_TYPED:
        case buma3d::VIEW_DIMENSION_BUFFER_STRUCTURED:
        case buma3d::VIEW_DIMENSION_BUFFER_BYTEADDRESS:
            return a.buffer == b.buffer;

        case buma3d::VIEW_DIMENSION_TEXTURE_1D:
        case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
        case buma3d::VIEW_DIMENSION_TEXTURE_2D:
        case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
        case buma3d::VIEW_DIMENSION_TEXTURE_3D:
        case buma3d::VIEW_DIMENSION_TEXTURE_CUBE:
        case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY:
            return a.texture == b.texture;

        default:
            return false;
        }
    };
    return
        a.counter_offset_in_bytes == a.counter_offset_in_bytes &&
        a.flags                   == a.flags                   &&
        a.view                    == b.view                    && CompareDimension(a, b);
}

inline bool operator==(const RENDER_TARGET_VIEW_DESC& a, const RENDER_TARGET_VIEW_DESC& b)
{
    return
        a.flags   == a.flags &&
        a.view    == b.view  &&
        a.texture == b.texture;
}

inline bool operator==(const DEPTH_STENCIL_VIEW_DESC& a, const DEPTH_STENCIL_VIEW_DESC& b)
{
    return
        a.flags   == a.flags &&
        a.view    == b.view  &&
        a.texture == b.texture;
}

inline bool operator==(const SAMPLER_DESC& a, const SAMPLER_DESC& b)
{
    return
        a.filter.mode                   == b.filter.mode                    &&
        a.filter.reduction_mode         == b.filter.reduction_mode          &&
        a.filter.max_anisotropy         == b.filter.max_anisotropy          &&
        a.filter.comparison_func        == b.filter.comparison_func         &&

        a.texture.sample.minification   == b.texture.sample.minification    &&
        a.texture.sample.magnification  == b.texture.sample.magnification   &&
        a.texture.sample.mip            == b.texture.sample.mip             &&

        a.texture.address.u             == b.texture.address.u              &&
        a.texture.address.v             == b.texture.address.v              &&
        a.texture.address.w             == b.texture.address.w              &&

        a.mip_lod.bias                  == b.mip_lod.bias                   &&
        a.mip_lod.max                   == b.mip_lod.max                    &&
        a.mip_lod.min                   == b.mip_lod.min                    &&
        a.border_color                  == b.border_color                   ;
}

}// namespace buma3d

namespace std
{

template<>
struct hash<buma3d::VIEW_DESC>
{
    size_t operator()(const buma3d::VIEW_DESC& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.type);
        buma::util::hash_combine(seed, _data.format);
        buma::util::hash_combine(seed, _data.dimension);
        return seed;
    }
};

template<>
struct hash<buma3d::BUFFER_VIEW>
{
    size_t operator()(const buma3d::BUFFER_VIEW& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.first_element        );
        buma::util::hash_combine(seed, _data.num_elements         );
        buma::util::hash_combine(seed, _data.structure_byte_stride);
        return seed;
    }
};

template<>
struct hash<buma3d::COMPONENT_MAPPING>
{
    size_t operator()(const buma3d::COMPONENT_MAPPING& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.r);
        buma::util::hash_combine(seed, _data.g);
        buma::util::hash_combine(seed, _data.b);
        buma::util::hash_combine(seed, _data.a);
        return seed;
    }
};

template<>
struct hash<buma3d::SUBRESOURCE_OFFSET>
{
    size_t operator()(const buma3d::SUBRESOURCE_OFFSET& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.aspect      );
        buma::util::hash_combine(seed, _data.mip_slice   );
        buma::util::hash_combine(seed, _data.array_slice );
        return seed;
    }
};

template<>
struct hash<buma3d::SUBRESOURCE_RANGE>
{
    size_t operator()(const buma3d::SUBRESOURCE_RANGE& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.offset     );
        buma::util::hash_combine(seed, _data.mip_levels );
        buma::util::hash_combine(seed, _data.array_size );
        return seed;
    }
};

template<>
struct hash<buma3d::TEXTURE_VIEW>
{
    size_t operator()(const buma3d::TEXTURE_VIEW& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.components       );
        buma::util::hash_combine(seed, _data.subresource_range);
        return seed;
    }
};

template<>
struct hash<buma3d::SHADER_RESOURCE_VIEW_DESC>
{
    size_t operator()(const buma3d::SHADER_RESOURCE_VIEW_DESC& _data) const
    {
        auto HashDimension = [](size_t _seed, const buma3d::SHADER_RESOURCE_VIEW_DESC& _data)
        {
            switch (_data.view.dimension)
            {
            case buma3d::VIEW_DIMENSION_BUFFER_TYPED:
            case buma3d::VIEW_DIMENSION_BUFFER_STRUCTURED:
            case buma3d::VIEW_DIMENSION_BUFFER_BYTEADDRESS:
                buma::util::hash_combine(_seed, _data.view);
                break;

            case buma3d::VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE:
                buma::util::hash_combine(_seed, _data.acceleration_structure.location);
                break;

            case buma3d::VIEW_DIMENSION_TEXTURE_1D:
            case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
            case buma3d::VIEW_DIMENSION_TEXTURE_2D:
            case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
            case buma3d::VIEW_DIMENSION_TEXTURE_3D:
            case buma3d::VIEW_DIMENSION_TEXTURE_CUBE:
            case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY:
                buma::util::hash_combine(_seed, _data.texture);
                break;

            default:
                break;
            }
        };

        size_t seed = 0;
        buma::util::hash_combine(seed, _data.view);
        HashDimension(seed, _data);
        buma::util::hash_combine(seed, _data.flags);
        return seed;
    }
};

template<>
struct hash<buma3d::UNORDERED_ACCESS_VIEW_DESC>
{
    size_t operator()(const buma3d::UNORDERED_ACCESS_VIEW_DESC& _data) const
    {
        auto HashDimension = [](size_t _seed, const buma3d::UNORDERED_ACCESS_VIEW_DESC& _data)
        {
            switch (_data.view.dimension)
            {
            case buma3d::VIEW_DIMENSION_BUFFER_TYPED:
            case buma3d::VIEW_DIMENSION_BUFFER_STRUCTURED:
            case buma3d::VIEW_DIMENSION_BUFFER_BYTEADDRESS:
                buma::util::hash_combine(_seed, _data.view);
                break;

            case buma3d::VIEW_DIMENSION_TEXTURE_1D:
            case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
            case buma3d::VIEW_DIMENSION_TEXTURE_2D:
            case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
            case buma3d::VIEW_DIMENSION_TEXTURE_3D:
            case buma3d::VIEW_DIMENSION_TEXTURE_CUBE:
            case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY:
                buma::util::hash_combine(_seed, _data.texture);
                break;

            default:
                break;
            }
        };

        size_t seed = 0;
        buma::util::hash_combine(seed, _data.view);
        HashDimension(seed, _data);
        buma::util::hash_combine(seed, _data.counter_offset_in_bytes);
        buma::util::hash_combine(seed, _data.flags);
        return seed;
    }
};

template<>
struct hash<buma3d::RENDER_TARGET_VIEW_DESC>
{
    size_t operator()(const buma3d::RENDER_TARGET_VIEW_DESC& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.view);
        buma::util::hash_combine(seed, _data.texture);
        buma::util::hash_combine(seed, _data.flags);
        return seed;
    }
};

template<>
struct hash<buma3d::DEPTH_STENCIL_VIEW_DESC>
{
    size_t operator()(const buma3d::DEPTH_STENCIL_VIEW_DESC& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.view);
        buma::util::hash_combine(seed, _data.texture);
        buma::util::hash_combine(seed, _data.flags);
        return seed;
    }
};

template<>
struct hash<buma3d::SAMPLER_DESC>
{
    size_t operator()(const buma3d::SAMPLER_DESC& _data) const
    {
        size_t seed = 0;
        buma::util::hash_combine(seed, _data.filter.mode                   );
        buma::util::hash_combine(seed, _data.filter.reduction_mode         );
        buma::util::hash_combine(seed, _data.filter.max_anisotropy         );
        buma::util::hash_combine(seed, _data.filter.comparison_func        );

        buma::util::hash_combine(seed, _data.texture.sample.minification   );
        buma::util::hash_combine(seed, _data.texture.sample.magnification  );
        buma::util::hash_combine(seed, _data.texture.sample.mip            );

        buma::util::hash_combine(seed, _data.texture.address.u             );
        buma::util::hash_combine(seed, _data.texture.address.v             );
        buma::util::hash_combine(seed, _data.texture.address.w             );

        buma::util::hash_combine(seed, _data.mip_lod.bias                  );
        buma::util::hash_combine(seed, _data.mip_lod.max                   );
        buma::util::hash_combine(seed, _data.mip_lod.min                   );
        buma::util::hash_combine(seed, _data.border_color                  );

        return seed;
    }
};


}// namespace std
