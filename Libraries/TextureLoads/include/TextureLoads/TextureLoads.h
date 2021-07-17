#pragma once

#include <string>
#include <memory>

namespace buma
{
namespace tex
{

enum TEXTURE_FILE_FORMAT
{
      TEXTURE_FILE_FORMAT_JPG // .jpg (including .jpeg)
    , TEXTURE_FILE_FORMAT_PNG // .png
    , TEXTURE_FILE_FORMAT_TGA // .tga
    , TEXTURE_FILE_FORMAT_BMP // .bmp
    , TEXTURE_FILE_FORMAT_HDR // .hdr
};

struct TEXTURE_FILE_DESC
{
    std::string         name;
    TEXTURE_FILE_FORMAT format;
};

enum TEXTURE_FORMAT
{
      TEXTURE_FORMAT_UNSPECIFIED
    , TEXTURE_FORMAT_SINT
    , TEXTURE_FORMAT_UINT
    , TEXTURE_FORMAT_SFLOAT
};

struct EXTENT3D
{
    size_t w;
    size_t h;
    size_t d;
};

struct TEXTURE_DESC
{
    size_t          width;
    size_t          height;
    size_t          depth;              // (array size)
    size_t          num_mips;
    TEXTURE_FORMAT  format;
    size_t          component_count;    // 4 := xyzw
    size_t          component_size;     // in bytes
};

struct TEXTURE_LAYOUT
{
    size_t texel_size;  // component_count * component_size
    size_t row_pitch;   // texel_size * width
    size_t slice_pitch; // row_pitch * height
};

struct TEXTURE_DATA
{
    size_t          total_size;
    const void*     data;
    TEXTURE_LAYOUT  layout;
    EXTENT3D        extent;

    template<typename T>
    const T* Get(size_t _depth_index = 0)
    {
        uint8_t* d = reinterpret_cast<const uint8_t*>(data);
        d += layout.slice_pitch * _depth_index;

        return reinterpret_cast<const T*>(d);
    }

};

struct ITextures
{
    virtual ~ITextures() {}

    virtual const TEXTURE_DATA*         Get(size_t _mip_slice = 0)  const = 0;
    virtual const TEXTURE_DESC&         GetDesc()                   const = 0;
    virtual const TEXTURE_FILE_DESC&    GetFileDesc()               const = 0;

};

struct TEXTURE_CREATE_DESC
{
    TEXTURE_CREATE_DESC() = default;
    TEXTURE_CREATE_DESC(const char* _filename, size_t _mip_count = 1, size_t _row_pitch_alignment = 0, size_t _slice_pitch_alignment = 0)
        : filename{ _filename }, mip_count{ _mip_count }, row_pitch_alignment{ _row_pitch_alignment }, slice_pitch_alignment{ _slice_pitch_alignment } {}

    const char*    filename;
    size_t         mip_count;               // set 0 to generate all mips
    size_t         row_pitch_alignment;     // set 0 to follows the texture resolution
    size_t         slice_pitch_alignment;   // set 0 to follows the texture resolution
};
std::unique_ptr<ITextures> CreateTexturesFromFile(const TEXTURE_CREATE_DESC& _desc);


}// namespace tex
}// namespace buma
