#include <ShaderTools/ShaderLoader.h>

#include <Utils/Definitions.h>
#include <Utils/Utils.h>
#include <Utils/Logger.h>

#include <d3d12shader.h>
#include <dxcapi.h>

#ifdef _WIN32
#include <wrl.h>
#else
#error support non-Windows
#endif // _WIN32

#include <cassert>
#include <fstream>
#include <memory>
#include <map>
#include <filesystem>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace buma
{
namespace shader
{

namespace /*anonymous*/
{

class IncludeHandler : public IDxcIncludeHandler
{
public:
    explicit IncludeHandler(IDxcUtils* _utl)
        : ref{ 0 }
        , utl{ _utl }
    {
    }

    HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
    {
        if (!std::filesystem::exists(pFilename))
            return E_FAIL;

        std::ifstream file(pFilename, std::ios::in);
        std::string source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        *ppIncludeSource = nullptr;
        return utl->CreateBlob(source.c_str(), source.size(), DXC_CP_UTF8, reinterpret_cast<IDxcBlobEncoding**>(ppIncludeSource));
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        ++ref;
        return ref;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        --ref;
        ULONG result = ref;
        if (result == 0)
        {
            delete this;
        }
        return result;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
    {
        if (IsEqualIID(iid, __uuidof(IDxcIncludeHandler)))
        {
            *object = dynamic_cast<IDxcIncludeHandler*>(this);
            this->AddRef();
            return S_OK;
        }
        else if (IsEqualIID(iid, __uuidof(IUnknown)))
        {
            *object = dynamic_cast<IUnknown*>(this);
            this->AddRef();
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

private:
    std::atomic_uint32_t ref;
    IDxcUtils* utl;

};

std::string BlobToUtf8(IDxcBlob* _blob)
{
    if (!_blob)
        return std::string();

    ComPtr<IDxcBlobUtf8> blobu8;
    if (SUCCEEDED(_blob->QueryInterface(IID_PPV_ARGS(&blobu8))))
        return std::string(blobu8->GetStringPointer(), blobu8->GetStringLength());

    ComPtr<IDxcBlobEncoding> enc;
    _blob->QueryInterface(IID_PPV_ARGS(&enc));
    BOOL known; UINT32 cp;
    enc->GetEncoding(&known, &cp);
    if (!known)
        BUMA_ASSERT(false && "unknown codepage for blob.");

    std::string result;
    if (cp == DXC_CP_UTF16)
    {
        const wchar_t* text = (const wchar_t*)_blob->GetBufferPointer();
        size_t length = _blob->GetBufferSize() / sizeof(wchar_t);
        if (length >= 1 && text[length - 1] == L'\0')
            length -= 1;  // Exclude null-terminator

        result = util::ConvertWideToUtf8(text);
        return result;
    }
    else if (cp == CP_UTF8)
    {
        const char* text = (const char*)_blob->GetBufferPointer();
        size_t length = _blob->GetBufferSize();
        if (length >= 1 && text[length - 1] == '\0')
            length -= 1;  // Exclude null-terminator

        result.resize(length);
        memcpy(result.data(), text, length);
        return result;
    }
    else
    {
        BUMA_ASSERT(false && "Unsupported codepage.");
        return std::string();
    }
}

void PrepareDefines(const buma::shader::LOAD_SHADER_DESC& _desc, IDxcCompilerArgs* _args)
{
    for (auto& i : _desc.defines)
    {
        auto&& n = util::ConvertAnsiToWide(i.def_name);
        auto&& v = util::ConvertAnsiToWide(i.def_value);
        _args->AddDefines(&DxcDefine({ n.c_str(), v.c_str() }), 1);
    }
}

void PrepareTargetDesc(buma::shader::SHADER_STAGE _stage, COMPILE_TARGET _type, IDxcCompilerArgs* _args)
{
    if (_type == COMPILE_TARGET_VULKAN)
    {
        const char* target[] = { "-spirv" };
        _args->AddArgumentsUTF8(target, 1);
    }
}

void PrepareShaderModel(buma::shader::SHADER_STAGE _stage, buma::shader::SHADER_MODEL _sm, IDxcCompilerArgs* _args)
{
    const char* tgt = "-T";
    _args->AddArgumentsUTF8(&tgt, 1);

    std::string profile;
    switch (_stage)
    {
    case buma::shader::SHADER_STAGE_VERTEX   : profile = "vs_";  break;
    case buma::shader::SHADER_STAGE_PIXEL    : profile = "ps_";  break;
    case buma::shader::SHADER_STAGE_GEOMETRY : profile = "gs_";  break;
    case buma::shader::SHADER_STAGE_HULL     : profile = "hs_";  break;
    case buma::shader::SHADER_STAGE_DOMAIN   : profile = "ds_";  break;
    case buma::shader::SHADER_STAGE_COMPUTE  : profile = "cs_";  break;
    case buma::shader::SHADER_STAGE_LIBRARY  : profile = "lib_"; break;
    default:
        break;
    }
    profile += '0' + _sm.major_ver;
    profile += '_';
    profile += '0' + _sm.minor_ver;

    auto pfof = profile.c_str();
    _args->AddArgumentsUTF8(&pfof, 1);
}

void AddRegisterShifts(const shader::OPTIONS& _opt, std::vector<std::string>& _args)
{
    if (_opt.shift_all_cbuf_bindings > 0)
    {
        _args.push_back("-fvk-b-shift");
        _args.push_back(std::to_string(_opt.shift_all_cbuf_bindings));
        _args.push_back("all");
    }
    if (_opt.shift_all_ubuf_bindings > 0)
    {
        _args.push_back("-fvk-u-shift");
        _args.push_back(std::to_string(_opt.shift_all_ubuf_bindings));
        _args.push_back("all");
    }
    if (_opt.shift_all_samp_bindings > 0)
    {
        _args.push_back("-fvk-s-shift");
        _args.push_back(std::to_string(_opt.shift_all_samp_bindings));
        _args.push_back("all");
    }
    if (_opt.shift_all_tex_bindings > 0)
    {
        _args.push_back("-fvk-t-shift");
        _args.push_back(std::to_string(_opt.shift_all_tex_bindings));
        _args.push_back("all");
    }

    if (_opt.register_shifts)
    {
        for (auto& i : *_opt.register_shifts)
        {
            //TODO: register_shifts;
        }
    }
}

void SetInvertYCoordinate(shader::SHADER_STAGE _stage, std::vector<std::string>& _args)
{
    switch (_stage)
    {
    case SHADER_STAGE_VERTEX:
    case SHADER_STAGE_GEOMETRY:
    case SHADER_STAGE_DOMAIN:
        _args.push_back("-fvk-invert-y");
        break;
    default:
        break;
    }
}

void PrepareOptions(  const buma::shader::OPTIONS&  _opt
                    , COMPILE_TARGET                _type
                    , buma::shader::SHADER_STAGE    _stage
                    , IDxcCompilerArgs*             _args
)
{
    PrepareShaderModel(_stage, _opt.shader_model, _args);

    std::vector<std::string> args;

    if (_opt.pack_matrices_in_row_major)
        args.push_back("-Zpr");
    else
        args.push_back("-Zpc");

    if (_opt.enable_16bit_types)
        args.push_back("-enable-16bit-types");

    if (_opt.enable_debug_info)
        args.push_back("-Zi");

    if (_opt.disable_optimizations)
        args.push_back("-Od");
    else
        args.push_back(std::string("-O") + char('0' + _opt.optimization_level));
    
    switch (_type)
    {
    case COMPILE_TARGET_D3D12:
        break;
    case COMPILE_TARGET_VULKAN:
        AddRegisterShifts(_opt, args);
        SetInvertYCoordinate(_stage, args);
        break;
    default:
        break;
    }

    size_t cnt = 0;
    std::vector<LPCSTR> tmp(args.size());
    for (auto& i : args) tmp.data()[cnt++] = i.c_str();
    _args->AddArgumentsUTF8(tmp.data(), (uint32_t)tmp.size());
}

//void PrepareModules(std::vector<ShaderConductor::Compiler::ModuleDesc>& _modules, std::vector<const ShaderConductor::Compiler::ModuleDesc*>& _pmodules, const buma::shader::LIBRARY_LINK_DESC& _desc)
//{
//    _modules.reserve(_desc.link.modules.size());
//    _pmodules.reserve(_desc.link.modules.size());
//    for (auto& i : _desc.link.modules)
//    {
//        auto&& m = _modules.emplace_back();
//        m.name = i->library_name;
//        m.target.Reset(i->target->data(), (uint32_t)i->target->size());
//        _pmodules.emplace_back(&m);
//    }
//}
//
//bool CheckResult(ShaderConductor::Compiler::ResultDesc& _result)
//{
//    if (_result.hasError)
//    {
//        auto msg = (const char*)_result.errorWarningMsg.Data();
//        log::error(msg);
//        return false;
//    }
//    return true;
//}


}// namespace /*anonymous*/


ShaderLoader::ShaderLoader(COMPILE_TARGET _type)
    : type{ _type }
{

}

void ShaderLoader::LoadShaderFromBinary(const char* _filename, std::vector<uint8_t>* _dst)
{
    if (!std::filesystem::exists(_filename))
    {
        BUMA_LOGE("{} not found", _filename);
        return;
    }
    std::ifstream file(_filename, std::ios::in | std::ios::binary);
    std::string source;
    source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    *_dst = std::vector<uint8_t>(  reinterpret_cast<const uint8_t*>(source.data())
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

//void ShaderLoader::LinkLibrary(const LIBRARY_LINK_DESC& _desc, std::vector<uint8_t>* _dst)
//{
//    LinkLibrary(type, _desc, _dst);
//}

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
    // OPTIMIZE: モジュールインスタンスを保持すべき
    auto dxc = LoadLibraryA("dxcompiler.dll");
    {
        auto DxcCreateInstance2 = (DxcCreateInstance2Proc)GetProcAddress(dxc, "DxcCreateInstance2");

        // IDxcUtils は、コンパイラに渡すBlobや引数バッファ等のユーティリティオブジェクトを作成することができます。
        ComPtr<IDxcUtils> utl;
        DxcCreateInstance2(nullptr, CLSID_DxcUtils, IID_PPV_ARGS(&utl));

        // コンパイル引数を設定
        ComPtr<IDxcCompilerArgs> args;
        DxcCreateInstance2(nullptr, CLSID_DxcCompilerArgs, IID_PPV_ARGS(&args));
        {
            PrepareDefines(_desc, args.Get());

            PrepareTargetDesc(_desc.stage, _type, args.Get());

            PrepareOptions(_desc.options, _type, _desc.stage, args.Get());

            const char* entry_point[2] = { "-E", _desc.entry_point ? _desc.entry_point : "main" };
            args->AddArgumentsUTF8(entry_point, 2);

            if (_desc.filename)
            {
                const char* filename = _desc.filename;
                args->AddArgumentsUTF8(&filename, 1);
            }
        }

        // コンパイル
        ComPtr<IDxcResult> result;
        {
            ComPtr<IDxcCompiler3> comp;
            DxcCreateInstance2(nullptr, CLSID_DxcCompiler, IID_PPV_ARGS(&comp));

            DxcBuffer src{};
            src.Ptr      = _src;
            src.Size     = std::strlen(_src);
            src.Encoding = DXC_CP_UTF8;

            ComPtr<IncludeHandler> handler = new IncludeHandler(utl.Get());
            comp->Compile(&src, args->GetArguments(), args->GetCount(), handler.Get(), IID_PPV_ARGS(&result));
        }

        // コンパイル結果を処理
        if (result->HasOutput(DXC_OUT_ERRORS))
        {
            ComPtr<IDxcBlobEncoding> err;
            result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&err), nullptr);
            if (auto s = BlobToUtf8(err.Get()); !s.empty())
            {
                if (s.find_first_of("warning:") != std::string::npos)
                    BUMA_LOGW(s);
                else
                    BUMA_LOGE(s);
            }
        }
        if (result->HasOutput(DXC_OUT_OBJECT))
        {
            ComPtr<IDxcBlob> obj;
            result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&obj), nullptr);

            *_dst = std::vector<uint8_t>(  reinterpret_cast<const uint8_t*>(obj->GetBufferPointer())
                                         , reinterpret_cast<const uint8_t*>(obj->GetBufferPointer()) + obj->GetBufferSize());
        }
    }
    FreeLibrary(dxc);
}

//void ShaderLoader::LinkLibrary(COMPILE_TARGET _type, const LIBRARY_LINK_DESC& _desc, std::vector<uint8_t>* _dst)
//{
//    namespace SC = ShaderConductor;
//    using SCC = ShaderConductor::Compiler;
//
//    SCC::TargetDesc tgt{};
//    PrepareTargetDesc(tgt, _desc.link.stage, _type);
//
//    std::unique_ptr<std::vector<SCC::RegisterShift>> shifts;
//    SCC::Options opt{};
//    PrepareOptions(opt, _desc.options, _type, shifts, _desc.link.stage);
//
//    std::vector<SCC::ModuleDesc> modules;
//    std::vector<const SCC::ModuleDesc*> pmodules;
//    PrepareModules(modules, pmodules, _desc);
//
//    SCC::LinkDesc ld{
//          _desc.link.entry_point                            // entryPoint
//        , static_cast<SC::ShaderStage>(_desc.link.stage)    // stage
//        , pmodules.data()                                   // modules
//        , (uint32_t)pmodules.size()                         // numModules
//    };
//
//    auto result = ShaderConductor::Compiler::Link(ld, opt, tgt);
//    if (!CheckResult(result))
//        return;
//
//    *_dst = std::vector<uint8_t>(  reinterpret_cast<const uint8_t*>(result.target.Data())
//                                 , reinterpret_cast<const uint8_t*>(result.target.Data()) + result.target.Size());
//}
//
//bool ShaderLoader::LinkSupport()
//{
//    return ShaderConductor::Compiler::LinkSupport();
//}


}// namespace shader
}// namespace buma3d
