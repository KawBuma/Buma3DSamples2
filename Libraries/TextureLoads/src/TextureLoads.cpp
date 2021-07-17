#include <TextureLoads/TextureLoads.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#endif

#ifndef STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#endif

#include <vector>
#include <filesystem>


namespace buma
{
namespace tex
{

namespace /*anonymous*/
{

inline size_t CalcMipLevels(size_t _w, size_t _h, size_t _d)
{
    return 1ull + static_cast<size_t>(floorf(log2f(static_cast<float>((std::max)({ _w, _h, _d })))));
}

inline size_t CalcMipLevels(const EXTENT3D& _extent)
{
    return 1ull + static_cast<size_t>(floorf(log2f(static_cast<float>((std::max)({ _extent.w, _extent.h, _extent.d })))));
}

inline EXTENT3D CalcMipExtents(size_t _mip_slice, const TEXTURE_DESC& _extent_mip0)
{
    return EXTENT3D{ (std::max)(_extent_mip0.width  >> _mip_slice, 1ull)
                   , (std::max)(_extent_mip0.height >> _mip_slice, 1ull)
                   , (std::max)(_extent_mip0.depth  >> _mip_slice, 1ull) };
}

inline size_t AlignUp(size_t _val, size_t _alignment)
{
    return (_val + (_alignment - 1)) & ~(_alignment - 1);
}

class RawData
{
public:
    RawData()
        : size_in_bytes {}
        , memory        {}
    {
    }
    ~RawData()
    {
        Free();
    }
    void Allocate(size_t _size_in_bytes)
    {
        assert(memory == nullptr);
        size_in_bytes = _size_in_bytes;
        memory        = (uint8_t*)malloc(size_in_bytes);
        std::fill(ui8, ui8 + size_in_bytes, 0);
    }
    void Free()
    {
        if (memory)
            free(memory);
        memory        = nullptr;
        size_in_bytes = 0;
    }

public:
    size_t size_in_bytes;
    union
    {
        uint8_t*    memory;
        uint8_t*    ui8;
        int8_t*     si8;
        float*      flt;
    };

};


}// namespace /*anonymous*/


class Textures : public ITextures
{
public:
    Textures()
        : file_desc     {}
        , desc          {}
        , textures_data {} 
        , raw_data      {}
    {
    }

    ~Textures()
    {
        Term();
    }

    static std::unique_ptr<ITextures> Create(const TEXTURE_CREATE_DESC& _desc);

    const TEXTURE_DATA*         Get(size_t _mip_slice = 0)  const override;
    const TEXTURE_DESC&         GetDesc()                   const override;
    const TEXTURE_FILE_DESC&    GetFileDesc()               const override;

private:
    bool Init(const TEXTURE_CREATE_DESC& _desc)
    {
        if (!PrepareFileDesc(_desc))
            return false;

        void* stbi_data{};
        if (!LoadFromFile(_desc, &stbi_data))
            return false;

        if (!CreateData(_desc, stbi_data))
            return false;

        stbi_image_free(stbi_data);
        stbi_data = nullptr;

        return true;
    }
    bool PrepareFileDesc(const TEXTURE_CREATE_DESC& _desc);
    bool LoadFromFile(const TEXTURE_CREATE_DESC& _desc, void** _stbi_data);
    bool CreateData(const TEXTURE_CREATE_DESC& _desc, void* _stbi_data);

    void Term()
    {
        file_desc       = {};
        desc            = {};
        textures_data   = {};
        raw_data        = {};
    }

private:
    TEXTURE_FILE_DESC           file_desc;
    TEXTURE_DESC                desc;
    std::vector<TEXTURE_DATA>   textures_data; // per mips
    std::vector<RawData>        raw_data;

};

std::unique_ptr<ITextures> Textures::Create(const TEXTURE_CREATE_DESC& _desc)
{
    if (!_desc.filename)
        return nullptr;

    auto result = std::make_unique<Textures>();
    if (!result->Init(_desc))
        return nullptr;

    return result;
}

const TEXTURE_DATA* Textures::Get(size_t _mip_slice) const
{
    if (_mip_slice > textures_data.size())
        return nullptr;

    return textures_data.data() + _mip_slice;
}

const TEXTURE_DESC& Textures::GetDesc() const
{
    return desc;
}

const TEXTURE_FILE_DESC& Textures::GetFileDesc() const
{
    return file_desc;
}

std::unique_ptr<ITextures> CreateTexturesFromFile(const TEXTURE_CREATE_DESC& _desc)
{
    return Textures::Create(_desc);
}

bool Textures::PrepareFileDesc(const TEXTURE_CREATE_DESC& _desc)
{
    file_desc.name = _desc.filename;
    std::filesystem::path file_path(file_desc.name);
    if (!std::filesystem::exists(file_path))
        return false;

         if (file_path.extension() == ".jpg")   file_desc.format = TEXTURE_FILE_FORMAT_JPG;
    else if (file_path.extension() == ".jpeg")  file_desc.format = TEXTURE_FILE_FORMAT_JPG;
    else if (file_path.extension() == ".png")   file_desc.format = TEXTURE_FILE_FORMAT_PNG;
    else if (file_path.extension() == ".tga")   file_desc.format = TEXTURE_FILE_FORMAT_TGA;
    else if (file_path.extension() == ".bmp")   file_desc.format = TEXTURE_FILE_FORMAT_BMP;
    else if (file_path.extension() == ".hdr")   file_desc.format = TEXTURE_FILE_FORMAT_HDR;
    else return false;

    return true;
}
bool Textures::LoadFromFile(const TEXTURE_CREATE_DESC& _desc, void** _stbi_data)
{
    TEXTURE_FORMAT  format          {};
    int             x               {};
    int             y               {};
    int             chs_in_file     {};
    int             component_size  {};
    int             req_comp        = STBI_default;

    // load
    {
        if (stbi_info(_desc.filename, &x, &y, &req_comp) == 0)
            return false;

        if (req_comp == STBI_rgb)
            req_comp = STBI_rgb_alpha;

        if (stbi_is_hdr(_desc.filename))
        {
            *_stbi_data     = stbi_loadf(_desc.filename, &x, &y, &chs_in_file, req_comp);
            component_size  = sizeof(float);
            format          = TEXTURE_FORMAT_SFLOAT;
        }
        else
        {
            *_stbi_data     = stbi_load(_desc.filename, &x, &y, &chs_in_file, req_comp);
            component_size  = sizeof(stbi_uc);
            format          = TEXTURE_FORMAT_UINT;
        }

        if (!(*_stbi_data))
            return false;
    }

    // desc
    {
        desc.width           = x;
        desc.height          = y;
        desc.depth           = 1;  // 現状1固定
        desc.num_mips        = _desc.mip_count == 0 ? CalcMipLevels(desc.width, desc.height, desc.depth) : _desc.mip_count;
        desc.format          = format;
        desc.component_size  = component_size;
        desc.component_count = static_cast<size_t>(req_comp == STBI_default ? chs_in_file : req_comp);
    }

    return true;
}
bool Textures::CreateData(const TEXTURE_CREATE_DESC& _desc, void* _stbi_data)
{
    raw_data     .resize(desc.num_mips);
    textures_data.resize(desc.num_mips);
    auto tds = textures_data.data();
    auto rds = raw_data     .data();

    for (size_t i = 0; i < desc.num_mips; i++)
    {
        auto&& td = tds[i];
        td.extent = CalcMipExtents(i, desc);

        auto&& l = td.layout;
        l.texel_size = desc.component_count * desc.component_size;

        auto row_pitch = l.texel_size * td.extent.w;

        l.row_pitch = row_pitch;
        if (_desc.row_pitch_alignment != 0)
            l.row_pitch = AlignUp(l.row_pitch, _desc.row_pitch_alignment);

        l.slice_pitch = l.row_pitch * td.extent.h;
        if (_desc.slice_pitch_alignment != 0)
            l.slice_pitch = AlignUp(l.slice_pitch, _desc.slice_pitch_alignment);

        auto&& r = rds[i];
        r.Allocate(l.slice_pitch * td.extent.d);
        td.total_size = r.size_in_bytes;
        td.data       = r.memory;

        if (i == 0)
        {
            for (size_t y = 0; y < td.extent.h; y++)
            {
                memcpy(r.memory + (td.layout.row_pitch * y), (uint8_t*)(_stbi_data)+(row_pitch * y), row_pitch);
            }
        }
        else
        {
            int result = 0;
            auto&& prev_td = tds[i-1];
            if (desc.format == TEXTURE_FORMAT_SFLOAT)
                result = stbir_resize_float(  rds[i-1].flt, static_cast<int>(prev_td.extent.w), static_cast<int>(prev_td.extent.h), static_cast<int>(prev_td.layout.row_pitch)
                                            , rds[i]  .flt, static_cast<int>(td.extent.w)     , static_cast<int>(td.extent.h)     , static_cast<int>(td.layout.row_pitch)
                                            , static_cast<int>(desc.component_count));
            else
                result = stbir_resize_uint8(  rds[i-1].ui8, static_cast<int>(prev_td.extent.w), static_cast<int>(prev_td.extent.h), static_cast<int>(prev_td.layout.row_pitch)
                                            , rds[i]  .ui8, static_cast<int>(td.extent.w)     , static_cast<int>(td.extent.h)     , static_cast<int>(td.layout.row_pitch)
                                            , static_cast<int>(desc.component_count));

            if (result == 0)
                return false;
        }

    }

    return true;
}


}// namespace tex
}// namespace buma
