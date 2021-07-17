#pragma once

#include <memory>
#include <vector>
#include <array>
#include <string>

namespace buma::util { class InputLayoutDesc; }

namespace buma
{


namespace shader
{

struct SIGNATURE_PARAMETER_DESC;
std::unique_ptr<util::InputLayoutDesc> CreateInputLayoutDesc(const std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>& _descs);

#pragma region definitions

enum PRIMITIVE_TOPOLOGY
{
      PRIMITIVE_TOPOLOGY_UNDEFINED                   // D3D_PRIMITIVE_TOPOLOGY_UNDEFINED                  = 0 
    , PRIMITIVE_TOPOLOGY_POINT_LIST                  // D3D_PRIMITIVE_TOPOLOGY_POINTLIST                  = 1 
    , PRIMITIVE_TOPOLOGY_LINE_LIST                   // D3D_PRIMITIVE_TOPOLOGY_LINELIST                   = 2 
    , PRIMITIVE_TOPOLOGY_LINE_STRIP                  // D3D_PRIMITIVE_TOPOLOGY_LINESTRIP                  = 3 
    , PRIMITIVE_TOPOLOGY_TRIANGLE_LIST               // D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST               = 4 
    , PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP              // D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP              = 5 
    , PRIMITIVE_TOPOLOGY_LINE_LIST_ADJ               // D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ               = 10
    , PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJ              // D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ              = 11
    , PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJ           // D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ           = 12
    , PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJ          // D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ          = 13
    , PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST  = 33
    , PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST  = 34
    , PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST  = 35
    , PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST  = 36
    , PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST  = 37
    , PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST  = 38
    , PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST  = 39
    , PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST  = 40
    , PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCH_LIST  // D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST  = 41
    , PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST = 42
    , PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST = 43
    , PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST = 44
    , PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST = 45
    , PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST = 46
    , PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST = 47
    , PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST = 48
    , PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST = 49
    , PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST = 50
    , PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST = 51
    , PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST = 52
    , PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST = 53
    , PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST = 54
    , PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST = 55
    , PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST = 56
    , PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST = 57
    , PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST = 58
    , PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST = 59
    , PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST = 60
    , PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST = 61
    , PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST = 62
    , PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST = 63
    , PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCH_LIST // D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST = 64
};

enum PRIMITIVE_TYPE
{
      PRIMITIVE_TYPE_UNDEFINED              // D3D_PRIMITIVE_UNDEFINED                 = 0
    , PRIMITIVE_TYPE_POINT                  // D3D_PRIMITIVE_POINT                     = 1
    , PRIMITIVE_TYPE_LINE                   // D3D_PRIMITIVE_LINE                      = 2
    , PRIMITIVE_TYPE_TRIANGLE               // D3D_PRIMITIVE_TRIANGLE                  = 3
    , PRIMITIVE_TYPE_LINE_ADJ               // D3D_PRIMITIVE_LINE_ADJ                  = 6
    , PRIMITIVE_TYPE_TRIANGLE_ADJ           // D3D_PRIMITIVE_TRIANGLE_ADJ              = 7
    , PRIMITIVE_TYPE_1_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_1_CONTROL_POINT_PATCH     = 8
    , PRIMITIVE_TYPE_2_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_2_CONTROL_POINT_PATCH     = 9
    , PRIMITIVE_TYPE_3_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_3_CONTROL_POINT_PATCH     = 10
    , PRIMITIVE_TYPE_4_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_4_CONTROL_POINT_PATCH     = 11
    , PRIMITIVE_TYPE_5_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_5_CONTROL_POINT_PATCH     = 12
    , PRIMITIVE_TYPE_6_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_6_CONTROL_POINT_PATCH     = 13
    , PRIMITIVE_TYPE_7_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_7_CONTROL_POINT_PATCH     = 14
    , PRIMITIVE_TYPE_8_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_8_CONTROL_POINT_PATCH     = 15
    , PRIMITIVE_TYPE_9_CONTROL_POINT_PATCH  // D3D_PRIMITIVE_9_CONTROL_POINT_PATCH     = 16
    , PRIMITIVE_TYPE_10_CONTROL_POINT_PATCH // D3D_PRIMITIVE_10_CONTROL_POINT_PATCH    = 17
    , PRIMITIVE_TYPE_11_CONTROL_POINT_PATCH // D3D_PRIMITIVE_11_CONTROL_POINT_PATCH    = 18
    , PRIMITIVE_TYPE_12_CONTROL_POINT_PATCH // D3D_PRIMITIVE_12_CONTROL_POINT_PATCH    = 19
    , PRIMITIVE_TYPE_13_CONTROL_POINT_PATCH // D3D_PRIMITIVE_13_CONTROL_POINT_PATCH    = 20
    , PRIMITIVE_TYPE_14_CONTROL_POINT_PATCH // D3D_PRIMITIVE_14_CONTROL_POINT_PATCH    = 21
    , PRIMITIVE_TYPE_15_CONTROL_POINT_PATCH // D3D_PRIMITIVE_15_CONTROL_POINT_PATCH    = 22
    , PRIMITIVE_TYPE_16_CONTROL_POINT_PATCH // D3D_PRIMITIVE_16_CONTROL_POINT_PATCH    = 23
    , PRIMITIVE_TYPE_17_CONTROL_POINT_PATCH // D3D_PRIMITIVE_17_CONTROL_POINT_PATCH    = 24
    , PRIMITIVE_TYPE_18_CONTROL_POINT_PATCH // D3D_PRIMITIVE_18_CONTROL_POINT_PATCH    = 25
    , PRIMITIVE_TYPE_19_CONTROL_POINT_PATCH // D3D_PRIMITIVE_19_CONTROL_POINT_PATCH    = 26
    , PRIMITIVE_TYPE_20_CONTROL_POINT_PATCH // D3D_PRIMITIVE_20_CONTROL_POINT_PATCH    = 27
    , PRIMITIVE_TYPE_21_CONTROL_POINT_PATCH // D3D_PRIMITIVE_21_CONTROL_POINT_PATCH    = 28
    , PRIMITIVE_TYPE_22_CONTROL_POINT_PATCH // D3D_PRIMITIVE_22_CONTROL_POINT_PATCH    = 29
    , PRIMITIVE_TYPE_23_CONTROL_POINT_PATCH // D3D_PRIMITIVE_23_CONTROL_POINT_PATCH    = 30
    , PRIMITIVE_TYPE_24_CONTROL_POINT_PATCH // D3D_PRIMITIVE_24_CONTROL_POINT_PATCH    = 31
    , PRIMITIVE_TYPE_25_CONTROL_POINT_PATCH // D3D_PRIMITIVE_25_CONTROL_POINT_PATCH    = 32
    , PRIMITIVE_TYPE_26_CONTROL_POINT_PATCH // D3D_PRIMITIVE_26_CONTROL_POINT_PATCH    = 33
    , PRIMITIVE_TYPE_27_CONTROL_POINT_PATCH // D3D_PRIMITIVE_27_CONTROL_POINT_PATCH    = 34
    , PRIMITIVE_TYPE_28_CONTROL_POINT_PATCH // D3D_PRIMITIVE_28_CONTROL_POINT_PATCH    = 35
    , PRIMITIVE_TYPE_29_CONTROL_POINT_PATCH // D3D_PRIMITIVE_29_CONTROL_POINT_PATCH    = 36
    , PRIMITIVE_TYPE_30_CONTROL_POINT_PATCH // D3D_PRIMITIVE_30_CONTROL_POINT_PATCH    = 37
    , PRIMITIVE_TYPE_31_CONTROL_POINT_PATCH // D3D_PRIMITIVE_31_CONTROL_POINT_PATCH    = 38
    , PRIMITIVE_TYPE_32_CONTROL_POINT_PATCH // D3D_PRIMITIVE_32_CONTROL_POINT_PATCH    = 39
};

enum TESSELLATOR_OUTPUT_PRIMITIVE
{
      TESSELLATOR_OUTPUT_PRIMITIVE_UNDEFINED                  // D3D_TESSELLATOR_OUTPUT_UNDEFINED     = 0
    , TESSELLATOR_OUTPUT_PRIMITIVE_POINT                      // D3D_TESSELLATOR_OUTPUT_POINT         = 1
    , TESSELLATOR_OUTPUT_PRIMITIVE_LINE                       // D3D_TESSELLATOR_OUTPUT_LINE          = 2
    , TESSELLATOR_OUTPUT_PRIMITIVE_TRIANGLE_CLOCKWISE         // D3D_TESSELLATOR_OUTPUT_TRIANGLE_CW   = 3
    , TESSELLATOR_OUTPUT_PRIMITIVE_TRIANGLE_COUNTER_CLOCKWISE // D3D_TESSELLATOR_OUTPUT_TRIANGLE_CCW  = 4
};

enum TESSELLATOR_DOMAIN
{
      TESSELLATOR_DOMAIN_UNDEFINED  // D3D_TESSELLATOR_DOMAIN_UNDEFINED  = 0
    , TESSELLATOR_DOMAIN_ISOLINE    // D3D_TESSELLATOR_DOMAIN_ISOLINE    = 1
    , TESSELLATOR_DOMAIN_TRI        // D3D_TESSELLATOR_DOMAIN_TRI        = 2
    , TESSELLATOR_DOMAIN_QUAD       // D3D_TESSELLATOR_DOMAIN_QUAD       = 3
};

enum TESSELLATOR_PARTITIONING
{
      TESSELLATOR_PARTITIONING_UNDEFINED        // D3D_TESSELLATOR_PARTITIONING_UNDEFINED       = 0
    , TESSELLATOR_PARTITIONING_INTEGER          // D3D_TESSELLATOR_PARTITIONING_INTEGER         = 1
    , TESSELLATOR_PARTITIONING_POW2             // D3D_TESSELLATOR_PARTITIONING_POW2            = 2
    , TESSELLATOR_PARTITIONING_FRACTIONAL_ODD   // D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD  = 3
    , TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN  // D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN = 4
};

enum SHADER_INPUT_FLAG : uint32_t
{
      SHADER_INPUT_FLAG_UNDEFINED           = 0x0
    , SHADER_INPUT_FLAG_USERPACKED          = 0x1  // D3D_SIF_USERPACKED
    , SHADER_INPUT_FLAG_COMPARISON_SAMPLER  = 0x2  // D3D_SIF_COMPARISON_SAMPLER
    , SHADER_INPUT_FLAG_TEXTURE_COMPONENT_0 = 0x4  // D3D_SIF_TEXTURE_COMPONENT_0
    , SHADER_INPUT_FLAG_TEXTURE_COMPONENT_1 = 0x8  // D3D_SIF_TEXTURE_COMPONENT_1
    , SHADER_INPUT_FLAG_TEXTURE_COMPONENTS  = 0xc  // D3D_SIF_TEXTURE_COMPONENTS
    , SHADER_INPUT_FLAG_UNUSED              = 0x10 // D3D_SIF_UNUSED
};
using SHADER_INPUT_FLAGS = uint32_t;

enum RESOURCE_RETURN_TYPE
{
      RESOURCE_RETURN_TYPE_UNDEFINED
    , RESOURCE_RETURN_TYPE_UNORM     // D3D_RETURN_TYPE_UNORM       = 1
    , RESOURCE_RETURN_TYPE_SNORM     // D3D_RETURN_TYPE_SNORM       = 2
    , RESOURCE_RETURN_TYPE_SINT      // D3D_RETURN_TYPE_SINT        = 3
    , RESOURCE_RETURN_TYPE_UINT      // D3D_RETURN_TYPE_UINT        = 4
    , RESOURCE_RETURN_TYPE_FLOAT     // D3D_RETURN_TYPE_FLOAT       = 5
    , RESOURCE_RETURN_TYPE_MIXED     // D3D_RETURN_TYPE_MIXED       = 6
    , RESOURCE_RETURN_TYPE_DOUBLE    // D3D_RETURN_TYPE_DOUBLE      = 7
    , RESOURCE_RETURN_TYPE_CONTINUED // D3D_RETURN_TYPE_CONTINUED   = 8
};

enum SRV_DIMENSION
{
      SRV_DIMENSION_UNKNOWN             // D3D_SRV_DIMENSION_UNKNOWN             = 0
    , SRV_DIMENSION_BUFFER              // D3D_SRV_DIMENSION_BUFFER              = 1
    , SRV_DIMENSION_TEXTURE1D           // D3D_SRV_DIMENSION_TEXTURE1D           = 2
    , SRV_DIMENSION_TEXTURE1DARRAY      // D3D_SRV_DIMENSION_TEXTURE1DARRAY      = 3
    , SRV_DIMENSION_TEXTURE2D           // D3D_SRV_DIMENSION_TEXTURE2D           = 4
    , SRV_DIMENSION_TEXTURE2DARRAY      // D3D_SRV_DIMENSION_TEXTURE2DARRAY      = 5
    , SRV_DIMENSION_TEXTURE2DMS         // D3D_SRV_DIMENSION_TEXTURE2DMS         = 6
    , SRV_DIMENSION_TEXTURE2DMSARRAY    // D3D_SRV_DIMENSION_TEXTURE2DMSARRAY    = 7
    , SRV_DIMENSION_TEXTURE3D           // D3D_SRV_DIMENSION_TEXTURE3D           = 8
    , SRV_DIMENSION_TEXTURECUBE         // D3D_SRV_DIMENSION_TEXTURECUBE         = 9
    , SRV_DIMENSION_TEXTURECUBEARRAY    // D3D_SRV_DIMENSION_TEXTURECUBEARRAY    = 10
    , SRV_DIMENSION_BUFFEREX            // D3D_SRV_DIMENSION_BUFFEREX            = 11
};

enum CBUFFER_TYPE
{
      CBUFFER_TYPE_CBUFFER             // D3D_CT_CBUFFER            = 0
    , CBUFFER_TYPE_TBUFFER             // D3D_CT_TBUFFER            = ( D3D_CT_CBUFFER + 1 )
    , CBUFFER_TYPE_INTERFACE_POINTERS  // D3D_CT_INTERFACE_POINTERS = ( D3D_CT_TBUFFER + 1 )
    , CBUFFER_TYPE_RESOURCE_BIND_INFO  // D3D_CT_RESOURCE_BIND_INFO = ( D3D_CT_INTERFACE_POINTERS + 1 )

    , CBUFFER_TYPE_UNKNOWN
};

enum SHADER_VARIABLE_CLASS : uint32_t
{
      SHADER_VARIABLE_CLASS_SCALAR              // D3D_SVC_SCALAR                 = 0                               
    , SHADER_VARIABLE_CLASS_VECTOR              // D3D_SVC_VECTOR                 = ( D3D_SVC_SCALAR + 1 )          
    , SHADER_VARIABLE_CLASS_MATRIX_ROWS         // D3D_SVC_MATRIX_ROWS            = ( D3D_SVC_VECTOR + 1 )          
    , SHADER_VARIABLE_CLASS_MATRIX_COLUMNS      // D3D_SVC_MATRIX_COLUMNS         = ( D3D_SVC_MATRIX_ROWS + 1 )     
    , SHADER_VARIABLE_CLASS_OBJECT              // D3D_SVC_OBJECT                 = ( D3D_SVC_MATRIX_COLUMNS + 1 )  
    , SHADER_VARIABLE_CLASS_STRUCT              // D3D_SVC_STRUCT                 = ( D3D_SVC_OBJECT + 1 )          
    , SHADER_VARIABLE_CLASS_INTERFACE_CLASS     // D3D_SVC_INTERFACE_CLASS        = ( D3D_SVC_STRUCT + 1 )          
    , SHADER_VARIABLE_CLASS_INTERFACE_POINTER   // D3D_SVC_INTERFACE_POINTER      = ( D3D_SVC_INTERFACE_CLASS + 1 )

    , SHADER_VARIABLE_CLASS_UNDEFINED
};

enum SHADER_VARIABLE_TYPE : uint32_t
{
      SHADER_VARIABLE_TYPE_VOID                          // D3D_SVT_VOID                           = 0,
    , SHADER_VARIABLE_TYPE_BOOL                          // D3D_SVT_BOOL                           = 1,
    , SHADER_VARIABLE_TYPE_INT                           // D3D_SVT_INT                            = 2,
    , SHADER_VARIABLE_TYPE_FLOAT                         // D3D_SVT_FLOAT                          = 3,
    , SHADER_VARIABLE_TYPE_STRING                        // D3D_SVT_STRING                         = 4,
    , SHADER_VARIABLE_TYPE_TEXTURE                       // D3D_SVT_TEXTURE                        = 5,
    , SHADER_VARIABLE_TYPE_TEXTURE1D                     // D3D_SVT_TEXTURE1D                      = 6,
    , SHADER_VARIABLE_TYPE_TEXTURE2D                     // D3D_SVT_TEXTURE2D                      = 7,
    , SHADER_VARIABLE_TYPE_TEXTURE3D                     // D3D_SVT_TEXTURE3D                      = 8,
    , SHADER_VARIABLE_TYPE_TEXTURECUBE                   // D3D_SVT_TEXTURECUBE                    = 9,
    , SHADER_VARIABLE_TYPE_SAMPLER                       // D3D_SVT_SAMPLER                        = 10,
    , SHADER_VARIABLE_TYPE_SAMPLER1D                     // D3D_SVT_SAMPLER1D                      = 11,
    , SHADER_VARIABLE_TYPE_SAMPLER2D                     // D3D_SVT_SAMPLER2D                      = 12,
    , SHADER_VARIABLE_TYPE_SAMPLER3D                     // D3D_SVT_SAMPLER3D                      = 13,
    , SHADER_VARIABLE_TYPE_SAMPLERCUBE                   // D3D_SVT_SAMPLERCUBE                    = 14,
    , SHADER_VARIABLE_TYPE_PIXELSHADER                   // D3D_SVT_PIXELSHADER                    = 15,
    , SHADER_VARIABLE_TYPE_VERTEXSHADER                  // D3D_SVT_VERTEXSHADER                   = 16,
    , SHADER_VARIABLE_TYPE_PIXELFRAGMENT                 // D3D_SVT_PIXELFRAGMENT                  = 17,
    , SHADER_VARIABLE_TYPE_VERTEXFRAGMENT                // D3D_SVT_VERTEXFRAGMENT                 = 18,
    , SHADER_VARIABLE_TYPE_UINT                          // D3D_SVT_UINT                           = 19,
    , SHADER_VARIABLE_TYPE_UINT8                         // D3D_SVT_UINT8                          = 20,
    , SHADER_VARIABLE_TYPE_GEOMETRYSHADER                // D3D_SVT_GEOMETRYSHADER                 = 21,
    , SHADER_VARIABLE_TYPE_RASTERIZER                    // D3D_SVT_RASTERIZER                     = 22,
    , SHADER_VARIABLE_TYPE_DEPTHSTENCIL                  // D3D_SVT_DEPTHSTENCIL                   = 23,
    , SHADER_VARIABLE_TYPE_BLEND                         // D3D_SVT_BLEND                          = 24,
    , SHADER_VARIABLE_TYPE_BUFFER                        // D3D_SVT_BUFFER                         = 25,
    , SHADER_VARIABLE_TYPE_CBUFFER                       // D3D_SVT_CBUFFER                        = 26,
    , SHADER_VARIABLE_TYPE_TBUFFER                       // D3D_SVT_TBUFFER                        = 27,
    , SHADER_VARIABLE_TYPE_TEXTURE1DARRAY                // D3D_SVT_TEXTURE1DARRAY                 = 28,
    , SHADER_VARIABLE_TYPE_TEXTURE2DARRAY                // D3D_SVT_TEXTURE2DARRAY                 = 29,
    , SHADER_VARIABLE_TYPE_RENDERTARGETVIEW              // D3D_SVT_RENDERTARGETVIEW               = 30,
    , SHADER_VARIABLE_TYPE_DEPTHSTENCILVIEW              // D3D_SVT_DEPTHSTENCILVIEW               = 31,
    , SHADER_VARIABLE_TYPE_TEXTURE2DMS                   // D3D_SVT_TEXTURE2DMS                    = 32,
    , SHADER_VARIABLE_TYPE_TEXTURE2DMSARRAY              // D3D_SVT_TEXTURE2DMSARRAY               = 33,
    , SHADER_VARIABLE_TYPE_TEXTURECUBEARRAY              // D3D_SVT_TEXTURECUBEARRAY               = 34,
    , SHADER_VARIABLE_TYPE_HULLSHADER                    // D3D_SVT_HULLSHADER                     = 35,
    , SHADER_VARIABLE_TYPE_DOMAINSHADER                  // D3D_SVT_DOMAINSHADER                   = 36,
    , SHADER_VARIABLE_TYPE_INTERFACE_POINTER             // D3D_SVT_INTERFACE_POINTER              = 37,
    , SHADER_VARIABLE_TYPE_COMPUTESHADER                 // D3D_SVT_COMPUTESHADER                  = 38,
    , SHADER_VARIABLE_TYPE_DOUBLE                        // D3D_SVT_DOUBLE                         = 39,
    , SHADER_VARIABLE_TYPE_RWTEXTURE1D                   // D3D_SVT_RWTEXTURE1D                    = 40,
    , SHADER_VARIABLE_TYPE_RWTEXTURE1DARRAY              // D3D_SVT_RWTEXTURE1DARRAY               = 41,
    , SHADER_VARIABLE_TYPE_RWTEXTURE2D                   // D3D_SVT_RWTEXTURE2D                    = 42,
    , SHADER_VARIABLE_TYPE_RWTEXTURE2DARRAY              // D3D_SVT_RWTEXTURE2DARRAY               = 43,
    , SHADER_VARIABLE_TYPE_RWTEXTURE3D                   // D3D_SVT_RWTEXTURE3D                    = 44,
    , SHADER_VARIABLE_TYPE_RWBUFFER                      // D3D_SVT_RWBUFFER                       = 45,
    , SHADER_VARIABLE_TYPE_BYTEADDRESS_BUFFER            // D3D_SVT_BYTEADDRESS_BUFFER             = 46,
    , SHADER_VARIABLE_TYPE_RWBYTEADDRESS_BUFFER          // D3D_SVT_RWBYTEADDRESS_BUFFER           = 47,
    , SHADER_VARIABLE_TYPE_STRUCTURED_BUFFER             // D3D_SVT_STRUCTURED_BUFFER              = 48,
    , SHADER_VARIABLE_TYPE_RWSTRUCTURED_BUFFER           // D3D_SVT_RWSTRUCTURED_BUFFER            = 49,
    , SHADER_VARIABLE_TYPE_APPEND_STRUCTURED_BUFFER      // D3D_SVT_APPEND_STRUCTURED_BUFFER       = 50,
    , SHADER_VARIABLE_TYPE_CONSUME_STRUCTURED_BUFFER     // D3D_SVT_CONSUME_STRUCTURED_BUFFER      = 51,
    , SHADER_VARIABLE_TYPE_MIN8FLOAT                     // D3D_SVT_MIN8FLOAT                      = 52,
    , SHADER_VARIABLE_TYPE_MIN10FLOAT                    // D3D_SVT_MIN10FLOAT                     = 53,
    , SHADER_VARIABLE_TYPE_MIN16FLOAT                    // D3D_SVT_MIN16FLOAT                     = 54,
    , SHADER_VARIABLE_TYPE_MIN12INT                      // D3D_SVT_MIN12INT                       = 55,
    , SHADER_VARIABLE_TYPE_MIN16INT                      // D3D_SVT_MIN16INT                       = 56,
    , SHADER_VARIABLE_TYPE_MIN16UINT                     // D3D_SVT_MIN16UINT                      = 57,

    , SHADER_VARIABLE_TYPE_UNDEFINED
};


enum SHADER_REQUIRE_FLAG : uint64_t
{
      SHADER_REQUIRE_FLAG_NONE                              = 0x0
    , SHADER_REQUIRE_FLAG_DOUBLES                           = 0x1    // D3D_SHADER_REQUIRES_DOUBLES
    , SHADER_REQUIRE_FLAG_EARLY_DEPTH_STENCIL               = 0x2    // D3D_SHADER_REQUIRES_EARLY_DEPTH_STENCIL
    , SHADER_REQUIRE_FLAG_UAVS_AT_EVERY_STAGE               = 0x4    // D3D_SHADER_REQUIRES_UAVS_AT_EVERY_STAGE
    , SHADER_REQUIRE_FLAG_64UAVS                            = 0x8    // D3D_SHADER_REQUIRES_64_UAVS
    , SHADER_REQUIRE_FLAG_MINIMUM_PRECISION                 = 0x10   // D3D_SHADER_REQUIRES_MINIMUM_PRECISION
    , SHADER_REQUIRE_FLAG_11_1_DOUBLE_EXTENSIONS            = 0x20   // D3D_SHADER_REQUIRES_11_1_DOUBLE_EXTENSIONS
    , SHADER_REQUIRE_FLAG_11_1_SHADER_EXTENSIONS            = 0x40   // D3D_SHADER_REQUIRES_11_1_SHADER_EXTENSIONS
    , SHADER_REQUIRE_FLAG_LEVEL9_COMPARISON_FILTERING       = 0x80   // D3D_SHADER_REQUIRES_LEVEL_9_COMPARISON_FILTERING
    , SHADER_REQUIRE_FLAG_TILED_RESOURCES                   = 0x100  // D3D_SHADER_REQUIRES_TILED_RESOURCES
    , SHADER_REQUIRE_FLAG_STENCIL_REF                       = 0x200  // D3D_SHADER_REQUIRES_STENCIL_REF
    , SHADER_REQUIRE_FLAG_INNER_COVERAGE                    = 0x400  // D3D_SHADER_REQUIRES_INNER_COVERAGE
    , SHADER_REQUIRE_FLAG_TYPED_UAV_LOAD_ADDITIONAL_FORMATS = 0x800  // D3D_SHADER_REQUIRES_TYPED_UAV_LOAD_ADDITIONAL_FORMATS
    , SHADER_REQUIRE_FLAG_ROVS                              = 0x1000 // D3D_SHADER_REQUIRES_ROVS
    , SHADER_REQUIRE_FLAG_ARRAY_INDEX_FROM_ANY_SHADER       = 0x2000 // D3D_SHADER_REQUIRES_VIEWPORT_AND_RT_ARRAY_INDEX_FROM_ANY_SHADER_FEEDING_RASTERIZER // シェーダーフィーディングラスタライザーからのビューポートとRT配列インデックス
};
using SHADER_REQUIRE_FLAGS = uint64_t;

enum SHADER_VARIABLE_FLAG : uint32_t
{
      SHADER_VARIABLE_FLAG_NONE                 = 0x0
    , SHADER_VARIABLE_FLAG_USERPACKED           = 0x1 // D3D_SVF_USERPACKED
    , SHADER_VARIABLE_FLAG_USED                 = 0x2 // D3D_SVF_USED
    , SHADER_VARIABLE_FLAG_INTERFACE_POINTER    = 0x4 // D3D_SVF_INTERFACE_POINTER
    , SHADER_VARIABLE_FLAG_INTERFACE_PARAMETER  = 0x8 // D3D_SVF_INTERFACE_PARAMETER
};
using SHADER_VARIABLE_FLAGS = uint32_t;

enum SHADER_INPUT_TYPE
{
      SHADER_INPUT_TYPE_CBUFFER                        // D3D_SIT_CBUFFER
    , SHADER_INPUT_TYPE_TBUFFER                        // D3D_SIT_TBUFFER
    , SHADER_INPUT_TYPE_TEXTURE                        // D3D_SIT_TEXTURE
    , SHADER_INPUT_TYPE_SAMPLER                        // D3D_SIT_SAMPLER
    , SHADER_INPUT_TYPE_UAV_RWTYPED                    // D3D_SIT_UAV_RWTYPED
    , SHADER_INPUT_TYPE_STRUCTURED                     // D3D_SIT_STRUCTURED
    , SHADER_INPUT_TYPE_UAV_RWSTRUCTURED               // D3D_SIT_UAV_RWSTRUCTURED
    , SHADER_INPUT_TYPE_BYTEADDRESS                    // D3D_SIT_BYTEADDRESS
    , SHADER_INPUT_TYPE_UAV_RWBYTEADDRESS              // D3D_SIT_UAV_RWBYTEADDRESS
    , SHADER_INPUT_TYPE_UAV_APPEND_STRUCTURED          // D3D_SIT_UAV_APPEND_STRUCTURED
    , SHADER_INPUT_TYPE_UAV_CONSUME_STRUCTURED         // D3D_SIT_UAV_CONSUME_STRUCTURED
    , SHADER_INPUT_TYPE_UAV_RWSTRUCTURED_WITH_COUNTER  // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
    , SHADER_INPUT_TYPE_RTACCELERATIONSTRUCTURE        // D3D_SIT_RTACCELERATIONSTRUCTURE
    , SHADER_INPUT_TYPE_UAV_FEEDBACKTEXTURE            // D3D_SIT_UAV_FEEDBACKTEXTURE

    , SHADER_INPUT_TYPE_UNKNOWN
};

enum REGISTER_COMPONENT_TYPE
{
      REGISTER_COMPONENT_TYPE_UNKNOWN     // D3D_REGISTER_COMPONENT_UNKNOWN = 0
    , REGISTER_COMPONENT_TYPE_UINT32      // D3D_REGISTER_COMPONENT_UINT32  = 1
    , REGISTER_COMPONENT_TYPE_SINT32      // D3D_REGISTER_COMPONENT_SINT32  = 2
    , REGISTER_COMPONENT_TYPE_FLOAT32     // D3D_REGISTER_COMPONENT_FLOAT32 = 3
};

enum SYSTEM_VALUE_NAME
{
      SYSTEM_VALUE_NAME_UNDEFINED                                                                    // D3D_NAME_UNDEFINED                     = 0,
    , SYSTEM_VALUE_NAME_POSITION                           // SV_Position                            // D3D_NAME_POSITION                      = 1, 
    , SYSTEM_VALUE_NAME_CLIP_DISTANCE                      // SV_ClipDistance                        // D3D_NAME_CLIP_DISTANCE                 = 2,
    , SYSTEM_VALUE_NAME_CULL_DISTANCE                      // SV_CullDistance                        // D3D_NAME_CULL_DISTANCE                 = 3,
    , SYSTEM_VALUE_NAME_RENDER_TARGET_ARRAY_INDEX          // SV_RenderTargetArrayIndex              // D3D_NAME_RENDER_TARGET_ARRAY_INDEX     = 4,
    , SYSTEM_VALUE_NAME_VIEWPORT_ARRAY_INDEX               // SV_ViewportArrayIndex                  // D3D_NAME_VIEWPORT_ARRAY_INDEX          = 5,
    , SYSTEM_VALUE_NAME_VERTEX_ID                          // SV_VertexID                            // D3D_NAME_VERTEX_ID                     = 6,
    , SYSTEM_VALUE_NAME_PRIMITIVE_ID                       // SV_PrimitiveID                         // D3D_NAME_PRIMITIVE_ID                  = 7,
    , SYSTEM_VALUE_NAME_INSTANCE_ID                        // SV_InstanceID                          // D3D_NAME_INSTANCE_ID                   = 8,
    , SYSTEM_VALUE_NAME_IS_FRONT_FACE                      // SV_IsFrontFace                         // D3D_NAME_IS_FRONT_FACE                 = 9,
    , SYSTEM_VALUE_NAME_SAMPLE_INDEX                       // SV_SampleIndex                         // D3D_NAME_SAMPLE_INDEX                  = 10,
    , SYSTEM_VALUE_NAME_FINAL_QUAD_EDGE_TESSFACTOR         // SV_FinalQuadEdgeTessfactor             // D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR    = 11,
    , SYSTEM_VALUE_NAME_FINAL_QUAD_INSIDE_TESSFACTOR       // SV_FinalQuadInsideTessfactor           // D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR  = 12,
    , SYSTEM_VALUE_NAME_FINAL_TRI_EDGE_TESSFACTOR          // SV_FinalTriEdgeTessfactor              // D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR     = 13,
    , SYSTEM_VALUE_NAME_FINAL_TRI_INSIDE_TESSFACTOR        // SV_FinalTriInsideTessfactor            // D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR   = 14,
    , SYSTEM_VALUE_NAME_FINAL_LINE_DETAIL_TESSFACTOR       // SV_FinalLineDetailTessfactor           // D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR  = 15,
    , SYSTEM_VALUE_NAME_FINAL_LINE_DENSITY_TESSFACTOR      // SV_FinalLineDensityTessfactor          // D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR = 16,
    , SYSTEM_VALUE_NAME_BARYCENTRICS                       // SV_Barycentrics                        // D3D_NAME_BARYCENTRICS                  = 23, 
    , SYSTEM_VALUE_NAME_SHADING_RATE                       // SV_ShadingRate                         // D3D_NAME_SHADINGRATE                   = 24, 
    , SYSTEM_VALUE_NAME_CULL_PRIMITIVE                     // SV_CullPrimitive                       // D3D_NAME_CULLPRIMITIVE                 = 25, 
    , SYSTEM_VALUE_NAME_TARGET                             // SV_Target                              // D3D_NAME_TARGET                        = 64, 
    , SYSTEM_VALUE_NAME_DEPTH                              // SV_Depth                               // D3D_NAME_DEPTH                         = 65, 
    , SYSTEM_VALUE_NAME_COVERAGE                           // SV_Coverage                            // D3D_NAME_COVERAGE                      = 66, 
    , SYSTEM_VALUE_NAME_DEPTH_GREATER_EQUAL                // SV_DepthGreaterEqual                   // D3D_NAME_DEPTH_GREATER_EQUAL           = 67,
    , SYSTEM_VALUE_NAME_DEPTH_LESS_EQUAL                   // SV_DepthLessEqual                      // D3D_NAME_DEPTH_LESS_EQUAL              = 68,
    , SYSTEM_VALUE_NAME_STENCIL_REF                        // SV_StencilRef                          // D3D_NAME_STENCIL_REF                   = 69,
    , SYSTEM_VALUE_NAME_INNER_COVERAGE                     // SV_InnerCoverage                       // D3D_NAME_INNER_COVERAGE                = 70,
};

enum MIN_PRECISION
{
      MIN_PRECISION_DEFAULT      // D3D_MIN_PRECISION_DEFAULT      = 0,
    , MIN_PRECISION_FLOAT_16     // D3D_MIN_PRECISION_FLOAT_16     = 1,
    , MIN_PRECISION_FLOAT_2_8    // D3D_MIN_PRECISION_FLOAT_2_8    = 2,
    , MIN_PRECISION_SINT_16      // D3D_MIN_PRECISION_SINT_16      = 4,
    , MIN_PRECISION_UINT_16      // D3D_MIN_PRECISION_UINT_16      = 5,
    , MIN_PRECISION_ANY_16       // D3D_MIN_PRECISION_ANY_16       = 0xf0,
    , MIN_PRECISION_ANY_10       // D3D_MIN_PRECISION_ANY_10       = 0xf1

    , MIN_PRECISION_UNKNOWN      // D3D_MIN_PRECISION_RESERVED     = 3,
};

enum FEATURE_LEVEL
{
      FEATURE_LEVEL_D3D_1_0_CORE  = 0x1000  // D3D_FEATURE_LEVEL_1_0_CORE    = 0x1000,
    , FEATURE_LEVEL_D3D_9_1       = 0x9100  // D3D_FEATURE_LEVEL_9_1         = 0x9100,
    , FEATURE_LEVEL_D3D_9_2       = 0x9200  // D3D_FEATURE_LEVEL_9_2         = 0x9200,
    , FEATURE_LEVEL_D3D_9_3       = 0x9300  // D3D_FEATURE_LEVEL_9_3         = 0x9300,
    , FEATURE_LEVEL_D3D_10_0      = 0xa000  // D3D_FEATURE_LEVEL_10_0        = 0xa000,
    , FEATURE_LEVEL_D3D_10_1      = 0xa100  // D3D_FEATURE_LEVEL_10_1        = 0xa100,
    , FEATURE_LEVEL_D3D_11_0      = 0xb000  // D3D_FEATURE_LEVEL_11_0        = 0xb000,
    , FEATURE_LEVEL_D3D_11_1      = 0xb100  // D3D_FEATURE_LEVEL_11_1        = 0xb100,
    , FEATURE_LEVEL_D3D_12_0      = 0xc000  // D3D_FEATURE_LEVEL_12_0        = 0xc000,
    , FEATURE_LEVEL_D3D_12_1      = 0xc100  // D3D_FEATURE_LEVEL_12_1        = 0xc100
    , FEATURE_LEVEL_D3D_12_2      = 0xc200
};

enum INTERPOLATION_MODE
{
      INTERPOLATION_MODE_UNDEFINED                        // D3D_INTERPOLATION_UNDEFINED                        = 0,
    , INTERPOLATION_MODE_CONSTANT                         // D3D_INTERPOLATION_CONSTANT                         = 1,
    , INTERPOLATION_MODE_LINEAR                           // D3D_INTERPOLATION_LINEAR                           = 2,
    , INTERPOLATION_MODE_LINEAR_CENTROID                  // D3D_INTERPOLATION_LINEAR_CENTROID                  = 3,
    , INTERPOLATION_MODE_LINEAR_NOPERSPECTIVE             // D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE             = 4,
    , INTERPOLATION_MODE_LINEAR_NOPERSPECTIVE_CENTROID    // D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID    = 5,
    , INTERPOLATION_MODE_LINEAR_SAMPLE                    // D3D_INTERPOLATION_LINEAR_SAMPLE                    = 6,
    , INTERPOLATION_MODE_LINEAR_NOPERSPECTIVE_SAMPLE      // D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE_SAMPLE      = 7
};

enum PARAMETER_FLAG : uint32_t
{
      PARAMETER_FLAG_NONE = 0x0 // D3D_PF_NONE = 0,
    , PARAMETER_FLAG_IN   = 0x1 // D3D_PF_IN   = 0x1,
    , PARAMETER_FLAG_OUT  = 0x2 // D3D_PF_OUT  = 0x2,
};
using PARAMETER_FLAGS = uint32_t;

#pragma endregion definitions

#pragma region structures

struct SHADER_DESC
{
    uint32_t                            version;                        // シェーダーバージョン

    std::string                         creator;                        // 作成者
    uint32_t                            flags;                          // シェーダーのコンパイル / 解析フラグ (予約済みフラグ 2021/02/04現在)

    uint32_t                            constant_buffers;               // 定数バッファーの数
    uint32_t                            bound_resources;                // バインドされたリソースの数
    uint32_t                            input_parameters;               // 入力シグネチャのパラメーターの数
    uint32_t                            output_parameters;              // 出力シグネチャのパラメーターの数
    uint32_t                            instruction_count;              // 発行された命令の数
    uint32_t                            temp_register_count;            // 使用される一時レジスタの数 
    uint32_t                            temp_array_count;               // 使用されている一時配列の数
    uint32_t                            def_count;                      // 定数定義の数
    uint32_t                            dcl_count;                      // 宣言の数（入力出力）
    uint32_t                            texture_normal_instructions;    // 分類されていないテクスチャ命令の数
    uint32_t                            texture_load_instructions;      // テクスチャのロード命令の数
    uint32_t                            texture_comp_instructions;      // テクスチャ比較命令の数
    uint32_t                            texture_bias_instructions;      // テクスチャバイアス命令の数
    uint32_t                            texture_gradient_instructions;  // テクスチャグラデーション命令の数
    uint32_t                            float_instruction_count;        // 使用された浮動小数点演算命令の数
    uint32_t                            int_instruction_count;          // 使用される符号付き整数算術命令の数
    uint32_t                            uint_instruction_count;         // 使用される符号なし整数算術命令の数
    uint32_t                            static_flow_control_count;      // 使用された静的フロー制御命令の数
    uint32_t                            dynamic_flow_control_count;     // 使用される動的フロー制御命令の数
    uint32_t                            macro_instruction_count;        // 使用されたマクロ命令の数
    uint32_t                            array_instruction_count;        // 使用された配列命令の数
    uint32_t                            cut_instruction_count;          // 使用された切断指示の数
    uint32_t                            emit_instruction_count;         // 使用された発行命令の数
    PRIMITIVE_TOPOLOGY                  gs_output_topology;             // ジオメトリシェーダーの出力トポロジ
    uint32_t                            gs_max_output_vertex_count;     // ジオメトリシェーダーの最大出力頂点数
    PRIMITIVE_TYPE                      input_primitive;                // GS / HS入力プリミティブ
    uint32_t                            patch_constant_parameters;      // パッチ定数シグネチャのパラメーターの数
    uint32_t                            gs_instance_count;              // Geometryシェーダーインスタンスの数
    uint32_t                            control_points;                 // HS->DSステージの制御点の数
    TESSELLATOR_OUTPUT_PRIMITIVE        hs_output_primitive;            // テッセレータによるプリミティブ出力
    TESSELLATOR_PARTITIONING            hs_partitioning;                // テッセレータの分割モード
    TESSELLATOR_DOMAIN                  tessellator_domain;             // テッセレータのドメイン(quad, tri, isoline)

    // インストラクションカウント
    uint32_t                            barrier_instructions;           // 計算シェーダーのgroupbarrier命令の数
    uint32_t                            interlocked_instructions;       // interlocked命令の数
    uint32_t                            texture_store_instructions;     // テクスチャ書き込みの数
};

struct SHADER_INPUT_BIND_DESC
{
    std::string                         name;                           // リソースの名前
    SHADER_INPUT_TYPE                   shader_input_type;              // リソースの型（例：テクスチャ、cbufferなど）
    uint32_t                            start_bind_point;               // バインドポイントの開始点
    uint32_t                            bind_count;                     // 連続したバインドポイントの数（配列の場合）

    SHADER_INPUT_FLAGS                  binding_flags;                  // 入力バインディングフラグ
    RESOURCE_RETURN_TYPE                return_type;                    // 戻り値タイプ（テクスチャの場合）
    SRV_DIMENSION                       dimension;                      // ディメンション (テクスチャの場合)
    uint32_t                            num_samples;                    // サンプル数（MSテクスチャでない場合は0）
    uint32_t                            register_space;                 // Register space
    uint32_t                            range_id;                       // バイトコードの範囲ID
};

struct SHADER_VARIABLE_DESC
{
    std::string                         name;                           // 変数の名前
    uint32_t                            start_offset;                   // 定数バッファのバッキングストアのオフセット
    uint32_t                            variable_size;                  // 変数のサイズ（バイト単位）
    SHADER_VARIABLE_FLAGS               variable_flags;                 // 変数フラグ
    std::vector<uint8_t>                default_value;                  // デフォルト値のデータ
    uint32_t                            start_texture;                  // 最初のテクスチャインデックス（またはテクスチャが使用されていない場合は-1）
    uint32_t                            texture_size;                   // 使用される可能性のあるテクスチャスロットの数
    uint32_t                            start_sampler;                  // 最初のサンプラーインデックス（またはテクスチャが使用されていない場合は-1）
    uint32_t                            sampler_size;                   // 使用される可能性のあるサンプラースロットの数
};

struct SHADER_BUFFER_DESC
{
    std::string                         name;                           // 定数バッファーの名前
    CBUFFER_TYPE                        cb_type;                        // バッファコンテンツのタイプを示します
    uint32_t                            num_variables;                  // メンバー変数の数
    uint32_t                            size_of_cb;                     // CBのサイズ(バイト単位)
    uint32_t                            buffer_desc_flags;              // Buffer description flags (予約済みフラグ 2021/02/04現在)
};

struct SHADER_TYPE_DESC
{
    SHADER_VARIABLE_CLASS               variable_class;                 // 変数クラス(オブジェクト、マトリックスなど)
    SHADER_VARIABLE_TYPE                variable_type;                  // 変数の型(float, Samplerなど)
    uint32_t                            num_rows;                       // 行数(行列の場合、他の数値の場合は1、該当しない場合は0)
    uint32_t                            num_columns;                    // 列数(ベクトルと行列の場合、他の数値の場合は1、該当しない場合は0)
    uint32_t                            num_elements;                   // 要素の数(配列でない場合は0)
    uint32_t                            num_members;                    // メンバーの数(structでない場合は0)
    uint32_t                            structure_offset;               // structの開始からのオフセット(構造体のメンバーでない場合は0)
    std::string                         type_name;                      // 型の名前, can be NULL
};

struct SIGNATURE_PARAMETER_DESC
{
    std::string                         semantic_name;                  // セマンティック名
    uint32_t                            semantic_index;                 // セマンティックインデックス
    uint32_t                            num_register;                   // メンバ変数の数
    SYSTEM_VALUE_NAME                   system_value_type;              // 定義済みのシステム値、または該当しない場合はD3D_NAME_UNDEFINED
    REGISTER_COMPONENT_TYPE             component_type;                 // スカラー型(uint、floatなど)
    uint8_t                             mask;                           // レジスタのどのコンポーネントが使用されているかを示すマスク(D3D10_COMPONENT_MASK値の組み合わせ)
    uint8_t                             read_write_mask;                // 特定のコンポーネントが書き込まれない(これが出力シグネチャの場合)か、常に読み取られる(これが入力シグネチャの場合)かを示すマスク(D3D_MASK_*値の組み合わせ)
    uint32_t                            stream_index;                   // ストリームのインデックス
    MIN_PRECISION                       min_precision;                  // 最低限必要な補間精度
};

struct LIBRARY_DESC
{
    std::string                         creator;                        // ライブラリの発信者の名前
    uint32_t                            compilation_flags;              // 編集フラグ (予約済みフラグ 2021/02/04現在)
    uint32_t                            function_count;                 // ライブラリからエクスポートされた関数の数
};
struct FUNCTION_DESC
{
    uint32_t                            version;                        // シェーダーバージョン
    std::string                         creator;                        // 作成者ストリング
    uint32_t                            compilation_parse_flags;        // シェーダーのコンパイル / 解析フラグ (予約済みフラグ 2021/02/04現在)
    uint32_t                            constant_buffers;               // 定数バッファーの数
    uint32_t                            bound_resources;                // バインドされたリソースの数
    uint32_t                            instruction_count;              // 発行された命令の数
    uint32_t                            temp_register_count;            // 使用される一時レジスタの数 
    uint32_t                            temp_array_count;               // 使用される一時配列の数
    uint32_t                            def_count;                      // 定数定義の数
    uint32_t                            dcl_count;                      // 宣言の数（入出力）
    uint32_t                            texture_normal_instructions;    // 分類されていないテクスチャ命令の数
    uint32_t                            texture_load_instructions;      // テクスチャのロード命令の数
    uint32_t                            texture_comp_instructions;      // テクスチャ比較命令の数
    uint32_t                            texture_bias_instructions;      // テクスチャバイアス命令の数
    uint32_t                            texture_gradient_instructions;  // テクスチャグラデーション命令の数
    uint32_t                            float_instruction_count;        // 使用された浮動小数点演算命令の数
    uint32_t                            int_instruction_count;          // 使用される符号付き整数算術命令の数
    uint32_t                            uint_instruction_count;         // 使用される符号なし整数算術命令の数
    uint32_t                            static_flow_control_count;      // 使用された静的フロー制御命令の数
    uint32_t                            dynamic_flow_control_count;     // 使用される動的フロー制御命令の数
    uint32_t                            macro_instruction_count;        // 使用されたマクロ命令の数
    uint32_t                            array_instruction_count;        // 使用された配列命令の数
    uint32_t                            mov_instruction_count;          // 使用されたmov命令の数
    uint32_t                            movc_instruction_count;         // 使用されたmovc命令の数
    uint32_t                            conversion_instruction_count;   // 使用された型変換命令の数
    uint32_t                            bitwise_instruction_count;      // 使用されたビット演算命令の数
    FEATURE_LEVEL                       min_feature_level;              // 関数バイトコードの最小ターゲット
    SHADER_REQUIRE_FLAGS                required_feature_flags;         // 機能要求フラグ
    std::string                         name;                           // 関数名
    int                                 function_parameter_count;       // 関数シグネチャの論理パラメーターの数（戻り値を含まない）
    bool                                has_return;                     // 関数が値を返す場合TRUE, FALSE - サブルーチンです
    bool                                has10_level9_vertex_shader;     // 10L9 VS blobがある場合TRUE
    bool                                has10_level9_pixel_shader;      // 10L9 PS blobがある場合TRUE
};

struct PARAMETER_DESC
{
    std::string                         name;                           // パラメーター名
    std::string                         semantic_name;                  // パラメーターセマンティック名(+index)
    SHADER_VARIABLE_TYPE                variable_type;                  // エレメントタイプ
    SHADER_VARIABLE_CLASS               variable_class;                 // Scalar/Vector/Matrix.
    uint32_t                            rows;                           // 行数(matrixの場合に使用)
    uint32_t                            columns;                        // 列数(matrixの場合に使用)
    INTERPOLATION_MODE                  interpolation_mode;             // 補間モード
    PARAMETER_FLAGS                     parameter_flags;                // パラメータ修飾子
    uint32_t                            first_in_register;              // このパラメーターの最初の入力レジスター
    uint32_t                            first_in_component;             // このパラメータの最初の入力レジスタコンポーネント
    uint32_t                            first_out_register;             // このパラメーターの最初の出力レジスター
    uint32_t                            first_out_component;            // このパラメーターの最初の出力レジスタコンポーネント

};


#pragma endregion structures


class ShaderReflectionType
{
public:
    class Initialize;
public:
    ShaderReflectionType();
    ~ShaderReflectionType();

    const SHADER_TYPE_DESC&                      GetTypeDesc()       const { return type_desc; }
    const uint32_t                               GetNumInterfaces()  const { return num_interfaces; }
    const std::vector<ShaderReflectionType>&     GetMemberTypes()    const { return member_types; }
    const std::shared_ptr<ShaderReflectionType>& GetSubType()        const { return sub_type; }
    const std::shared_ptr<ShaderReflectionType>& GetBaseClass()      const { return base_class; }
    const std::vector<ShaderReflectionType>&     GetInterfaceTypes() const { return interface_types; }

private:
    SHADER_TYPE_DESC                            type_desc;
    uint32_t                                    num_interfaces;
    std::vector<ShaderReflectionType>           member_types;
    std::shared_ptr<ShaderReflectionType>       sub_type;
    std::shared_ptr<ShaderReflectionType>       base_class;
    std::vector<ShaderReflectionType>           interface_types;
};

class ShaderReflectionConstantBuffer;

class ShaderReflectionVariable
{
public:
    class Initialize;
public:
    ShaderReflectionVariable();
    ~ShaderReflectionVariable();

    const std::shared_ptr<const SHADER_VARIABLE_DESC>&           GetVariableDesc()             const { return variable_desc; }
    const std::shared_ptr<const ShaderReflectionConstantBuffer>& GetReflectionConstantBuffer() const { return reflection_constant_buffer; }
    const std::shared_ptr<const ShaderReflectionType>&           GetReflectionTypes()          const { return reflection_types; }
    const std::vector<uint32_t>&                                 GetInterfaceSlots()           const { return interface_slots; }

private:
    std::shared_ptr<SHADER_VARIABLE_DESC>           variable_desc;
    std::shared_ptr<ShaderReflectionConstantBuffer> reflection_constant_buffer;
    std::shared_ptr<ShaderReflectionType>           reflection_types;
    std::vector<uint32_t>                           interface_slots;

};

class ShaderReflectionConstantBuffer
{
public:
    class Initialize;
public:
    ShaderReflectionConstantBuffer();
    ~ShaderReflectionConstantBuffer();

    std::shared_ptr<const SHADER_BUFFER_DESC> GetShaderBUfferDesc() const { return desc; }

private:
    std::shared_ptr<SHADER_BUFFER_DESC> desc;

};

class ShaderReflection;
class ShaderDescData
{
    friend class ShaderReflection;
public:
    class Initialize;
public:
    ShaderDescData();
    ~ShaderDescData();

    const std::vector<std::shared_ptr<ShaderReflectionConstantBuffer>>&    GetReflectionCbufs()            const { return reflection_cbufs; }
    const std::vector<std::shared_ptr<SHADER_INPUT_BIND_DESC>>&            GetInputBindDescs()                const { return input_bind_descs; }
    const std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>&        GetInputSigParamDescs()            const { return input_sig_param_descs; }
    const std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>&        GetOutputSigParamDescs()        const { return output_sig_param_descs; }
    const std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>&        GetPatchConstantSigParamDescs()    const { return patch_constant_sig_param_descs; }

private:
    std::vector<std::shared_ptr<ShaderReflectionConstantBuffer>>    reflection_cbufs;
    std::vector<std::shared_ptr<SHADER_INPUT_BIND_DESC>>            input_bind_descs;
    std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>            input_sig_param_descs;
    std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>            output_sig_param_descs;
    std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>            patch_constant_sig_param_descs;
    
};

class ShaderReflection
{
public:
    ShaderReflection();
    ~ShaderReflection();

    bool ReflectFromBlob(const std::vector<uint8_t>& _blob);

    const SHADER_DESC&             GetShaderDesc()                     const { return *shader_desc; }
    const ShaderDescData&          GetShaderDescData()                 const { return *shader_desc_data; }
    const std::array<uint32_t, 3>& GetThreadGroupSizes()               const { return thread_group_sizes; }
    uint32_t                       GetThreadGroupTotalSize()           const { return thread_group_total_size; }
    uint32_t                       GetMovInstructionCount()            const { return mov_instruction_count; }
    uint32_t                       GetMovcInstructionCount()           const { return movc_instruction_count; }
    uint32_t                       GetConversionInstructionCount()     const { return conversion_instruction_count; }
    uint32_t                       GetBitwiseInstructionCount()        const { return bitwise_instruction_count; }
    PRIMITIVE_TYPE                 GetGSInputPrimitive()               const { return gs_input_primitive; }
    bool                           GetIsSampleFrequencyShader()        const { return is_sample_frequency_shader; }
    uint32_t                       GetNumInterfaceSlots()              const { return num_interface_slots; }
    FEATURE_LEVEL                  GetFeatureLevel()                   const { return feature_level; }
    SHADER_REQUIRE_FLAGS           GetRequireFlags()                   const { return require_flags; }

private:
    std::shared_ptr<SHADER_DESC>    shader_desc;
    std::shared_ptr<ShaderDescData> shader_desc_data;

    std::array<uint32_t, 3>         thread_group_sizes;
    uint32_t                        thread_group_total_size;
    uint32_t                        mov_instruction_count;
    uint32_t                        movc_instruction_count;
    uint32_t                        conversion_instruction_count;
    uint32_t                        bitwise_instruction_count;
    PRIMITIVE_TYPE                  gs_input_primitive;
    bool                            is_sample_frequency_shader;
    uint32_t                        num_interface_slots;

    FEATURE_LEVEL                   feature_level;

    SHADER_REQUIRE_FLAGS            require_flags;
};

class FunctionParameterReflection
{
public:
    class Initialize;
public:
    FunctionParameterReflection();
    ~FunctionParameterReflection();

    std::shared_ptr<const PARAMETER_DESC> GetParameterDesc() const { return param_desc; }

private:
    std::shared_ptr<PARAMETER_DESC> param_desc;

};

class FunctionReflection
{
public:
    class Initialize;
public:
    FunctionReflection();
    ~FunctionReflection();

    const std::shared_ptr<const FUNCTION_DESC>&                         GetFunctionDesc()                 const { return func_desc; }
    const std::vector<std::shared_ptr<ShaderReflectionConstantBuffer>>& GetReflectionConstantBuffers()    const { return reflection_cbufs; }
    const std::vector<std::shared_ptr<SHADER_INPUT_BIND_DESC>>&         GetInputBindDescs()               const { return input_bind_descs; }
    const std::vector<std::shared_ptr<FunctionParameterReflection>>&    GetFunctionParameterReflections() const { return func_param_reflections; }

private:
    std::shared_ptr<FUNCTION_DESC>                                    func_desc;
    std::vector<std::shared_ptr<ShaderReflectionConstantBuffer>>      reflection_cbufs;
    std::vector<std::shared_ptr<SHADER_INPUT_BIND_DESC>>              input_bind_descs;
    std::vector<std::shared_ptr<FunctionParameterReflection>>         func_param_reflections;
};

class LibraryReflection
{
public:
    LibraryReflection();
    ~LibraryReflection();

    bool ReflectFromBlob(const std::vector<uint8_t>& _blob);

    std::shared_ptr<const LIBRARY_DESC>                         GetLibraryDesc()            const { return lib_desc; }
    const std::vector<std::shared_ptr<FunctionReflection>>&     GetFunctionReflections()    const { return func_reflections; }

private:
    std::shared_ptr<LIBRARY_DESC>                           lib_desc;
    std::vector<std::shared_ptr<FunctionReflection>>        func_reflections;

};


}// namespace shader
}// namespace buma
