#pragma once

#include <Buma3D/Buma3D.h>

#include <cfloat>

namespace buma3d
{
namespace init
{

inline VIEWPORT Viewport(float _x, float _y, float _width, float _height, float _min_depth = B3D_VIEWPORT_MIN_DEPTH, float _max_depth = B3D_VIEWPORT_MAX_DEPTH)
{
    return VIEWPORT{ _x, _y, _width, _height, _min_depth, _max_depth };
}
inline SCISSOR_RECT ScissorRect(int32_t _x, int32_t _y, uint32_t _w, uint32_t _h)
{
    return SCISSOR_RECT{ _x, _y, _w, _h };
}

inline COMMAND_ALLOCATOR_DESC CommandAllocatorDesc(COMMAND_TYPE _type, COMMAND_LIST_LEVEL _level = COMMAND_LIST_LEVEL_PRIMARY, COMMAND_ALLOCATOR_FLAGS _flags = COMMAND_ALLOCATOR_FLAG_NONE)
{
    return COMMAND_ALLOCATOR_DESC{ _type, _level, _flags };
}

inline COMMAND_LIST_DESC CommandListDesc(ICommandAllocator* _allocator, NodeMask _node_mask)
{
    auto&& desc = _allocator->GetDesc();
    return COMMAND_LIST_DESC{ _allocator, desc.type, desc.level, _node_mask };
}

inline FENCE_DESC TimelineFenceDesc(uint64_t _initial_value = 0, FENCE_FLAGS _flags = FENCE_FLAG_NONE)
{
    return FENCE_DESC{ FENCE_TYPE_TIMELINE, _initial_value, _flags };
}

inline FENCE_DESC BinaryFenceDesc(uint64_t _initial_value = 0, FENCE_FLAGS _flags = FENCE_FLAG_NONE)
{
    return FENCE_DESC{ FENCE_TYPE_BINARY_GPU_TO_GPU, _initial_value, _flags };
}

inline FENCE_DESC BinaryCpuFenceDesc(uint64_t _initial_value = 0, FENCE_FLAGS _flags = FENCE_FLAG_NONE)
{
    return FENCE_DESC{ FENCE_TYPE_BINARY_GPU_TO_CPU, _initial_value, _flags };
}

inline SWAP_CHAIN_BUFFER_DESC SwapChainBufferDesc(uint32_t                      _width,
                                                  uint32_t                      _height,
                                                  uint32_t                      _count       = 3,
                                                  const TEXTURE_FORMAT_DESC&    _format_desc = { RESOURCE_FORMAT_UNKNOWN },
                                                  SWAP_CHAIN_BUFFER_FLAGS       _flags       = SWAP_CHAIN_BUFFER_FLAG_COPY_DST)
{
    return SWAP_CHAIN_BUFFER_DESC{ _width, _height, _count, _format_desc, _flags };
}

inline SWAP_CHAIN_DESC SwapChainDesc(ISurface*                      _surface,
                                     COLOR_SPACE                    _color_space,
                                     const SWAP_CHAIN_BUFFER_DESC&  _buffer,
                                     ICommandQueue*const *          _present_queues,
                                     uint32_t                       _num_present_queues = 1,
                                     ROTATION_MODE                  _pre_roration       = ROTATION_MODE_IDENTITY,
                                     SWAP_CHAIN_ALPHA_MODE          _alpha_mode         = SWAP_CHAIN_ALPHA_MODE_DEFAULT,
                                     SWAP_CHAIN_FLAGS               _flags              = SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT)
{
    return SWAP_CHAIN_DESC{ _surface, _color_space, _pre_roration, _buffer, _alpha_mode, _flags, _num_present_queues, _present_queues };
}


inline constexpr RESOURCE_HEAP_PROPERTY_FLAGS HEAP_HOST_VISIBLE_FLAGS = RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE;

inline constexpr BUFFER_USAGE_FLAGS BUF_COPYABLE_FLAGS      = BUFFER_USAGE_FLAG_COPY_DST | BUFFER_USAGE_FLAG_COPY_SRC;
inline constexpr BUFFER_USAGE_FLAGS BUF_CBV_FLAGS           = BUFFER_USAGE_FLAG_CONSTANT_BUFFER | BUF_COPYABLE_FLAGS;
inline constexpr BUFFER_USAGE_FLAGS BUF_SRV_FLAGS           = BUFFER_USAGE_FLAG_SHADER_RESOURCE_BUFFER | BUF_COPYABLE_FLAGS;
inline constexpr BUFFER_USAGE_FLAGS BUF_UAV_FLAGS           = BUFFER_USAGE_FLAG_UNORDERED_ACCESS_BUFFER | BUF_COPYABLE_FLAGS;

inline constexpr TEXTURE_USAGE_FLAGS TEX_COPYABLE_FLAGS     = TEXTURE_USAGE_FLAG_COPY_DST | TEXTURE_USAGE_FLAG_COPY_SRC;
inline constexpr TEXTURE_USAGE_FLAGS TEX_STATIC_SRV_FLAGS   = TEXTURE_USAGE_FLAG_SHADER_RESOURCE | TEXTURE_USAGE_FLAG_COPY_DST;
inline constexpr TEXTURE_USAGE_FLAGS TEX_COPYABLE_SRV_FLAGS = TEXTURE_USAGE_FLAG_SHADER_RESOURCE          | TEX_COPYABLE_FLAGS;
inline constexpr TEXTURE_USAGE_FLAGS TEX_UAV_FLAGS          = TEXTURE_USAGE_FLAG_UNORDERED_ACCESS         | TEX_COPYABLE_FLAGS;
inline constexpr TEXTURE_USAGE_FLAGS TEX_DSV_FLAGS          = TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT | TEX_COPYABLE_FLAGS;
inline constexpr TEXTURE_USAGE_FLAGS TEX_RTV_FLAGS          = TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT | TEX_COPYABLE_FLAGS;

inline RESOURCE_DESC BufferResourceDesc(uint64_t _size_in_bytes, BUFFER_USAGE_FLAGS _buffer_usage, RESOURCE_FLAGS _flags = RESOURCE_FLAG_NONE)
{
    return RESOURCE_DESC{ RESOURCE_DIMENSION_BUFFER, BUFFER_DESC{ _size_in_bytes, BUFFER_CREATE_FLAG_NONE, _buffer_usage }, _flags };
}

inline RESOURCE_DESC Tex2DResourceDesc(const EXTENT2D& _size, RESOURCE_FORMAT _format, TEXTURE_USAGE_FLAGS  _texture_usage = TEX_STATIC_SRV_FLAGS,
                                       uint32_t _mips = 1, uint32_t _array_size = 1, uint32_t _sample_cont = 1,
                                       TEXTURE_CREATE_FLAGS _texture_flags = TEXTURE_CREATE_FLAG_NONE, RESOURCE_FLAGS _flags = RESOURCE_FLAG_NONE)
{
    auto result = RESOURCE_DESC{ RESOURCE_DIMENSION_TEX2D, {}, _flags };
    result.texture = TEXTURE_DESC{ EXTENT3D{ _size.width, _size.height, 1 },
                                   _array_size,
                                   _mips,
                                   _sample_cont,
                                   TEXTURE_FORMAT_DESC{ _format, 0, nullptr },
                                   TEXTURE_LAYOUT_UNKNOWN,
                                   nullptr,
                                   _texture_flags,
                                   _texture_usage };

    return result;
}

inline COMMITTED_RESOURCE_DESC CommittedResourceDesc(  uint32_t             _heap_index
                                                     , RESOURCE_HEAP_FLAGS  _heap_flags
                                                     , const RESOURCE_DESC& _resource_desc
                                                     , NodeMask             _creation_node_mask     = B3D_DEFAULT_NODE_MASK
                                                     , NodeMask             _visible_node_mask      = B3D_DEFAULT_NODE_MASK
                                                     , uint32_t             _num_bind_node_masks    = 0
                                                     , const NodeMask*      _bind_node_masks        = nullptr)
{
    return COMMITTED_RESOURCE_DESC{ _heap_index, _heap_flags,
                                    _creation_node_mask, _visible_node_mask,
                                    _resource_desc,
                                    _num_bind_node_masks, _bind_node_masks };
}

inline SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDescDescTex2D(
    RESOURCE_FORMAT             _format,
    TEXTURE_ASPECT_FLAGS        _aspect         = TEXTURE_ASPECT_FLAG_COLOR,
    uint32_t                    _mip_slice      = 0,
    uint32_t                    _mip_levels     = 1,
    uint32_t                    _array_slice    = 0,
    uint32_t                    _array_size     = 1,
    SHADER_RESOURCE_VIEW_FLAGS  _flags          = SHADER_RESOURCE_VIEW_FLAG_NONE)
{
    SHADER_RESOURCE_VIEW_DESC result = { VIEW_DESC{ VIEW_TYPE_SHADER_RESOURCE, _format, _array_size > 1 ? VIEW_DIMENSION_TEXTURE_2D_ARRAY : VIEW_DIMENSION_TEXTURE_2D } };
    result.texture = TEXTURE_VIEW{ COMPONENT_MAPPING{ COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY },
                                   SUBRESOURCE_RANGE{ SUBRESOURCE_OFFSET{ _aspect,_mip_slice ,_array_slice }, _array_size, _mip_levels } };
    result.flags = _flags;

    return result;
}

inline SHADER_RESOURCE_VIEW_DESC SrvTex2D(
    RESOURCE_FORMAT             _format,
    TEXTURE_ASPECT_FLAGS        _aspect         = TEXTURE_ASPECT_FLAG_COLOR,
    uint32_t                    _mip_slice      = 0,
    uint32_t                    _mip_levels     = 1,
    uint32_t                    _array_slice    = 0,
    uint32_t                    _array_size     = 1,
    SHADER_RESOURCE_VIEW_FLAGS  _flags          = SHADER_RESOURCE_VIEW_FLAG_NONE)
{
    return ShaderResourceViewDescDescTex2D(_format, _aspect, _mip_slice, _mip_levels, _array_slice, _array_size, _flags);
}
inline UNORDERED_ACCESS_VIEW_DESC UavTex2D(
    RESOURCE_FORMAT             _format,
    TEXTURE_ASPECT_FLAGS        _aspect         = TEXTURE_ASPECT_FLAG_COLOR,
    uint32_t                    _mip_slice      = 0,
    uint32_t                    _mip_levels     = 1,
    uint32_t                    _array_slice    = 0,
    uint32_t                    _array_size     = 1,
    UNORDERED_ACCESS_VIEW_FLAGS _flags          = UNORDERED_ACCESS_VIEW_FLAG_NONE)
{
    UNORDERED_ACCESS_VIEW_DESC result = { VIEW_DESC{ VIEW_TYPE_UNORDERED_ACCESS, _format, _array_size > 1 ? VIEW_DIMENSION_TEXTURE_2D_ARRAY : VIEW_DIMENSION_TEXTURE_2D } };
    result.texture = TEXTURE_VIEW{ COMPONENT_MAPPING{ COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY },
                                   SUBRESOURCE_RANGE{ SUBRESOURCE_OFFSET{ _aspect, _mip_slice , _array_slice }, _array_size, _mip_levels } };
    result.flags = _flags;

    return result;
}

inline SHADER_RESOURCE_VIEW_DESC SrvBufferStructured(
    uint64_t                      _first_element,
    uint64_t                      _num_elements,
    uint32_t                      _structure_byte_stride,
    SHADER_RESOURCE_VIEW_FLAGS    _flags = SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT)
{
    SHADER_RESOURCE_VIEW_DESC result = { VIEW_DESC{ VIEW_TYPE_SHADER_RESOURCE, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_BUFFER_STRUCTURED },
                                         BUFFER_VIEW{_first_element, _num_elements, _structure_byte_stride }, _flags };
    return result;
}
inline UNORDERED_ACCESS_VIEW_DESC UavBufferStructured(
    uint64_t                      _first_element,
    uint64_t                      _num_elements,
    uint32_t                      _structure_byte_stride,
    UNORDERED_ACCESS_VIEW_FLAGS   _flags = UNORDERED_ACCESS_VIEW_FLAG_NONE)
{
    UNORDERED_ACCESS_VIEW_DESC result = { VIEW_DESC{ VIEW_TYPE_UNORDERED_ACCESS, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_BUFFER_STRUCTURED },
                                          BUFFER_VIEW{_first_element, _num_elements, _structure_byte_stride }, 0, _flags };
    return result;
}
inline SHADER_RESOURCE_VIEW_DESC SrvBufferByteAddress(
    uint64_t                      _first_element,
    uint64_t                      _num_elements,
    SHADER_RESOURCE_VIEW_FLAGS    _flags = SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT)
{
    SHADER_RESOURCE_VIEW_DESC result = { VIEW_DESC{ VIEW_TYPE_SHADER_RESOURCE, RESOURCE_FORMAT_R32_TYPELESS, VIEW_DIMENSION_BUFFER_BYTEADDRESS },
                                         BUFFER_VIEW{_first_element, _num_elements, 0 }, _flags };
    return result;
}
inline UNORDERED_ACCESS_VIEW_DESC UavBufferByteAddress(
    uint64_t                      _first_element,
    uint64_t                      _num_elements,
    UNORDERED_ACCESS_VIEW_FLAGS   _flags = UNORDERED_ACCESS_VIEW_FLAG_NONE)
{
    UNORDERED_ACCESS_VIEW_DESC result = { VIEW_DESC{ VIEW_TYPE_UNORDERED_ACCESS, RESOURCE_FORMAT_R32_TYPELESS, VIEW_DIMENSION_BUFFER_BYTEADDRESS },
                                          BUFFER_VIEW{_first_element, _num_elements, 0 }, 0, _flags };
    return result;
}

inline RENDER_TARGET_VIEW_DESC RenderTargetViewDescTex2D(
    RESOURCE_FORMAT             _format,
    TEXTURE_ASPECT_FLAGS        _aspect         = TEXTURE_ASPECT_FLAG_COLOR,
    uint32_t                    _mip_slice      = 0,
    uint32_t                    _mip_levels     = 1,
    uint32_t                    _array_slice    = 0,
    uint32_t                    _array_size     = 1,
    RENDER_TARGET_VIEW_FLAGS    _flags          = RENDER_TARGET_VIEW_FLAG_NONE
)
{
    return RENDER_TARGET_VIEW_DESC{ VIEW_DESC{ VIEW_TYPE_RENDER_TARGET, _format, _array_size > 1 ? VIEW_DIMENSION_TEXTURE_2D_ARRAY : VIEW_DIMENSION_TEXTURE_2D },
                                    TEXTURE_VIEW{ COMPONENT_MAPPING{ COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY },
                                    SUBRESOURCE_RANGE{ SUBRESOURCE_OFFSET{ _aspect,_mip_slice ,_array_slice }, _array_size, _mip_levels } }, _flags };
}

inline RENDER_TARGET_VIEW_DESC RtvTex2D(
    RESOURCE_FORMAT             _format,
    TEXTURE_ASPECT_FLAGS        _aspect         = TEXTURE_ASPECT_FLAG_COLOR,
    uint32_t                    _mip_slice      = 0,
    uint32_t                    _mip_levels     = 1,
    uint32_t                    _array_slice    = 0,
    uint32_t                    _array_size     = 1,
    RENDER_TARGET_VIEW_FLAGS    _flags          = RENDER_TARGET_VIEW_FLAG_NONE
)
{
    return RenderTargetViewDescTex2D(_format, _aspect, _mip_slice, _mip_levels, _array_slice, _array_size, _flags);
}
inline DEPTH_STENCIL_VIEW_DESC DsvTex2D(
    RESOURCE_FORMAT             _format         = RESOURCE_FORMAT_D32_FLOAT,
    TEXTURE_ASPECT_FLAGS        _aspect         = TEXTURE_ASPECT_FLAG_DEPTH,
    uint32_t                    _mip_slice      = 0,
    uint32_t                    _mip_levels     = 1,
    uint32_t                    _array_slice    = 0,
    uint32_t                    _array_size     = 1,
    DEPTH_STENCIL_VIEW_FLAGS    _flags          = DEPTH_STENCIL_VIEW_FLAG_NONE
)
{
    return DEPTH_STENCIL_VIEW_DESC{ VIEW_DESC{ VIEW_TYPE_DEPTH_STENCIL, _format, _array_size > 1 ? VIEW_DIMENSION_TEXTURE_2D_ARRAY : VIEW_DIMENSION_TEXTURE_2D },
                                    TEXTURE_VIEW{ COMPONENT_MAPPING{ COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY, COMPONENT_SWIZZLE_IDENTITY },
                                    SUBRESOURCE_RANGE{ SUBRESOURCE_OFFSET{ _aspect,_mip_slice,_array_slice }, _array_size, _mip_levels } }, _flags };
}

inline BUFFER_BARRIER_DESC BufferBarrierDesc(IBuffer*               _buffer,
                                             RESOURCE_STATE         _src_state,
                                             RESOURCE_STATE         _dst_state,
                                             COMMAND_TYPE           _src_queue_type = COMMAND_TYPE_DIRECT,
                                             COMMAND_TYPE           _dst_queue_type = COMMAND_TYPE_DIRECT,
                                             RESOURCE_BARRIER_FLAG  _barrier_flags  = RESOURCE_BARRIER_FLAG_NONE)
{
    return BUFFER_BARRIER_DESC{
        _buffer,
        _src_state,
        _dst_state,
        _src_queue_type,
        _dst_queue_type,
        _barrier_flags
    };
}

inline TEXTURE_BARRIER_DESC TextureBarrierDesc(const TEXTURE_BARRIER_RANGE* _barrier_range,
                                               RESOURCE_STATE               _src_state,
                                               RESOURCE_STATE               _dst_state,
                                               COMMAND_TYPE                 _src_queue_type = COMMAND_TYPE_DIRECT,
                                               COMMAND_TYPE                 _dst_queue_type = COMMAND_TYPE_DIRECT,
                                               RESOURCE_BARRIER_FLAG        _barrier_flags  = RESOURCE_BARRIER_FLAG_NONE)
{
    return TEXTURE_BARRIER_DESC{
        TEXTURE_BARRIER_TYPE_BARRIER_RANGE,
        _barrier_range,
        _src_state,
        _dst_state,
        _src_queue_type,
        _dst_queue_type,
        _barrier_flags
    };
}

inline TEXTURE_BARRIER_DESC TextureBarrierDesc(IView*                   _view,
                                               RESOURCE_STATE           _src_state,
                                               RESOURCE_STATE           _dst_state,
                                               COMMAND_TYPE             _src_queue_type = COMMAND_TYPE_DIRECT,
                                               COMMAND_TYPE             _dst_queue_type = COMMAND_TYPE_DIRECT,
                                               RESOURCE_BARRIER_FLAG    _barrier_flags  = RESOURCE_BARRIER_FLAG_NONE)
{
    auto result = TEXTURE_BARRIER_DESC{
        TEXTURE_BARRIER_TYPE_VIEW,
        nullptr,
        _src_state,
        _dst_state,
        _src_queue_type,
        _dst_queue_type,
        _barrier_flags
    };
    result.view = _view;
    return result;
}

inline RASTERIZATION_STATE_DESC RasterizationStateDesc(FILL_MODE _fill_mode, CULL_MODE _cull_mode, bool _is_front_counter_clockwise, bool _is_enabled_conservative_raster
                                                       , LINE_RASTERIZATION_MODE _line_rasterization_mode = LINE_RASTERIZATION_MODE_DEFAULT, float _line_width = 0)
{
    return RASTERIZATION_STATE_DESC{
          _fill_mode                        // fill_mode;
        , _cull_mode                        // cull_mode;
        , _is_front_counter_clockwise       // is_front_counter_clockwise;
        , true                              // is_enabled_depth_clip;
        , false                             // is_enabled_depth_bias;
        , 0                                 // depth_bias_scale;
        , 0                                 // depth_bias_clamp;
        , 0                                 // depth_bias_slope_scale;
        , _is_enabled_conservative_raster   // is_enabled_conservative_raster;
        , _line_rasterization_mode          // line_rasterization_mode;
        , _line_width                       // line_width;
    };
}
inline RASTERIZATION_STATE_DESC RasterizationStateDesc(FILL_MODE _fill_mode, CULL_MODE _cull_mode, bool _is_front_counter_clockwise, bool _is_enabled_conservative_raster
                                                       , bool _is_enabled_depth_clip
                                                       , int32_t _depth_bias_scale = 0, float _depth_bias_clamp = 0, float _depth_bias_slope_scale = 0
                                                       , LINE_RASTERIZATION_MODE _line_rasterization_mode = LINE_RASTERIZATION_MODE_DEFAULT, float _line_width = 0)
{
    return RASTERIZATION_STATE_DESC{
          _fill_mode                        // fill_mode;
        , _cull_mode                        // cull_mode;
        , _is_front_counter_clockwise       // is_front_counter_clockwise;
        , _is_enabled_depth_clip            // is_enabled_depth_clip;
        , true                              // is_enabled_depth_bias;
        , _depth_bias_scale                 // depth_bias_scale;
        , _depth_bias_clamp                 // depth_bias_clamp;
        , _depth_bias_slope_scale           // depth_bias_slope_scale;
        , _is_enabled_conservative_raster   // is_enabled_conservative_raster;
        , _line_rasterization_mode          // line_rasterization_mode;
        , _line_width                       // line_width;
    };
}

inline MULTISAMPLE_STATE_DESC MultisampleStateDesc(  uint32_t            _rasterization_samples          = 1
                                                   , bool                _is_enabled_alpha_to_coverage   = false
                                                   , bool                _is_enabled_sample_rate_shading = false
                                                   , const SampleMask*   _sample_masks                   = buma3d::B3D_DEFAULT_SAMPLE_MASK
)
{
    return MULTISAMPLE_STATE_DESC{
          _rasterization_samples
        , _sample_masks
        , _is_enabled_alpha_to_coverage
        , _is_enabled_sample_rate_shading
        , SAMPLE_POSITION_STATE_DESC{ false }
    };
}

inline DEPTH_STENCILOP_DESC DepthStencilOpDesc(
      STENCIL_OP      _fail_op         = buma3d::STENCIL_OP_KEEP
    , STENCIL_OP      _depth_fail_op   = buma3d::STENCIL_OP_KEEP
    , STENCIL_OP      _pass_op         = buma3d::STENCIL_OP_KEEP
    , COMPARISON_FUNC _comparison_func = buma3d::COMPARISON_FUNC_ALWAYS
    , uint32_t        _compare_mask    = buma3d::B3D_DEFAULT_STENCIL_COMPARE_MASK
    , uint32_t        _write_mask      = buma3d::B3D_DEFAULT_STENCIL_WRITE_MASK
    , uint32_t        _reference       = buma3d::B3D_DEFAULT_STENCIL_REFERENCE)
{
    return DEPTH_STENCILOP_DESC{
          _fail_op         // STENCIL_OP      fail_op;
        , _depth_fail_op   // STENCIL_OP      depth_fail_op;
        , _pass_op         // STENCIL_OP      pass_op;
        , _comparison_func // COMPARISON_FUNC comparison_func;
        , _compare_mask    // uint32_t        compare_mask;
        , _write_mask      // uint32_t        write_mask;
        , _reference       // uint32_t        reference;
    };
}

inline DEPTH_STENCIL_STATE_DESC DepthStencilStateDesc()
{
    return DEPTH_STENCIL_STATE_DESC{
          false                     // bool                    is_enabled_depth_test;
        , false                     // bool                    is_enabled_depth_write;
        , COMPARISON_FUNC_NEVER     // COMPARISON_FUNC         depth_comparison_func;
        , false                     // bool                    is_enabled_depth_bounds_test;
        , 0.f                       // float                   min_depth_bounds;
        , 1.f                       // float                   max_depth_bounds;
        , false                     // bool                    is_enabled_stencil_test;
        , DepthStencilOpDesc()      // DEPTH_STENCILOP_DESC    stencil_front_face;
        , DepthStencilOpDesc()      // DEPTH_STENCILOP_DESC    stencil_back_face;
    };
}
inline DEPTH_STENCIL_STATE_DESC DepthStencilStateDesc(  bool                    _is_enabled_depth_write
                                                      , COMPARISON_FUNC         _depth_comparison_func)
{
    return DEPTH_STENCIL_STATE_DESC{
          true                      // bool                    is_enabled_depth_test;
        , _is_enabled_depth_write   // bool                    is_enabled_depth_write;
        , _depth_comparison_func    // COMPARISON_FUNC         depth_comparison_func;

        , false                     // bool                    is_enabled_depth_bounds_test;
        , 0.f                       // float                   min_depth_bounds;
        , 1.f                       // float                   max_depth_bounds;

        , false                     // bool                    is_enabled_stencil_test;
        , DepthStencilOpDesc()      // DEPTH_STENCILOP_DESC    stencil_front_face;
        , DepthStencilOpDesc()      // DEPTH_STENCILOP_DESC    stencil_back_face;
    };
}
inline DEPTH_STENCIL_STATE_DESC DepthStencilStateDesc(  bool                    _is_enabled_depth_write
                                                      , COMPARISON_FUNC         _depth_comparison_func
                                                      , float                   _min_depth_bounds
                                                      , float                   _max_depth_bounds
)
{
    return DEPTH_STENCIL_STATE_DESC{
          true                      // bool                    is_enabled_depth_test;
        , _is_enabled_depth_write   // bool                    is_enabled_depth_write;
        , _depth_comparison_func    // COMPARISON_FUNC         depth_comparison_func;

        , true                      // bool                    is_enabled_depth_bounds_test;
        , _min_depth_bounds         // float                   min_depth_bounds;
        , _max_depth_bounds         // float                   max_depth_bounds;

        , false                     // bool                    is_enabled_stencil_test;
        , DepthStencilOpDesc()      // DEPTH_STENCILOP_DESC    stencil_front_face;
        , DepthStencilOpDesc()      // DEPTH_STENCILOP_DESC    stencil_back_face;
    };
}
inline DEPTH_STENCIL_STATE_DESC DepthStencilStateDesc(  bool                        _is_enabled_depth_write
                                                      , COMPARISON_FUNC             _depth_comparison_func
                                                      , float                       _min_depth_bounds
                                                      , float                       _max_depth_bounds
                                                      , const DEPTH_STENCILOP_DESC& _stencil_front_face
                                                      , const DEPTH_STENCILOP_DESC& _stencil_back_face
)
{
    return DEPTH_STENCIL_STATE_DESC{
          true                      // bool                    is_enabled_depth_test;
        , _is_enabled_depth_write   // bool                    is_enabled_depth_write;
        , _depth_comparison_func    // COMPARISON_FUNC         depth_comparison_func;

        , true                      // bool                    is_enabled_depth_bounds_test;
        , _min_depth_bounds         // float                   min_depth_bounds;
        , _max_depth_bounds         // float                   max_depth_bounds;

        , true                     // bool                    is_enabled_stencil_test;
        , _stencil_front_face      // DEPTH_STENCILOP_DESC    stencil_front_face;
        , _stencil_back_face       // DEPTH_STENCILOP_DESC    stencil_back_face;
    };
}

inline SAMPLER_DESC SamplerDesc()
{
    return SAMPLER_DESC{
        SAMPLER_FILTER_DESC{
            SAMPLER_FILTER_MODE_STANDARD,           // mode
            SAMPLER_FILTER_REDUCTION_MODE_STANDARD, // reduction_mode
            1,                                      // max_anisotropy  NOTE: 異方性フィルタが有効の場合にのみ使用されます。
            COMPARISON_FUNC_NEVER                   // comparison_func NOTE: REDUCTION_MODE_COMPARISONの場合にのみ使用されます。
        },
        SAMPLER_TEXTURE_DESC{
            TEXTURE_SAMPLE_MODE_DESC { TEXTURE_SAMPLE_MODE_LINEAR, TEXTURE_SAMPLE_MODE_LINEAR, TEXTURE_SAMPLE_MODE_LINEAR }, // min mag mip
            TEXTURE_ADDRESS_MODE_DESC{ TEXTURE_ADDRESS_MODE_CLAMP, TEXTURE_ADDRESS_MODE_CLAMP, TEXTURE_ADDRESS_MODE_CLAMP }  // u v w
        },
        SAMPLER_MIP_LOD_DESC{
            0.f,      // min
            FLT_MAX,  // max
            0.f       // bias
        },
        BORDER_COLOR_TRANSPARENT_BLACK_FLOAT // border_color NOTE: いずれかのサンプルモードがBORDERの場合にのみ使用されます。
    };
}

}// namespace init
}// namespace buma3d
