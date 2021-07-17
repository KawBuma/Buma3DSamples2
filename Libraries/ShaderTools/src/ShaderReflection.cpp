#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <ShaderTools/ShaderReflection.h>

#include <Utils/Utils.h>

#ifdef _WIN32
#include <wrl.h>
template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#endif // !_WIN32

#include <d3d12shader.h>
#include <dxc/dxcapi.h>


// 'std::iterator<std::input_iterator_tag,const hlsl::DxilContainerHeader *,ptrdiff_t,const hlsl::DxilContainerHeader **,const hlsl::DxilContainerHeader *&>':
// warning STL4015: The std::iterator class template (used as a base class to provide typedefs) is deprecated in C++17.(The <iterator> header is NOT deprecated.)
#pragma warning(disable : 4996)
#include <dxc/DxilContainer/DxilContainer.h>
#pragma warning(default : 4996)

#define ASSERT_HR(hr) assert(SUCCEEDED(hr))

namespace /* anonimous */
{
template <typename T>
static void CreateDxcReflectionFromBlob(const std::vector<uint8_t>& _buffer, ComPtr<T>& _out_reflection)
{
    auto dxc_module = LoadLibraryA("dxcompiler.dll");
    assert(dxc_module != NULL);
    DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(dxc_module, "DxcCreateInstance");

    HRESULT hr{};

    ComPtr<IDxcLibrary> lib;
    hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&lib));
    ASSERT_HR(hr);

    ComPtr<IDxcBlobEncoding> dxil_blob;
    hr = lib->CreateBlobWithEncodingFromPinned(_buffer.data(), (UINT32)_buffer.size(), DXC_CP_ACP, &dxil_blob);
    ASSERT_HR(hr);

    ComPtr<IDxcContainerReflection> container_reflection;
    hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&container_reflection));
    ASSERT_HR(hr);

    hr = container_reflection->Load(dxil_blob.Get());
    ASSERT_HR(hr);

    uint32_t dxil_part_index = ~0u;
    hr = container_reflection->FindFirstPartKind(hlsl::DFCC_DXIL, &dxil_part_index);
    ASSERT_HR(hr);
    hr = container_reflection->GetPartReflection(dxil_part_index, IID_PPV_ARGS(&_out_reflection));
    ASSERT_HR(hr);

    FreeLibrary(dxc_module);
    dxc_module = NULL;
}

//static const buma3d::RESOURCE_FORMAT FORMAT_TABLE[4][buma::shader::REGISTER_COMPONENT_TYPE_FLOAT32] =
//{
//      { buma3d::RESOURCE_FORMAT_R32_UINT            , buma3d::RESOURCE_FORMAT_R32_SINT             , buma3d::RESOURCE_FORMAT_R32_FLOAT            }
//    , { buma3d::RESOURCE_FORMAT_R32G32_UINT         , buma3d::RESOURCE_FORMAT_R32G32_SINT          , buma3d::RESOURCE_FORMAT_R32G32_FLOAT         }
//    , { buma3d::RESOURCE_FORMAT_R32G32B32_UINT      , buma3d::RESOURCE_FORMAT_R32G32B32_SINT       , buma3d::RESOURCE_FORMAT_R32G32B32_FLOAT      }
//    , { buma3d::RESOURCE_FORMAT_R32G32B32A32_UINT   , buma3d::RESOURCE_FORMAT_R32G32B32A32_SINT    , buma3d::RESOURCE_FORMAT_R32G32B32A32_FLOAT   }
//};
//
//static constexpr BYTE MASK_X    = D3D_COMPONENT_MASK_X;
//static constexpr BYTE MASK_XY   = D3D_COMPONENT_MASK_X | D3D_COMPONENT_MASK_Y;
//static constexpr BYTE MASK_XYZ  = D3D_COMPONENT_MASK_X | D3D_COMPONENT_MASK_Y | D3D_COMPONENT_MASK_Z;
//static constexpr BYTE MASK_XYZW = D3D_COMPONENT_MASK_X | D3D_COMPONENT_MASK_Y | D3D_COMPONENT_MASK_Z | D3D_COMPONENT_MASK_W;
//
//buma3d::RESOURCE_FORMAT DetermineDXGIFormat(buma::shader::SIGNATURE_PARAMETER_DESC& _desc)
//{
//    switch (_desc.mask)
//    {
//    case MASK_X    : return FORMAT_TABLE[0][_desc.component_type - 1];
//    case MASK_XY   : return FORMAT_TABLE[1][_desc.component_type - 1];
//    case MASK_XYZ  : return FORMAT_TABLE[2][_desc.component_type - 1];
//    case MASK_XYZW : return FORMAT_TABLE[3][_desc.component_type - 1];
//
//    default:
//        return buma3d::RESOURCE_FORMAT_UNKNOWN;
//    }
//}

}// namespace /*anonymous*/

namespace buma
{
namespace shader
{

//std::unique_ptr<util::InputLayoutDesc> CreateInputLayoutDesc(const std::vector<std::shared_ptr<SIGNATURE_PARAMETER_DESC>>& _descs)
//{
//    std::unique_ptr<util::InputLayoutDesc> result = std::make_unique<util::InputLayoutDesc>();
//
//    auto num_elem = _descs.size();
//    auto&& descs_data = _descs.data();
//    auto&& tmp = *result;
//    for (size_t i = 0; i < num_elem; i++)
//    {
//        auto&& elem = *descs_data[i];
//        auto&& res_elem = tmp;
//        res_elem.SetSemanticName(elem.semantic_name.c_str());
//        res_elem.SetSemanticIndex(elem.semantic_index);
//        res_elem.SetFormat(DetermineDXGIFormat(elem));
//
//        // 以下の値はリフレクションで取得できない
//        res_elem.SetInputSlot             (0);
//        res_elem.SetAlignedByteOffset     (D3D12_APPEND_ALIGNED_ELEMENT);
//        res_elem.SetInputSlotClass        (D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
//        res_elem.SetInstanceDataStepRate  (0);
//    }
//
//    return result;
//}


}// namespace shader
}// namespace buma

namespace buma
{
namespace shader
{


namespace /*anonymous*/
{

inline PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY _topology)
{
    switch (_topology)
    {
    case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED                  : return PRIMITIVE_TOPOLOGY_UNDEFINED;
    case D3D_PRIMITIVE_TOPOLOGY_POINTLIST                  : return PRIMITIVE_TOPOLOGY_POINT_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_LINELIST                   : return PRIMITIVE_TOPOLOGY_LINE_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP                  : return PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST               : return PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP              : return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ               : return PRIMITIVE_TOPOLOGY_LINE_LIST_ADJ;
    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ              : return PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJ;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ           : return PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJ;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ          : return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJ;
    case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST  : return PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCH_LIST;
    case D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST : return PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCH_LIST;

    default:
        return PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
};

inline PRIMITIVE_TYPE ConvertPrimitiveType(D3D_PRIMITIVE _primitive)
{
    switch (_primitive)
    {
    case D3D_PRIMITIVE_UNDEFINED              : return PRIMITIVE_TYPE_UNDEFINED;
    case D3D_PRIMITIVE_POINT                  : return PRIMITIVE_TYPE_POINT;
    case D3D_PRIMITIVE_LINE                   : return PRIMITIVE_TYPE_LINE;
    case D3D_PRIMITIVE_TRIANGLE               : return PRIMITIVE_TYPE_TRIANGLE;
    case D3D_PRIMITIVE_LINE_ADJ               : return PRIMITIVE_TYPE_LINE_ADJ;
    case D3D_PRIMITIVE_TRIANGLE_ADJ           : return PRIMITIVE_TYPE_TRIANGLE_ADJ;
    case D3D_PRIMITIVE_1_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_1_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_2_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_2_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_3_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_3_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_4_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_4_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_5_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_5_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_6_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_6_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_7_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_7_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_8_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_8_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_9_CONTROL_POINT_PATCH  : return PRIMITIVE_TYPE_9_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_10_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_10_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_11_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_11_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_12_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_12_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_13_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_13_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_14_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_14_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_15_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_15_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_16_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_16_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_17_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_17_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_18_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_18_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_19_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_19_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_20_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_20_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_21_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_21_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_22_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_22_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_23_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_23_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_24_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_24_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_25_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_25_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_26_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_26_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_27_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_27_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_28_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_28_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_29_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_29_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_30_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_30_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_31_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_31_CONTROL_POINT_PATCH;
    case D3D_PRIMITIVE_32_CONTROL_POINT_PATCH : return PRIMITIVE_TYPE_32_CONTROL_POINT_PATCH;

    default:
        return PRIMITIVE_TYPE_UNDEFINED;
    }
}

inline TESSELLATOR_OUTPUT_PRIMITIVE ConvertTessellatorOutputPrimitive(D3D_TESSELLATOR_OUTPUT_PRIMITIVE _output_primitive)
{
    switch (_output_primitive)
    {
    case D3D_TESSELLATOR_OUTPUT_UNDEFINED    : return TESSELLATOR_OUTPUT_PRIMITIVE_UNDEFINED;
    case D3D_TESSELLATOR_OUTPUT_POINT        : return TESSELLATOR_OUTPUT_PRIMITIVE_POINT;
    case D3D_TESSELLATOR_OUTPUT_LINE         : return TESSELLATOR_OUTPUT_PRIMITIVE_LINE;
    case D3D_TESSELLATOR_OUTPUT_TRIANGLE_CW  : return TESSELLATOR_OUTPUT_PRIMITIVE_TRIANGLE_CLOCKWISE;
    case D3D_TESSELLATOR_OUTPUT_TRIANGLE_CCW : return TESSELLATOR_OUTPUT_PRIMITIVE_TRIANGLE_COUNTER_CLOCKWISE;

    default:
        return TESSELLATOR_OUTPUT_PRIMITIVE_UNDEFINED;
    }
}

inline TESSELLATOR_DOMAIN ConvertTessellatorDomain(D3D_TESSELLATOR_DOMAIN _tessellator_domain)
{
    switch (_tessellator_domain)
    {
    case D3D_TESSELLATOR_DOMAIN_UNDEFINED : return TESSELLATOR_DOMAIN_UNDEFINED;
    case D3D_TESSELLATOR_DOMAIN_ISOLINE   : return TESSELLATOR_DOMAIN_ISOLINE;
    case D3D_TESSELLATOR_DOMAIN_TRI       : return TESSELLATOR_DOMAIN_TRI;
    case D3D_TESSELLATOR_DOMAIN_QUAD      : return TESSELLATOR_DOMAIN_QUAD;

    default:
        return TESSELLATOR_DOMAIN_UNDEFINED;
    }
}

inline TESSELLATOR_PARTITIONING ConvertTessellatorPartitioning(D3D_TESSELLATOR_PARTITIONING _partitioning)
{
    switch (_partitioning)
    {
    case D3D_TESSELLATOR_PARTITIONING_UNDEFINED       : return TESSELLATOR_PARTITIONING_UNDEFINED;
    case D3D_TESSELLATOR_PARTITIONING_INTEGER         : return TESSELLATOR_PARTITIONING_INTEGER;
    case D3D_TESSELLATOR_PARTITIONING_POW2            : return TESSELLATOR_PARTITIONING_POW2;
    case D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD  : return TESSELLATOR_PARTITIONING_FRACTIONAL_ODD;
    case D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN : return TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN;

    default:
        return TESSELLATOR_PARTITIONING_UNDEFINED;
    }
}

inline SHADER_INPUT_FLAGS ConvertShaderInputFlags(D3D_SHADER_INPUT_FLAGS _flags)
{
    SHADER_INPUT_FLAGS result = SHADER_INPUT_FLAG_UNDEFINED;
    if (_flags & D3D_SIF_USERPACKED)          result |= SHADER_INPUT_FLAG_USERPACKED;
    if (_flags & D3D_SIF_COMPARISON_SAMPLER)  result |= SHADER_INPUT_FLAG_COMPARISON_SAMPLER;
    if (_flags & D3D_SIF_TEXTURE_COMPONENT_0) result |= SHADER_INPUT_FLAG_TEXTURE_COMPONENT_0;
    if (_flags & D3D_SIF_TEXTURE_COMPONENT_1) result |= SHADER_INPUT_FLAG_TEXTURE_COMPONENT_1;
    if (_flags & D3D_SIF_TEXTURE_COMPONENTS)  result |= SHADER_INPUT_FLAG_TEXTURE_COMPONENTS;
    if (_flags & D3D_SIF_UNUSED)              result |= SHADER_INPUT_FLAG_UNUSED;

    return result;
}

inline RESOURCE_RETURN_TYPE ConvertResourceReturnType(D3D_RESOURCE_RETURN_TYPE _type)
{
    switch (_type)
    {
    case D3D_RETURN_TYPE_UNORM     : return RESOURCE_RETURN_TYPE_UNORM;
    case D3D_RETURN_TYPE_SNORM     : return RESOURCE_RETURN_TYPE_SNORM;
    case D3D_RETURN_TYPE_SINT      : return RESOURCE_RETURN_TYPE_SINT;
    case D3D_RETURN_TYPE_UINT      : return RESOURCE_RETURN_TYPE_UINT;
    case D3D_RETURN_TYPE_FLOAT     : return RESOURCE_RETURN_TYPE_FLOAT;
    case D3D_RETURN_TYPE_MIXED     : return RESOURCE_RETURN_TYPE_MIXED;
    case D3D_RETURN_TYPE_DOUBLE    : return RESOURCE_RETURN_TYPE_DOUBLE;
    case D3D_RETURN_TYPE_CONTINUED : return RESOURCE_RETURN_TYPE_CONTINUED;

    default:
        return RESOURCE_RETURN_TYPE_UNDEFINED;
    }
}

inline SRV_DIMENSION ConvertSrvDimension(D3D_SRV_DIMENSION _dimension)
{
    switch (_dimension)
    {
    case D3D_SRV_DIMENSION_UNKNOWN          : return SRV_DIMENSION_UNKNOWN;
    case D3D_SRV_DIMENSION_BUFFER           : return SRV_DIMENSION_BUFFER;
    case D3D_SRV_DIMENSION_TEXTURE1D        : return SRV_DIMENSION_TEXTURE1D;
    case D3D_SRV_DIMENSION_TEXTURE1DARRAY   : return SRV_DIMENSION_TEXTURE1DARRAY;
    case D3D_SRV_DIMENSION_TEXTURE2D        : return SRV_DIMENSION_TEXTURE2D;
    case D3D_SRV_DIMENSION_TEXTURE2DARRAY   : return SRV_DIMENSION_TEXTURE2DARRAY;
    case D3D_SRV_DIMENSION_TEXTURE2DMS      : return SRV_DIMENSION_TEXTURE2DMS;
    case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY : return SRV_DIMENSION_TEXTURE2DMSARRAY;
    case D3D_SRV_DIMENSION_TEXTURE3D        : return SRV_DIMENSION_TEXTURE3D;
    case D3D_SRV_DIMENSION_TEXTURECUBE      : return SRV_DIMENSION_TEXTURECUBE;
    case D3D_SRV_DIMENSION_TEXTURECUBEARRAY : return SRV_DIMENSION_TEXTURECUBEARRAY;
    case D3D_SRV_DIMENSION_BUFFEREX         : return SRV_DIMENSION_BUFFEREX;

    default:
        return SRV_DIMENSION_UNKNOWN;
    }
}

inline CBUFFER_TYPE ConvertCbufferType(D3D_CBUFFER_TYPE _type)
{
    switch (_type)
    {
    case D3D_CT_CBUFFER            : return CBUFFER_TYPE_CBUFFER;
    case D3D_CT_TBUFFER            : return CBUFFER_TYPE_TBUFFER;
    case D3D_CT_INTERFACE_POINTERS : return CBUFFER_TYPE_INTERFACE_POINTERS;
    case D3D_CT_RESOURCE_BIND_INFO : return CBUFFER_TYPE_RESOURCE_BIND_INFO;

    default:
        return CBUFFER_TYPE_UNKNOWN;
    }
}

inline SHADER_VARIABLE_CLASS ConvertShaderVariableClass(D3D_SHADER_VARIABLE_CLASS _class)
{
    switch (_class)
    {
    case D3D_SVC_SCALAR            : return SHADER_VARIABLE_CLASS_SCALAR;
    case D3D_SVC_VECTOR            : return SHADER_VARIABLE_CLASS_VECTOR;
    case D3D_SVC_MATRIX_ROWS       : return SHADER_VARIABLE_CLASS_MATRIX_ROWS;
    case D3D_SVC_MATRIX_COLUMNS    : return SHADER_VARIABLE_CLASS_MATRIX_COLUMNS;
    case D3D_SVC_OBJECT            : return SHADER_VARIABLE_CLASS_OBJECT;
    case D3D_SVC_STRUCT            : return SHADER_VARIABLE_CLASS_STRUCT;
    case D3D_SVC_INTERFACE_CLASS   : return SHADER_VARIABLE_CLASS_INTERFACE_CLASS;
    case D3D_SVC_INTERFACE_POINTER : return SHADER_VARIABLE_CLASS_INTERFACE_POINTER;

    default:
        return SHADER_VARIABLE_CLASS_UNDEFINED;
    }
}

inline SHADER_VARIABLE_TYPE ConvertShaderVariableType(D3D_SHADER_VARIABLE_TYPE _type)
{
    switch (_type)
    {
    case D3D_SVT_VOID                      : return SHADER_VARIABLE_TYPE_VOID;
    case D3D_SVT_BOOL                      : return SHADER_VARIABLE_TYPE_BOOL;
    case D3D_SVT_INT                       : return SHADER_VARIABLE_TYPE_INT;
    case D3D_SVT_FLOAT                     : return SHADER_VARIABLE_TYPE_FLOAT;
    case D3D_SVT_STRING                    : return SHADER_VARIABLE_TYPE_STRING;
    case D3D_SVT_TEXTURE                   : return SHADER_VARIABLE_TYPE_TEXTURE;
    case D3D_SVT_TEXTURE1D                 : return SHADER_VARIABLE_TYPE_TEXTURE1D;
    case D3D_SVT_TEXTURE2D                 : return SHADER_VARIABLE_TYPE_TEXTURE2D;
    case D3D_SVT_TEXTURE3D                 : return SHADER_VARIABLE_TYPE_TEXTURE3D;
    case D3D_SVT_TEXTURECUBE               : return SHADER_VARIABLE_TYPE_TEXTURECUBE;
    case D3D_SVT_SAMPLER                   : return SHADER_VARIABLE_TYPE_SAMPLER;
    case D3D_SVT_SAMPLER1D                 : return SHADER_VARIABLE_TYPE_SAMPLER1D;
    case D3D_SVT_SAMPLER2D                 : return SHADER_VARIABLE_TYPE_SAMPLER2D;
    case D3D_SVT_SAMPLER3D                 : return SHADER_VARIABLE_TYPE_SAMPLER3D;
    case D3D_SVT_SAMPLERCUBE               : return SHADER_VARIABLE_TYPE_SAMPLERCUBE;
    case D3D_SVT_PIXELSHADER               : return SHADER_VARIABLE_TYPE_PIXELSHADER;
    case D3D_SVT_VERTEXSHADER              : return SHADER_VARIABLE_TYPE_VERTEXSHADER;
    case D3D_SVT_PIXELFRAGMENT             : return SHADER_VARIABLE_TYPE_PIXELFRAGMENT;
    case D3D_SVT_VERTEXFRAGMENT            : return SHADER_VARIABLE_TYPE_VERTEXFRAGMENT;
    case D3D_SVT_UINT                      : return SHADER_VARIABLE_TYPE_UINT;
    case D3D_SVT_UINT8                     : return SHADER_VARIABLE_TYPE_UINT8;
    case D3D_SVT_GEOMETRYSHADER            : return SHADER_VARIABLE_TYPE_GEOMETRYSHADER;
    case D3D_SVT_RASTERIZER                : return SHADER_VARIABLE_TYPE_RASTERIZER;
    case D3D_SVT_DEPTHSTENCIL              : return SHADER_VARIABLE_TYPE_DEPTHSTENCIL;
    case D3D_SVT_BLEND                     : return SHADER_VARIABLE_TYPE_BLEND;
    case D3D_SVT_BUFFER                    : return SHADER_VARIABLE_TYPE_BUFFER;
    case D3D_SVT_CBUFFER                   : return SHADER_VARIABLE_TYPE_CBUFFER;
    case D3D_SVT_TBUFFER                   : return SHADER_VARIABLE_TYPE_TBUFFER;
    case D3D_SVT_TEXTURE1DARRAY            : return SHADER_VARIABLE_TYPE_TEXTURE1DARRAY;
    case D3D_SVT_TEXTURE2DARRAY            : return SHADER_VARIABLE_TYPE_TEXTURE2DARRAY;
    case D3D_SVT_RENDERTARGETVIEW          : return SHADER_VARIABLE_TYPE_RENDERTARGETVIEW;
    case D3D_SVT_DEPTHSTENCILVIEW          : return SHADER_VARIABLE_TYPE_DEPTHSTENCILVIEW;
    case D3D_SVT_TEXTURE2DMS               : return SHADER_VARIABLE_TYPE_TEXTURE2DMS;
    case D3D_SVT_TEXTURE2DMSARRAY          : return SHADER_VARIABLE_TYPE_TEXTURE2DMSARRAY;
    case D3D_SVT_TEXTURECUBEARRAY          : return SHADER_VARIABLE_TYPE_TEXTURECUBEARRAY;
    case D3D_SVT_HULLSHADER                : return SHADER_VARIABLE_TYPE_HULLSHADER;
    case D3D_SVT_DOMAINSHADER              : return SHADER_VARIABLE_TYPE_DOMAINSHADER;
    case D3D_SVT_INTERFACE_POINTER         : return SHADER_VARIABLE_TYPE_INTERFACE_POINTER;
    case D3D_SVT_COMPUTESHADER             : return SHADER_VARIABLE_TYPE_COMPUTESHADER;
    case D3D_SVT_DOUBLE                    : return SHADER_VARIABLE_TYPE_DOUBLE;
    case D3D_SVT_RWTEXTURE1D               : return SHADER_VARIABLE_TYPE_RWTEXTURE1D;
    case D3D_SVT_RWTEXTURE1DARRAY          : return SHADER_VARIABLE_TYPE_RWTEXTURE1DARRAY;
    case D3D_SVT_RWTEXTURE2D               : return SHADER_VARIABLE_TYPE_RWTEXTURE2D;
    case D3D_SVT_RWTEXTURE2DARRAY          : return SHADER_VARIABLE_TYPE_RWTEXTURE2DARRAY;
    case D3D_SVT_RWTEXTURE3D               : return SHADER_VARIABLE_TYPE_RWTEXTURE3D;
    case D3D_SVT_RWBUFFER                  : return SHADER_VARIABLE_TYPE_RWBUFFER;
    case D3D_SVT_BYTEADDRESS_BUFFER        : return SHADER_VARIABLE_TYPE_BYTEADDRESS_BUFFER;
    case D3D_SVT_RWBYTEADDRESS_BUFFER      : return SHADER_VARIABLE_TYPE_RWBYTEADDRESS_BUFFER;
    case D3D_SVT_STRUCTURED_BUFFER         : return SHADER_VARIABLE_TYPE_STRUCTURED_BUFFER;
    case D3D_SVT_RWSTRUCTURED_BUFFER       : return SHADER_VARIABLE_TYPE_RWSTRUCTURED_BUFFER;
    case D3D_SVT_APPEND_STRUCTURED_BUFFER  : return SHADER_VARIABLE_TYPE_APPEND_STRUCTURED_BUFFER;
    case D3D_SVT_CONSUME_STRUCTURED_BUFFER : return SHADER_VARIABLE_TYPE_CONSUME_STRUCTURED_BUFFER;
    case D3D_SVT_MIN8FLOAT                 : return SHADER_VARIABLE_TYPE_MIN8FLOAT;
    case D3D_SVT_MIN10FLOAT                : return SHADER_VARIABLE_TYPE_MIN10FLOAT;
    case D3D_SVT_MIN16FLOAT                : return SHADER_VARIABLE_TYPE_MIN16FLOAT;
    case D3D_SVT_MIN12INT                  : return SHADER_VARIABLE_TYPE_MIN12INT;
    case D3D_SVT_MIN16INT                  : return SHADER_VARIABLE_TYPE_MIN16INT;
    case D3D_SVT_MIN16UINT                 : return SHADER_VARIABLE_TYPE_MIN16UINT;

    default:
        return SHADER_VARIABLE_TYPE_UNDEFINED;
    }
}

inline SHADER_REQUIRE_FLAGS ConvertShaderRequireFlags(UINT64 _flags)
{
    SHADER_REQUIRE_FLAGS result = SHADER_REQUIRE_FLAG_NONE;
    if (_flags & D3D_SHADER_REQUIRES_DOUBLES)                                                        result |= SHADER_REQUIRE_FLAG_DOUBLES;
    if (_flags & D3D_SHADER_REQUIRES_EARLY_DEPTH_STENCIL)                                            result |= SHADER_REQUIRE_FLAG_EARLY_DEPTH_STENCIL;
    if (_flags & D3D_SHADER_REQUIRES_UAVS_AT_EVERY_STAGE)                                            result |= SHADER_REQUIRE_FLAG_UAVS_AT_EVERY_STAGE;
    if (_flags & D3D_SHADER_REQUIRES_64_UAVS)                                                        result |= SHADER_REQUIRE_FLAG_64UAVS;
    if (_flags & D3D_SHADER_REQUIRES_MINIMUM_PRECISION)                                              result |= SHADER_REQUIRE_FLAG_MINIMUM_PRECISION;
    if (_flags & D3D_SHADER_REQUIRES_11_1_DOUBLE_EXTENSIONS)                                         result |= SHADER_REQUIRE_FLAG_11_1_DOUBLE_EXTENSIONS;
    if (_flags & D3D_SHADER_REQUIRES_11_1_SHADER_EXTENSIONS)                                         result |= SHADER_REQUIRE_FLAG_11_1_SHADER_EXTENSIONS;
    if (_flags & D3D_SHADER_REQUIRES_LEVEL_9_COMPARISON_FILTERING)                                   result |= SHADER_REQUIRE_FLAG_LEVEL9_COMPARISON_FILTERING;
    if (_flags & D3D_SHADER_REQUIRES_TILED_RESOURCES)                                                result |= SHADER_REQUIRE_FLAG_TILED_RESOURCES;
    if (_flags & D3D_SHADER_REQUIRES_STENCIL_REF)                                                    result |= SHADER_REQUIRE_FLAG_STENCIL_REF;
    if (_flags & D3D_SHADER_REQUIRES_INNER_COVERAGE)                                                 result |= SHADER_REQUIRE_FLAG_INNER_COVERAGE;
    if (_flags & D3D_SHADER_REQUIRES_TYPED_UAV_LOAD_ADDITIONAL_FORMATS)                              result |= SHADER_REQUIRE_FLAG_TYPED_UAV_LOAD_ADDITIONAL_FORMATS;
    if (_flags & D3D_SHADER_REQUIRES_ROVS)                                                           result |= SHADER_REQUIRE_FLAG_ROVS;
    if (_flags & D3D_SHADER_REQUIRES_VIEWPORT_AND_RT_ARRAY_INDEX_FROM_ANY_SHADER_FEEDING_RASTERIZER) result |= SHADER_REQUIRE_FLAG_ARRAY_INDEX_FROM_ANY_SHADER;

    return result;
}

inline SHADER_VARIABLE_FLAGS ConvertShaderVariableFlags(UINT _flags)
{
    SHADER_VARIABLE_FLAGS result = SHADER_VARIABLE_FLAG_NONE;
    if (_flags & D3D_SHADER_REQUIRES_DOUBLES)               result |= SHADER_REQUIRE_FLAG_DOUBLES;
    if (_flags & D3D_SHADER_REQUIRES_EARLY_DEPTH_STENCIL)   result |= SHADER_VARIABLE_FLAG_USERPACKED;
    if (_flags & D3D_SHADER_REQUIRES_UAVS_AT_EVERY_STAGE)   result |= SHADER_VARIABLE_FLAG_USED;
    if (_flags & D3D_SHADER_REQUIRES_64_UAVS)               result |= SHADER_VARIABLE_FLAG_INTERFACE_POINTER;
    if (_flags & D3D_SHADER_REQUIRES_MINIMUM_PRECISION)     result |= SHADER_VARIABLE_FLAG_INTERFACE_PARAMETER;

    return result;
}

inline SHADER_INPUT_TYPE ConvertShaderInputType(D3D_SHADER_INPUT_TYPE _type)
{
    switch (_type)
    {
    case  D3D_SIT_CBUFFER                       : return SHADER_INPUT_TYPE_CBUFFER;
    case  D3D_SIT_TBUFFER                       : return SHADER_INPUT_TYPE_TBUFFER;
    case  D3D_SIT_TEXTURE                       : return SHADER_INPUT_TYPE_TEXTURE;
    case  D3D_SIT_SAMPLER                       : return SHADER_INPUT_TYPE_SAMPLER;
    case  D3D_SIT_UAV_RWTYPED                   : return SHADER_INPUT_TYPE_UAV_RWTYPED;
    case  D3D_SIT_STRUCTURED                    : return SHADER_INPUT_TYPE_STRUCTURED;
    case  D3D_SIT_UAV_RWSTRUCTURED              : return SHADER_INPUT_TYPE_UAV_RWSTRUCTURED;
    case  D3D_SIT_BYTEADDRESS                   : return SHADER_INPUT_TYPE_BYTEADDRESS;
    case  D3D_SIT_UAV_RWBYTEADDRESS             : return SHADER_INPUT_TYPE_UAV_RWBYTEADDRESS;
    case  D3D_SIT_UAV_APPEND_STRUCTURED         : return SHADER_INPUT_TYPE_UAV_APPEND_STRUCTURED;
    case  D3D_SIT_UAV_CONSUME_STRUCTURED        : return SHADER_INPUT_TYPE_UAV_CONSUME_STRUCTURED;
    case  D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER : return SHADER_INPUT_TYPE_UAV_RWSTRUCTURED_WITH_COUNTER;
    case  D3D_SIT_RTACCELERATIONSTRUCTURE       : return SHADER_INPUT_TYPE_RTACCELERATIONSTRUCTURE;
    case  D3D_SIT_UAV_FEEDBACKTEXTURE           : return SHADER_INPUT_TYPE_UAV_FEEDBACKTEXTURE;

    default:
        return SHADER_INPUT_TYPE_UNKNOWN;
    }
}

inline REGISTER_COMPONENT_TYPE ConvertRegisterComponentType(D3D_REGISTER_COMPONENT_TYPE _type)
{
    switch (_type)
    {
    case D3D_REGISTER_COMPONENT_UNKNOWN : return REGISTER_COMPONENT_TYPE_UNKNOWN;
    case D3D_REGISTER_COMPONENT_UINT32  : return REGISTER_COMPONENT_TYPE_UINT32;
    case D3D_REGISTER_COMPONENT_SINT32  : return REGISTER_COMPONENT_TYPE_SINT32;
    case D3D_REGISTER_COMPONENT_FLOAT32 : return REGISTER_COMPONENT_TYPE_FLOAT32;

    default:
        return REGISTER_COMPONENT_TYPE_UNKNOWN;
    }
}

inline SYSTEM_VALUE_NAME ConvertSystemValueName(D3D_NAME _name)
{
    switch (_name)
    {
    case D3D_NAME_UNDEFINED                     : return SYSTEM_VALUE_NAME_UNDEFINED;
    case D3D_NAME_POSITION                      : return SYSTEM_VALUE_NAME_POSITION;
    case D3D_NAME_CLIP_DISTANCE                 : return SYSTEM_VALUE_NAME_CLIP_DISTANCE;
    case D3D_NAME_CULL_DISTANCE                 : return SYSTEM_VALUE_NAME_CULL_DISTANCE;
    case D3D_NAME_RENDER_TARGET_ARRAY_INDEX     : return SYSTEM_VALUE_NAME_RENDER_TARGET_ARRAY_INDEX;
    case D3D_NAME_VIEWPORT_ARRAY_INDEX          : return SYSTEM_VALUE_NAME_VIEWPORT_ARRAY_INDEX;
    case D3D_NAME_VERTEX_ID                     : return SYSTEM_VALUE_NAME_VERTEX_ID;
    case D3D_NAME_PRIMITIVE_ID                  : return SYSTEM_VALUE_NAME_PRIMITIVE_ID;
    case D3D_NAME_INSTANCE_ID                   : return SYSTEM_VALUE_NAME_INSTANCE_ID;
    case D3D_NAME_IS_FRONT_FACE                 : return SYSTEM_VALUE_NAME_IS_FRONT_FACE;
    case D3D_NAME_SAMPLE_INDEX                  : return SYSTEM_VALUE_NAME_SAMPLE_INDEX;
    case D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR    : return SYSTEM_VALUE_NAME_FINAL_QUAD_EDGE_TESSFACTOR;
    case D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR  : return SYSTEM_VALUE_NAME_FINAL_QUAD_INSIDE_TESSFACTOR;
    case D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR     : return SYSTEM_VALUE_NAME_FINAL_TRI_EDGE_TESSFACTOR;
    case D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR   : return SYSTEM_VALUE_NAME_FINAL_TRI_INSIDE_TESSFACTOR;
    case D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR  : return SYSTEM_VALUE_NAME_FINAL_LINE_DETAIL_TESSFACTOR;
    case D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR : return SYSTEM_VALUE_NAME_FINAL_LINE_DENSITY_TESSFACTOR;
    case D3D_NAME_BARYCENTRICS                  : return SYSTEM_VALUE_NAME_BARYCENTRICS;
    case D3D_NAME_SHADINGRATE                   : return SYSTEM_VALUE_NAME_SHADING_RATE;
    case D3D_NAME_CULLPRIMITIVE                 : return SYSTEM_VALUE_NAME_CULL_PRIMITIVE;
    case D3D_NAME_TARGET                        : return SYSTEM_VALUE_NAME_TARGET;
    case D3D_NAME_DEPTH                         : return SYSTEM_VALUE_NAME_DEPTH;
    case D3D_NAME_COVERAGE                      : return SYSTEM_VALUE_NAME_COVERAGE;
    case D3D_NAME_DEPTH_GREATER_EQUAL           : return SYSTEM_VALUE_NAME_DEPTH_GREATER_EQUAL;
    case D3D_NAME_DEPTH_LESS_EQUAL              : return SYSTEM_VALUE_NAME_DEPTH_LESS_EQUAL;
    case D3D_NAME_STENCIL_REF                   : return SYSTEM_VALUE_NAME_STENCIL_REF;
    case D3D_NAME_INNER_COVERAGE                : return SYSTEM_VALUE_NAME_INNER_COVERAGE;

    default:
        return SYSTEM_VALUE_NAME_UNDEFINED;
    }
}

inline MIN_PRECISION ConvertMinPrecision(D3D_MIN_PRECISION _percision)
{
    switch (_percision)
    {
    case D3D_MIN_PRECISION_DEFAULT   : return MIN_PRECISION_DEFAULT;
    case D3D_MIN_PRECISION_FLOAT_16  : return MIN_PRECISION_FLOAT_16;
    case D3D_MIN_PRECISION_FLOAT_2_8 : return MIN_PRECISION_FLOAT_2_8;
    case D3D_MIN_PRECISION_SINT_16   : return MIN_PRECISION_SINT_16;
    case D3D_MIN_PRECISION_UINT_16   : return MIN_PRECISION_UINT_16;
    case D3D_MIN_PRECISION_ANY_16    : return MIN_PRECISION_ANY_16;
    case D3D_MIN_PRECISION_ANY_10    : return MIN_PRECISION_ANY_10;

    default:
        return MIN_PRECISION_UNKNOWN;
    }
}

inline FEATURE_LEVEL ConvertFeatureLevel(D3D_FEATURE_LEVEL _level)
{
    return static_cast<FEATURE_LEVEL>(_level);
}

inline INTERPOLATION_MODE ConvertInterpolationMode(D3D_INTERPOLATION_MODE _mode)
{
    switch (_mode)
    {
    case D3D_INTERPOLATION_UNDEFINED                     : return INTERPOLATION_MODE_UNDEFINED;
    case D3D_INTERPOLATION_CONSTANT                      : return INTERPOLATION_MODE_CONSTANT;
    case D3D_INTERPOLATION_LINEAR                        : return INTERPOLATION_MODE_LINEAR;
    case D3D_INTERPOLATION_LINEAR_CENTROID               : return INTERPOLATION_MODE_LINEAR_CENTROID;
    case D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE          : return INTERPOLATION_MODE_LINEAR_NOPERSPECTIVE;
    case D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID : return INTERPOLATION_MODE_LINEAR_NOPERSPECTIVE_CENTROID;
    case D3D_INTERPOLATION_LINEAR_SAMPLE                 : return INTERPOLATION_MODE_LINEAR_SAMPLE;
    case D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE_SAMPLE   : return INTERPOLATION_MODE_LINEAR_NOPERSPECTIVE_SAMPLE;

    default:
        return INTERPOLATION_MODE_UNDEFINED;
    }
}

inline PARAMETER_FLAGS ConvertParameterFlags(D3D_PARAMETER_FLAGS _flags)
{
    PARAMETER_FLAGS result = PARAMETER_FLAG_NONE;
    if (_flags & D3D_PF_IN)   result |= PARAMETER_FLAG_IN;
    if (_flags & D3D_PF_OUT)  result |= PARAMETER_FLAG_OUT;

    return result;
}


void InitShaderDesc(SHADER_DESC& _dst, const D3D12_SHADER_DESC& _desc)
{
    _dst.version                        = _desc.Version;
    _dst.creator                        = _desc.Creator ? _desc.Creator : "";
    _dst.flags                          = _desc.Flags;
    _dst.constant_buffers               = _desc.ConstantBuffers;
    _dst.bound_resources                = _desc.BoundResources;
    _dst.input_parameters               = _desc.InputParameters;
    _dst.output_parameters              = _desc.OutputParameters;
    _dst.instruction_count              = _desc.InstructionCount;
    _dst.temp_register_count            = _desc.TempRegisterCount;
    _dst.temp_array_count               = _desc.TempArrayCount;
    _dst.def_count                      = _desc.DefCount;
    _dst.dcl_count                      = _desc.DclCount;
    _dst.texture_normal_instructions    = _desc.TextureNormalInstructions;
    _dst.texture_load_instructions      = _desc.TextureLoadInstructions;
    _dst.texture_comp_instructions      = _desc.TextureCompInstructions;
    _dst.texture_bias_instructions      = _desc.TextureBiasInstructions;
    _dst.texture_gradient_instructions  = _desc.TextureGradientInstructions;
    _dst.float_instruction_count        = _desc.FloatInstructionCount;
    _dst.int_instruction_count          = _desc.IntInstructionCount;
    _dst.uint_instruction_count         = _desc.UintInstructionCount;
    _dst.static_flow_control_count      = _desc.StaticFlowControlCount;
    _dst.dynamic_flow_control_count     = _desc.DynamicFlowControlCount;
    _dst.macro_instruction_count        = _desc.MacroInstructionCount;
    _dst.array_instruction_count        = _desc.ArrayInstructionCount;
    _dst.cut_instruction_count          = _desc.CutInstructionCount;
    _dst.emit_instruction_count         = _desc.EmitInstructionCount;
    _dst.gs_output_topology             = ConvertPrimitiveTopology(_desc.GSOutputTopology);
    _dst.gs_max_output_vertex_count     = _desc.GSMaxOutputVertexCount;
    _dst.input_primitive                = ConvertPrimitiveType(_desc.InputPrimitive);
    _dst.patch_constant_parameters      = _desc.PatchConstantParameters;
    _dst.gs_instance_count              = _desc.cGSInstanceCount;
    _dst.control_points                 = _desc.cControlPoints;
    _dst.hs_output_primitive            = ConvertTessellatorOutputPrimitive(_desc.HSOutputPrimitive);
    _dst.hs_partitioning                = ConvertTessellatorPartitioning(_desc.HSPartitioning);
    _dst.tessellator_domain             = ConvertTessellatorDomain(_desc.TessellatorDomain);
    _dst.barrier_instructions           = _desc.cBarrierInstructions;
    _dst.interlocked_instructions       = _desc.cInterlockedInstructions;
    _dst.texture_store_instructions     = _desc.cTextureStoreInstructions;
}

void InitShaderInputBindDesc(SHADER_INPUT_BIND_DESC& _dst, const D3D12_SHADER_INPUT_BIND_DESC& _desc)
{
    _dst.name               = _desc.Name;
    _dst.shader_input_type  = ConvertShaderInputType(_desc.Type);
    _dst.start_bind_point   = _desc.BindPoint;
    _dst.bind_count         = _desc.BindCount;
    _dst.binding_flags      = ConvertShaderInputFlags(static_cast<D3D_SHADER_INPUT_FLAGS>(_desc.uFlags));
    _dst.return_type        = ConvertResourceReturnType(_desc.ReturnType);
    _dst.dimension          = ConvertSrvDimension(_desc.Dimension);
    _dst.num_samples        = _desc.NumSamples;
    _dst.register_space     = _desc.Space;
    _dst.range_id           = _desc.uID;
}

void InitSignatureParameterDesc(SIGNATURE_PARAMETER_DESC& _dst, const D3D12_SIGNATURE_PARAMETER_DESC& _desc)
{
    _dst.semantic_name      = _desc.SemanticName;
    _dst.semantic_index     = _desc.SemanticIndex;
    _dst.num_register       = _desc.Register;
    _dst.system_value_type  = ConvertSystemValueName(_desc.SystemValueType);
    _dst.component_type     = ConvertRegisterComponentType(_desc.ComponentType);
    _dst.mask               = _desc.Mask;
    _dst.read_write_mask    = _desc.ReadWriteMask;
    _dst.stream_index       = _desc.Stream;
    _dst.min_precision      = ConvertMinPrecision(_desc.MinPrecision);
}

void InitLibraryDesc(LIBRARY_DESC& _dst, const D3D12_LIBRARY_DESC& _desc)
{
    _dst.creator            = _desc.Creator ? _desc.Creator : "";
    _dst.compilation_flags  = _desc.Flags;
    _dst.function_count     = _desc.FunctionCount;
}

void InitFunctionDesc(FUNCTION_DESC& _dst, const D3D12_FUNCTION_DESC& _desc)
{
    _dst.version                            = _desc.Version;
    _dst.creator                            = _desc.Creator ? _desc.Creator : "";
    _dst.compilation_parse_flags            = _desc.Flags;
    _dst.constant_buffers                   = _desc.ConstantBuffers;
    _dst.bound_resources                    = _desc.BoundResources;
    _dst.instruction_count                  = _desc.InstructionCount;
    _dst.temp_register_count                = _desc.TempRegisterCount;
    _dst.temp_array_count                   = _desc.TempArrayCount;
    _dst.def_count                          = _desc.DefCount;
    _dst.dcl_count                          = _desc.DclCount;
    _dst.texture_normal_instructions        = _desc.TextureNormalInstructions;
    _dst.texture_load_instructions          = _desc.TextureLoadInstructions;
    _dst.texture_comp_instructions          = _desc.TextureCompInstructions;
    _dst.texture_bias_instructions          = _desc.TextureBiasInstructions;
    _dst.texture_gradient_instructions      = _desc.TextureGradientInstructions;
    _dst.float_instruction_count            = _desc.FloatInstructionCount;
    _dst.int_instruction_count              = _desc.IntInstructionCount;
    _dst.uint_instruction_count             = _desc.UintInstructionCount;
    _dst.static_flow_control_count          = _desc.StaticFlowControlCount;
    _dst.dynamic_flow_control_count         = _desc.DynamicFlowControlCount;
    _dst.macro_instruction_count            = _desc.MacroInstructionCount;
    _dst.array_instruction_count            = _desc.ArrayInstructionCount;
    _dst.mov_instruction_count              = _desc.MovInstructionCount;
    _dst.movc_instruction_count             = _desc.MovcInstructionCount;
    _dst.conversion_instruction_count       = _desc.ConversionInstructionCount;
    _dst.bitwise_instruction_count          = _desc.BitwiseInstructionCount;
    _dst.min_feature_level                  = ConvertFeatureLevel(_desc.MinFeatureLevel);
    _dst.required_feature_flags             = ConvertShaderRequireFlags(_desc.RequiredFeatureFlags);
    _dst.name                               = _desc.Name ? _desc.Name : "";
    _dst.function_parameter_count           = _desc.FunctionParameterCount;
    _dst.has_return                         = _desc.HasReturn;
    _dst.has10_level9_vertex_shader         = _desc.Has10Level9VertexShader;
    _dst.has10_level9_pixel_shader          = _desc.Has10Level9PixelShader;
}

void InitParameterDesc(PARAMETER_DESC& _dst, const D3D12_PARAMETER_DESC& _desc)
{
    _dst.name                   = _desc.Name         ? _desc.Name         : "";
    _dst.semantic_name          = _desc.SemanticName ? _desc.SemanticName : "";
    _dst.variable_type          = ConvertShaderVariableType(_desc.Type);
    _dst.variable_class         = ConvertShaderVariableClass(_desc.Class);
    _dst.rows                   = _desc.Rows;
    _dst.columns                = _desc.Columns;
    _dst.interpolation_mode     = ConvertInterpolationMode(_desc.InterpolationMode);
    _dst.parameter_flags        = _desc.Flags;
    _dst.first_in_register      = _desc.FirstInRegister;
    _dst.first_in_component     = _desc.FirstInComponent;
    _dst.first_out_register     = _desc.FirstOutRegister;
    _dst.first_out_component    = _desc.FirstOutComponent;
}


}// namespace /*anonymous*/

class ShaderReflectionType::Initialize
{
public:
    static void Init(ShaderReflectionType& _dst, ID3D12ShaderReflectionType* _reflectin_type)
    {
        D3D12_SHADER_TYPE_DESC shader_type_desc{};
        HRESULT hr{};
        hr = _reflectin_type->GetDesc(&shader_type_desc);
        ASSERT_HR(hr);

        _dst.type_desc.variable_class       = ConvertShaderVariableClass(shader_type_desc.Class);
        _dst.type_desc.variable_type        = ConvertShaderVariableType(shader_type_desc.Type);
        _dst.type_desc.num_rows             = shader_type_desc.Rows;
        _dst.type_desc.num_columns          = shader_type_desc.Columns;
        _dst.type_desc.num_elements         = shader_type_desc.Elements;
        _dst.type_desc.num_members          = shader_type_desc.Members;
        _dst.type_desc.structure_offset     = shader_type_desc.Offset;
        _dst.type_desc.type_name            = shader_type_desc.Name ? shader_type_desc.Name : "";

        for (uint32_t i = 0; i < _dst.type_desc.num_members; i++)
        {
            auto&& t = _dst.member_types.emplace_back();
            ShaderReflectionType::Initialize::Init(t, _reflectin_type->GetMemberTypeByIndex(i));
        }

        if (ID3D12ShaderReflectionType* sub_type = _reflectin_type->GetSubType()) // クラスに基本クラスがない場合は、NULLを返します。
        {
            _dst.sub_type = std::make_shared<ShaderReflectionType>();
            ShaderReflectionType::Initialize::Init(*_dst.sub_type, sub_type);
        }

        if (ID3D12ShaderReflectionType* base_class = _reflectin_type->GetBaseClass())
        {
            _dst.base_class = std::make_shared<ShaderReflectionType>();
            ShaderReflectionType::Initialize::Init(*_dst.base_class, base_class);
        }

        _dst.num_interfaces = _reflectin_type->GetNumInterfaces();
        for (uint32_t i = 0; i < _dst.num_interfaces; i++)
        {
            auto&& t = _dst.interface_types.emplace_back();
            ShaderReflectionType::Initialize::Init(t, _reflectin_type->GetInterfaceByIndex(i));
        }

        //hr = _reflectin_type->IsOfType(ID3D12ShaderReflectionType* pType);
        //ASSERT_HR(hr);
        //hr = _reflectin_type->ImplementsInterface(_In_ ID3D12ShaderReflectionType * pBase);
        //ASSERT_HR(hr);
    }
};

ShaderReflectionType::ShaderReflectionType()
    : type_desc       {}
    , num_interfaces  {}
    , member_types    {}
    , sub_type        {}
    , base_class      {}
    , interface_types {}
{
}

ShaderReflectionType::~ShaderReflectionType()
{
}

class ShaderReflectionConstantBuffer::Initialize
{
public:
    static void Init(ShaderReflectionConstantBuffer& _dst, ID3D12ShaderReflectionConstantBuffer* _reflection_buffer)
    {
        HRESULT hr{};
        D3D12_SHADER_BUFFER_DESC shader_buffer_desc{};
        hr = _reflection_buffer->GetDesc(&shader_buffer_desc);
        ASSERT_HR(hr);

        _dst.desc = std::make_shared<SHADER_BUFFER_DESC>();
        _dst.desc->name                 = shader_buffer_desc.Name;
        _dst.desc->cb_type              = ConvertCbufferType(shader_buffer_desc.Type);
        _dst.desc->num_variables        = shader_buffer_desc.Variables;
        _dst.desc->size_of_cb           = shader_buffer_desc.Size;
        _dst.desc->buffer_desc_flags    = shader_buffer_desc.uFlags;
    }
};

class ShaderReflectionVariable::Initialize
{
public:
    static void Init(ShaderReflectionVariable& _dst, ID3D12ShaderReflectionVariable* _reflection_variable)
    {
        HRESULT hr{};
        D3D12_SHADER_VARIABLE_DESC sv_desc{};
        hr = _reflection_variable->GetDesc(&sv_desc);
        ASSERT_HR(hr);

        _dst.variable_desc = std::make_shared<SHADER_VARIABLE_DESC>();
        _dst.variable_desc->name             = sv_desc.Name;
        _dst.variable_desc->start_offset     = sv_desc.StartOffset;
        _dst.variable_desc->variable_size    = sv_desc.Size;
        _dst.variable_desc->variable_flags   = ConvertShaderVariableFlags(sv_desc.uFlags);
        _dst.variable_desc->start_texture    = sv_desc.StartTexture;
        _dst.variable_desc->texture_size     = sv_desc.TextureSize;
        _dst.variable_desc->start_sampler    = sv_desc.StartSampler;
        _dst.variable_desc->sampler_size     = sv_desc.SamplerSize;

        _dst.variable_desc->default_value.resize(_dst.variable_desc->variable_size, 0);
        if (sv_desc.DefaultValue != nullptr)
            memcpy(_dst.variable_desc->default_value.data(), sv_desc.DefaultValue, _dst.variable_desc->variable_size);

        _dst.reflection_types = std::make_shared<ShaderReflectionType>();
        ShaderReflectionType::Initialize::Init(*_dst.reflection_types, _reflection_variable->GetType());

        _dst.reflection_constant_buffer = std::make_shared<ShaderReflectionConstantBuffer>();
        ShaderReflectionConstantBuffer::Initialize::Init(*_dst.reflection_constant_buffer, _reflection_variable->GetBuffer());

    }
};

ShaderReflectionVariable::ShaderReflectionVariable()
    : variable_desc                 {}
    , reflection_constant_buffer    {}
    , reflection_types              {}
    , interface_slots               {}
{
}

ShaderReflectionVariable::~ShaderReflectionVariable()
{
}


ShaderReflectionConstantBuffer::ShaderReflectionConstantBuffer()
    :desc{}
{
}

ShaderReflectionConstantBuffer::~ShaderReflectionConstantBuffer()
{
}


class ShaderDescData::Initialize
{
public:
    static void Init(ShaderDescData& _dst, ID3D12ShaderReflection* _reflection, const SHADER_DESC& _desc)
    {
        HRESULT hr{};

        // constant_buffers
        {
            for (uint32_t i = 0; i < _desc.constant_buffers; i++)
            {
                auto&& c = _dst.reflection_cbufs.emplace_back(std::make_shared<ShaderReflectionConstantBuffer>());
                ShaderReflectionConstantBuffer::Initialize::Init(*c, _reflection->GetConstantBufferByIndex(i));
            }
        }
        // bound_resources
        {
            for (uint32_t i = 0; i < _desc.bound_resources; i++)
            {
                D3D12_SHADER_INPUT_BIND_DESC bind_desc{};
                hr = _reflection->GetResourceBindingDesc(i, &bind_desc);
                ASSERT_HR(hr);

                auto&& s = _dst.input_bind_descs.emplace_back(std::make_shared<SHADER_INPUT_BIND_DESC>());
                InitShaderInputBindDesc(*s, bind_desc);
            }
        }
        // input_parameters
        {
            for (uint32_t i = 0; i < _desc.input_parameters; i++)
            {
                D3D12_SIGNATURE_PARAMETER_DESC sigparam_desc{};
                hr = _reflection->GetInputParameterDesc(i, &sigparam_desc);
                ASSERT_HR(hr);

                auto&& s = _dst.input_sig_param_descs.emplace_back(std::make_shared<SIGNATURE_PARAMETER_DESC>());
                InitSignatureParameterDesc(*s, sigparam_desc);
            }
        }
        // output_parameters
        {
            for (uint32_t i = 0; i < _desc.output_parameters; i++)
            {
                D3D12_SIGNATURE_PARAMETER_DESC sigparam_desc{};
                hr = _reflection->GetOutputParameterDesc(i, &sigparam_desc);
                ASSERT_HR(hr);

                auto&& s = _dst.output_sig_param_descs.emplace_back(std::make_shared<SIGNATURE_PARAMETER_DESC>());
                InitSignatureParameterDesc(*s, sigparam_desc);
            }
        }
        // patch_constant_parameters
        {
            for (uint32_t i = 0; i < _desc.patch_constant_parameters; i++)
            {
                D3D12_SIGNATURE_PARAMETER_DESC sigparam_desc{};
                hr = _reflection->GetPatchConstantParameterDesc(i, &sigparam_desc);
                ASSERT_HR(hr);

                auto&& s =_dst.patch_constant_sig_param_descs.emplace_back(std::make_shared<SIGNATURE_PARAMETER_DESC>());
                InitSignatureParameterDesc(*s, sigparam_desc);
            }
        }
    }
};

ShaderDescData::ShaderDescData()
{
}

ShaderDescData::~ShaderDescData()
{
}

ShaderReflection::ShaderReflection()
    : thread_group_sizes            {}
    , mov_instruction_count         {}
    , movc_instruction_count        {}
    , conversion_instruction_count  {}
    , bitwise_instruction_count     {}
    , gs_input_primitive            {}
    , is_sample_frequency_shader    {}
    , num_interface_slots           {}
    , feature_level                 {}
    , require_flags                 {}
{
}

ShaderReflection::~ShaderReflection()
{
}

bool ShaderReflection::ReflectFromBlob(const std::vector<uint8_t>& _blob)
{
    ComPtr<ID3D12ShaderReflection> reflection;
    CreateDxcReflectionFromBlob(_blob, reflection);
    if (!reflection)
        return false;

    HRESULT hr{};
    D3D12_SHADER_DESC d3d_shader_desc{};
    hr = reflection->GetDesc(&d3d_shader_desc);
    ASSERT_HR(hr);

    shader_desc = std::make_shared<SHADER_DESC>();
    shader_desc_data = std::make_shared<ShaderDescData>();
    InitShaderDesc(*shader_desc, d3d_shader_desc);
    ShaderDescData::Initialize::Init(*shader_desc_data, reflection.Get(), *shader_desc);

    mov_instruction_count              = reflection->GetMovInstructionCount();
    movc_instruction_count             = reflection->GetMovcInstructionCount();
    conversion_instruction_count       = reflection->GetConversionInstructionCount();
    bitwise_instruction_count          = reflection->GetBitwiseInstructionCount();
    gs_input_primitive                 = ConvertPrimitiveType(reflection->GetGSInputPrimitive());
    is_sample_frequency_shader         = reflection->IsSampleFrequencyShader();
    num_interface_slots                = reflection->GetNumInterfaceSlots();

    D3D_FEATURE_LEVEL fl;
    hr = reflection->GetMinFeatureLevel(&fl);
    ASSERT_HR(hr);
    feature_level = ConvertFeatureLevel(fl);

    thread_group_total_size = reflection->GetThreadGroupSize(&thread_group_sizes[0], &thread_group_sizes[1], &thread_group_sizes[2]);

    require_flags = ConvertShaderRequireFlags(reflection->GetRequiresFlags());

    return true;
}

class FunctionParameterReflection::Initialize
{
public:
    static void Init(FunctionParameterReflection& _dst, ID3D12FunctionParameterReflection* _fparam_ref)
    {
        D3D12_PARAMETER_DESC d3dparam_desc{};
        auto hr = _fparam_ref->GetDesc(&d3dparam_desc);
        ASSERT_HR(hr);

        _dst.param_desc = std::make_shared<PARAMETER_DESC>();
        InitParameterDesc(*_dst.param_desc, d3dparam_desc);
    }
};

FunctionParameterReflection::FunctionParameterReflection()
    : param_desc {}
{
}

FunctionParameterReflection::~FunctionParameterReflection()
{
}

class FunctionReflection::Initialize
{
public:
    static void Init(FunctionReflection& _dst, ID3D12FunctionReflection* _func_ref)
    {
        D3D12_FUNCTION_DESC d3dfunc_desc{};
        HRESULT hr;
        hr = _func_ref->GetDesc(&d3dfunc_desc);
        ASSERT_HR(hr);

        _dst.func_desc = std::make_shared<FUNCTION_DESC>();
        InitFunctionDesc(*_dst.func_desc, d3dfunc_desc);

        for (uint32_t i = 0; i < _dst.func_desc->constant_buffers; i++)
        {
            ID3D12ShaderReflectionConstantBuffer* ref_cb = _func_ref->GetConstantBufferByIndex(i);
            auto&& b = _dst.reflection_cbufs.emplace_back(std::make_shared<ShaderReflectionConstantBuffer>());
            ShaderReflectionConstantBuffer::Initialize::Init(*b, ref_cb);
        }

        for (uint32_t i = 0; i < _dst.func_desc->bound_resources; i++)
        {
            D3D12_SHADER_INPUT_BIND_DESC bind_desc{};
            hr = _func_ref->GetResourceBindingDesc(i, &bind_desc);
            ASSERT_HR(hr);
            auto&& b = _dst.input_bind_descs.emplace_back(std::make_shared<SHADER_INPUT_BIND_DESC>());
            InitShaderInputBindDesc(*b, bind_desc);
        }

        for (int i = 0; i < _dst.func_desc->function_parameter_count; i++)
        {
            auto&& p = _dst.func_param_reflections.emplace_back(std::make_shared<FunctionParameterReflection>());
            FunctionParameterReflection::Initialize::Init(*p, _func_ref->GetFunctionParameter(i));
        }
    }
};

FunctionReflection::FunctionReflection()
    : func_desc              {}
    , reflection_cbufs       {}
    , input_bind_descs       {}
    , func_param_reflections {}
{
}

FunctionReflection::~FunctionReflection()
{
}

LibraryReflection::LibraryReflection()
    : lib_desc         {}
    , func_reflections {}
{
}

LibraryReflection::~LibraryReflection()
{
}

bool LibraryReflection::ReflectFromBlob(const std::vector<uint8_t>& _blob)
{
    ComPtr<ID3D12LibraryReflection> libreflection;
    CreateDxcReflectionFromBlob(_blob, libreflection);
    if (!libreflection)
        return false;

    D3D12_LIBRARY_DESC desc{};
    auto hr = libreflection->GetDesc(&desc);
    ASSERT_HR(hr);

    lib_desc = std::make_shared<LIBRARY_DESC>();
    InitLibraryDesc(*lib_desc, desc);

    for (uint32_t i = 0; i < lib_desc->function_count; i++)
    {
        auto&& f = func_reflections.emplace_back(std::make_shared<FunctionReflection>());
        FunctionReflection::Initialize::Init(*f, libreflection->GetFunctionByIndex((INT)i));
    }

    return true;
}


}// namespace shader
}// namespace buma
