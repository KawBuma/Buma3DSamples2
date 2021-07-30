#pragma once

#include <vector>

namespace buma
{
namespace shader
{

struct SHADER_MODEL
{
    uint8_t major_ver;
    uint8_t minor_ver;

    uint16_t FullVersion() const noexcept { return (major_ver << 8) | minor_ver; }

    bool operator< (const SHADER_MODEL& other) const noexcept { return this->FullVersion() < other.FullVersion(); }
    bool operator==(const SHADER_MODEL& other) const noexcept { return this->FullVersion() == other.FullVersion(); }
    bool operator> (const SHADER_MODEL& other) const noexcept { return other < *this; }
    bool operator<=(const SHADER_MODEL& other) const noexcept { return (*this < other) || (*this == other); }
    bool operator>=(const SHADER_MODEL& other) const noexcept { return (*this > other) || (*this == other); }
};

struct REGISTER_SHIFT
{
    REGISTER_SHIFT(uint32_t _space, int _shift_tex_bindings = 0, int _shift_samp_bindings = 0, int _shift_cbuf_bindings = 0, int _shift_ubuf_bindings = 0)
        : space{ _space }, shift_tex_bindings{ _shift_tex_bindings }, shift_samp_bindings{ _shift_samp_bindings }, shift_cbuf_bindings{ _shift_cbuf_bindings }, shift_ubuf_bindings{ _shift_ubuf_bindings }
    {}

    uint32_t space;
    int shift_tex_bindings;
    int shift_samp_bindings;
    int shift_cbuf_bindings;
    int shift_ubuf_bindings;
};

struct OPTIONS
{
    bool                                pack_matrices_in_row_major      = true;     // 実験的: 行列をどのようにパックするかを決定する                                   Experimental: Decide how a matrix get packed
    bool                                enable_16bit_types              = false;    // half,uint16_tなどの16ビットタイプを有効にします。 シェーダーモデル6.2以降が必要  Enable 16-bit types, such as half, uint16_t. Requires shader model 6.2+
    bool                                enable_debug_info               = false;    // デバッグ情報をバイナリに埋め込む                                                 Embed debug info into the binary
    bool                                disable_optimizations           = false;    // 最適化を強制的にオフにします。 以下のoptimizationLevelは無視してください。       Force to turn off optimizations. Ignore optimizationLevel below.

    uint32_t                            optimization_level              = 3;        // 0 to 3, no optimization to most optimization
    SHADER_MODEL                        shader_model                    = { 6, 0 };

    int                                 shift_all_tex_bindings          = 0;
    int                                 shift_all_samp_bindings         = 0;
    int                                 shift_all_cbuf_bindings         = 0;
    int                                 shift_all_ubuf_bindings         = 0;
    const std::vector<REGISTER_SHIFT>*  register_shifts                 = nullptr;
};

enum SHADER_STAGE : uint32_t
{
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_PIXEL,
    SHADER_STAGE_GEOMETRY,
    SHADER_STAGE_HULL,
    SHADER_STAGE_DOMAIN,
    SHADER_STAGE_COMPUTE,

    SHADER_STAGE_LIBRARY,

    SHADER_STAGE_NUM_STAGES,
};

struct SHADER_DEFINES
{
    const char* def_name;
    const char* def_value;
};

struct LOAD_SHADER_DESC
{
    const char*                     filename;
    const char*                     entry_point;
    SHADER_STAGE                    stage;
    std::vector<SHADER_DEFINES>     defines;
    OPTIONS                         options;
};

struct MODULE_DESC
{
    const char*                     library_name;
    const std::vector<uint8_t>*     target;
};

struct LINK_DESC
{
    const char*                     entry_point;
    SHADER_STAGE                    stage;

    std::vector<const MODULE_DESC*> modules;
};

struct LIBRARY_LINK_DESC
{
    LINK_DESC link;
    OPTIONS   options;
};

enum COMPILE_TARGET
{
      COMPILE_TARGET_D3D12
    , COMPILE_TARGET_VULKAN
};

class ShaderLoader
{
public:
    ShaderLoader(COMPILE_TARGET _type);
    ~ShaderLoader() {}

    void LoadShaderFromBinary(const char* _filename, std::vector<uint8_t>* _dst);
    void LoadShaderFromHLSL(const LOAD_SHADER_DESC& _desc, std::vector<uint8_t>* _dst);
    void LoadShaderFromHLSLString(const LOAD_SHADER_DESC& _desc, const char* _src, std::vector<uint8_t>* _dst);
    void LinkLibrary(const LIBRARY_LINK_DESC& _desc, std::vector<uint8_t>* _dst);
    static void LoadShaderFromHLSL(COMPILE_TARGET _type, const LOAD_SHADER_DESC& _desc, std::vector<uint8_t>* _dst);
    static void LoadShaderFromHLSL(COMPILE_TARGET _type, const LOAD_SHADER_DESC& _desc, const char* _src, std::vector<uint8_t>* _dst);
    static void LinkLibrary(COMPILE_TARGET _type, const LIBRARY_LINK_DESC& _desc, std::vector<uint8_t>* _dst);
    static bool LinkSupport();

private:
    COMPILE_TARGET type;

};


}// namespace shader
}// namespace buma3d
