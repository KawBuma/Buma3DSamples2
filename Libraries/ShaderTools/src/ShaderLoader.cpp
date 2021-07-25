#include <ShaderTools/ShaderLoader.h>

#include <Utils/Definitions.h>
#include <Utils/Logger.h>

#include <ShaderConductor/ShaderConductor.hpp>

#include <cassert>
#include <fstream>
#include <memory>
#include <map>
#include <filesystem>

namespace buma
{
namespace shader
{

namespace /*anonymous*/
{

void PrepareDefines(const buma::shader::LOAD_SHADER_DESC& _desc, std::vector<ShaderConductor::MacroDefine>& _defines)
{
    size_t count = 0;
    for (auto& i : _desc.defines)
        _defines.data()[count++] = { i.def_name, i.def_value };
}

ShaderConductor::ShadingLanguage GetShadingLanguage(COMPILE_TARGET _type)
{
    switch (_type)
    {
    case COMPILE_TARGET_D3D12  : return ShaderConductor::ShadingLanguage::Dxil;
    case COMPILE_TARGET_VULKAN : return ShaderConductor::ShadingLanguage::SpirV;
    default:
        BUMA_ASSERT(false);
        return ShaderConductor::ShadingLanguage(-1);
    }
}

void PrepareTargetDesc(ShaderConductor::Compiler::TargetDesc& _tgt, buma::shader::SHADER_STAGE _stage, COMPILE_TARGET _type)
{
    _tgt.asModule = _stage == SHADER_STAGE_LIBRARY;
    _tgt.language = GetShadingLanguage(_type);
}

void PrepareOptions(ShaderConductor::Compiler::Options& _dst
                    , const buma::shader::OPTIONS& _opt
                    , COMPILE_TARGET                                                          _type
                    , std::unique_ptr<std::vector<ShaderConductor::Compiler::RegisterShift>>& _shifts
                    , buma::shader::SHADER_STAGE                                              _stage
)
{
    namespace SC = ShaderConductor;
    using SCC = ShaderConductor::Compiler;

    _dst.packMatricesInRowMajor  = _opt.pack_matrices_in_row_major;
    _dst.enable16bitTypes        = _opt.enable_16bit_types;
    _dst.enableDebugInfo         = _opt.enable_debug_info;
    _dst.disableOptimizations    = _opt.disable_optimizations;
    //_dst.invertYCoordinate     = _opt.invertYCoordinate;
    _dst.optimizationLevel       = (int)_opt.optimization_level;
    _dst.shaderModel.major_ver   = _opt.shader_model.major_ver;
    _dst.shaderModel.minor_ver   = _opt.shader_model.minor_ver;

    switch (_type)
    {
    case COMPILE_TARGET_D3D12:
        break;

    case COMPILE_TARGET_VULKAN:
    {
        _dst.shiftAllTexturesBindings  = _opt.shift_all_tex_bindings;
        _dst.shiftAllSamplersBindings  = _opt.shift_all_samp_bindings;
        _dst.shiftAllCBuffersBindings  = _opt.shift_all_cbuf_bindings;
        _dst.shiftAllUABuffersBindings = _opt.shift_all_ubuf_bindings;

        if (_opt.register_shifts)
        {
            _shifts = std::make_unique<std::vector<SCC::RegisterShift>>();
            for (auto& i : *_opt.register_shifts)
                _shifts->emplace_back(SCC::RegisterShift{ i.space,i.shift_tex_bindings,i.shift_samp_bindings,i.shift_cbuf_bindings,i.shift_ubuf_bindings });
            _dst.numRegisterShifts = (uint32_t)_shifts->size();
            _dst.registerShifts    = _shifts->data();
        }

        switch (_stage)
        {
        case SHADER_STAGE_VERTEX:
        case SHADER_STAGE_GEOMETRY:
        case SHADER_STAGE_DOMAIN:
            _dst.invertYCoordinate = true;
            break;
        default:
            break;
        }

        break;
    }

    default:
        break;
    }
}

void PrepareModules(std::vector<ShaderConductor::Compiler::ModuleDesc>& _modules, std::vector<const ShaderConductor::Compiler::ModuleDesc*>& _pmodules, const buma::shader::LIBRARY_LINK_DESC& _desc)
{
    _modules.reserve(_desc.link.modules.size());
    _pmodules.reserve(_desc.link.modules.size());
    for (auto& i : _desc.link.modules)
    {
        auto&& m = _modules.emplace_back();
        m.name = i->library_name;
        m.target.Reset(i->target->data(), (uint32_t)i->target->size());
        _pmodules.emplace_back(&m);
    }
}

bool CheckResult(ShaderConductor::Compiler::ResultDesc& _result)
{
    if (_result.hasError)
    {
        auto msg = (const char*)_result.errorWarningMsg.Data();
        log::error(msg);
        return false;
    }
    return true;
}


}// namespace /*anonymous*/


ShaderLoader::ShaderLoader(COMPILE_TARGET _type)
    : type{ _type }
{

}

void ShaderLoader::LoadShaderFromBinary(const char* _filename, std::vector<uint8_t>* _dst)
{
    std::ifstream file(_filename, std::ios::in | std::ios::binary);
    std::string source;
    source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    *_dst = std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(source.data())
                                 , reinterpret_cast<const uint8_t*>(source.data()) + source.size());
}

void ShaderLoader::LoadShaderFromHLSL(const LOAD_SHADER_DESC& _desc, std::vector<uint8_t>* _dst)
{
    LoadShaderFromHLSL(type, _desc, _dst);
}

void ShaderLoader::LoadShaderFromHLSLString(const LOAD_SHADER_DESC& _desc, const char* _src, std::vector<uint8_t>* _dst)
{
    LoadShaderFromHLSL(type, _desc, _src, _dst);
}

void ShaderLoader::LinkLibrary(const LIBRARY_LINK_DESC& _desc, std::vector<uint8_t>* _dst)
{
    LinkLibrary(type, _desc, _dst);
}

void ShaderLoader::LoadShaderFromHLSL(COMPILE_TARGET _type, const LOAD_SHADER_DESC& _desc, std::vector<uint8_t>* _dst)
{
    if (!std::filesystem::exists(_desc.filename))
    {
        BUMA_LOGE("{} not found", _desc.filename);
        return;
    }
    std::string source;
    std::ifstream file(_desc.filename, std::ios::in);
    source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    LoadShaderFromHLSL(_type, _desc, source.c_str(), _dst);
}

void ShaderLoader::LoadShaderFromHLSL(COMPILE_TARGET _type, const LOAD_SHADER_DESC& _desc, const char* _src, std::vector<uint8_t>* _dst)
{
    namespace SC = ShaderConductor;
    using SCC = ShaderConductor::Compiler;

    std::vector<SC::MacroDefine> defines(_desc.defines.size());
    PrepareDefines(_desc, defines);

    SCC::TargetDesc tgt{};
    PrepareTargetDesc(tgt, _desc.stage, _type);

    std::unique_ptr<std::vector<SCC::RegisterShift>> shifts;
    SCC::Options opt{};
    PrepareOptions(opt, _desc.options, _type, shifts, _desc.stage);

    /*
    NOTE: src.fileName が nulltpr の場合、"hlsl.hlsl" が読み込まれます。
          DirectXShaderCompiler/tools/clang/tools/dxcompiler/dxcompilerobj.cpp : DxcCompiler::Compile 関数内にて、ファイル名が指定されていない場合、
          "hlsl.hlsl" という文字列がデフォルトで設定されますが、実際に "hlsl.hlsl" が存在しない場合、 
          DirectXShaderCompiler/tools/clang/lib/SPIRV/EmitVisitor.cpp : EmitVisitor::visit 関数内 ReadSourceCode() が失敗します。
          また、空のファイルの場合、 .empty() 条件式によって同様に失敗とみなされます。
          コメント等を含めた hlsl.hlsl ダミーファイルをカレントディレクトリに配置し、これを回避します。 
    */
    SCC::SourceDesc src{
          _src                                      // source
        , _desc.filename                            // fileName
        , _desc.entry_point                         // entryPoint
        , static_cast<SC::ShaderStage>(_desc.stage) // stage
        , defines.data()                            // defines
        , (uint32_t)defines.size()                  // numDefines
        , nullptr                                   // loadIncludeCallback
    };

    auto result = ShaderConductor::Compiler::Compile(src, opt, tgt);
    if (!CheckResult(result))
        return;

    *_dst = std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(result.target.Data())
                                 , reinterpret_cast<const uint8_t*>(result.target.Data()) + result.target.Size());
}

void ShaderLoader::LinkLibrary(COMPILE_TARGET _type, const LIBRARY_LINK_DESC& _desc, std::vector<uint8_t>* _dst)
{
    namespace SC = ShaderConductor;
    using SCC = ShaderConductor::Compiler;

    SCC::TargetDesc tgt{};
    PrepareTargetDesc(tgt, _desc.link.stage, _type);

    std::unique_ptr<std::vector<SCC::RegisterShift>> shifts;
    SCC::Options opt{};
    PrepareOptions(opt, _desc.options, _type, shifts, _desc.link.stage);

    std::vector<SCC::ModuleDesc> modules;
    std::vector<const SCC::ModuleDesc*> pmodules;
    PrepareModules(modules, pmodules, _desc);

    SCC::LinkDesc ld{
          _desc.link.entry_point                            // entryPoint
        , static_cast<SC::ShaderStage>(_desc.link.stage)    // stage
        , pmodules.data()                                   // modules
        , (uint32_t)pmodules.size()                         // numModules
    };

    auto result = ShaderConductor::Compiler::Link(ld, opt, tgt);
    if (!CheckResult(result))
        return;

    *_dst = std::vector<uint8_t>(  reinterpret_cast<const uint8_t*>(result.target.Data())
                                 , reinterpret_cast<const uint8_t*>(result.target.Data()) + result.target.Size());
}

bool ShaderLoader::LinkSupport()
{
    return ShaderConductor::Compiler::LinkSupport();
}


}// namespace shader
}// namespace buma3d
