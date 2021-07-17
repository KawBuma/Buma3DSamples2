#pragma once
#include <cassert>
#include <string>
#include <sstream>
#include <iomanip>

namespace buma
{
namespace util
{

template<typename T>
inline void SafeDeleteAry(T*& _p)
{
    if (_p)
    {
        delete[] _p;
        _p = nullptr;
    }
}

template<typename T>
inline void SafeDelete(T*& _p)
{
    if (_p)
    {
        delete _p;
        _p = nullptr;
    }
}

template<typename T>
inline uint32_t SafeRelease(T*& _p)
{
    auto result = (uint32_t)(_p ? _p->Release() : 0);
    _p = nullptr;
    return result;
}
template<typename T>
inline T* SafeAddRef(T*& _p)
{
    if (!_p) return;
    _p->AddRef();
    return _p;
}

#pragma region containerhelper

template <typename T>
inline void SwapClear(T& _container)
{
    { T().swap(_container); }
}

template <typename T>
inline typename T::iterator EraseContainerElem(T& _container, const size_t _erase_pos)
{
    return _container.erase(_container.begin() + _erase_pos);
}

// _first_pos: 0~, _last_pos: _container.size()までの間で設定してください
template <typename T>
inline typename T::iterator EraseContainerRange(T& _container, const size_t _first_pos, const size_t _last_pos)
{
    typename T::const_iterator it = _container.begin();
    return _container.erase(it + _first_pos, it + _last_pos);
}

template <typename T>
inline typename T::iterator InsertContainerElem(T& _container, const size_t _insert_pos, const typename T::value_type& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _value);
}

template <typename T>
inline typename T::iterator InsertContainerElem(T& _container, const size_t _insert_pos, typename T::value_type&& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _value);
}

template <typename T>
inline typename T::iterator InsertContainerElemCount(T& _container, const size_t _insert_pos, const size_t _insert_count, const typename T::value_type& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _insert_count, _value);
}

template <typename T>
inline typename T::iterator InsertContainerElemCount(T& _container, const size_t _insert_pos, const size_t _insert_count, typename T::value_type&& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _insert_count, _value);
}

// _insert_first: 0 ~ _insert_container.size()までの間で設定してください
// _insert_last: 0 ~ _insert_container.size()までの間で設定してください
// _insert_firstと _insert_lastが同じの場合要素は挿入されません
template <typename T>
inline typename T::iterator InsertContainerElemRange(T& _container, const size_t _insert_pos, T& _insert_container, const size_t _insert_first, const size_t _insert_last)
{
    typename T::iterator ins_it = _insert_container.begin();
    return _container.insert(_container.begin() + _insert_pos, ins_it + _insert_first, ins_it + _insert_last);
}

#pragma endregion containerhelper

inline std::string GetUUIDString(const uint8_t _uuid[16])
{
#define B3DFW std::setfill('0') << std::setw(2) 
    std::stringstream ss;
    ss << std::hex 
        << B3DFW << (uint32_t)_uuid[0]  << B3DFW << (uint32_t)_uuid[1] << B3DFW << (uint32_t)_uuid[2] << B3DFW << (uint32_t)_uuid[3] << "-"
        << B3DFW << (uint32_t)_uuid[4]  << B3DFW << (uint32_t)_uuid[5] << '-'
        << B3DFW << (uint32_t)_uuid[6]  << B3DFW << (uint32_t)_uuid[7] << '-'
        << B3DFW << (uint32_t)_uuid[8]  << B3DFW << (uint32_t)_uuid[9] << '-'
        << B3DFW << (uint32_t)_uuid[10] << B3DFW << (uint32_t)_uuid[11] << B3DFW << (uint32_t)_uuid[12] << B3DFW << (uint32_t)_uuid[13] << B3DFW << (uint32_t)_uuid[14] << B3DFW << (uint32_t)_uuid[15]
        << std::dec;
    return ss.str();
#undef B3DFW
}

inline std::string GetLUIDString(const uint8_t _luid[8])
{
#define B3DFW std::setfill('0') << std::setw(2) 
    std::stringstream ss;
    ss << std::hex
        << "Low: "    << B3DFW << (uint32_t)_luid[0] << B3DFW << (uint32_t)_luid[1] << B3DFW << (uint32_t)_luid[2] << B3DFW << (uint32_t)_luid[3]
        << ", High: " << B3DFW << (uint32_t)_luid[4] << B3DFW << (uint32_t)_luid[5] << B3DFW << (uint32_t)_luid[6] << B3DFW << (uint32_t)_luid[7]
        << std::dec;
    return ss.str();
#undef B3DFW
}

enum CODEPAGE : uint32_t
{
      CODEPAGE_ACP        = 0        // default to ANSI code page
    , CODEPAGE_OEMCP      = 1        // default to OEM  code page
    , CODEPAGE_MACCP      = 2        // default to MAC  code page
    , CODEPAGE_THREAD_ACP = 3        // current thread's ANSI code page
    , CODEPAGE_SYMBOL     = 42       // SYMBOL translations

    , CODEPAGE_UTF7       = 65000    // UTF-7 translation
    , CODEPAGE_UTF8       = 65001    // UTF-8 translation
};

std::string ConvertWideToCp(CODEPAGE _code_page /*= CP_UTF8*/, int _len_with_null_term, const wchar_t* _wstr);
std::wstring ConvertCpToWide(CODEPAGE _code_page /*= CP_UTF8*/, int _len_with_null_term, const char* _str);

inline std::string  ConvertWideToCp  (CODEPAGE _code_page, const std::wstring& _wstr)    { return ConvertWideToCp(_code_page, int(_wstr.size() + 1ull), _wstr.c_str()); }
inline std::wstring ConvertCpToWide  (CODEPAGE _code_page, const std::string& _str)      { return ConvertCpToWide(_code_page, int(_str.size() + 1ull), _str.c_str()); }

inline std::wstring ConvertUtf8ToWide(const char*          _str)                          { return ConvertCpToWide(CODEPAGE_UTF8, _str); }
inline std::wstring ConvertUtf8ToWide(const std::string&  _str)                           { return ConvertCpToWide(CODEPAGE_UTF8, int(_str.size() + 1ull), _str.c_str()); }
inline std::string  ConvertWideToUtf8(const wchar_t*       _wstr)                         { return ConvertWideToCp(CODEPAGE_UTF8, _wstr); }
inline std::string  ConvertWideToUtf8(const std::wstring& _wstr)                          { return ConvertWideToCp(CODEPAGE_UTF8, int(_wstr.size() + 1ull), _wstr.c_str()); }

inline std::wstring ConvertAnsiToWide(const char*          _str)                          { return ConvertCpToWide(CODEPAGE_ACP, _str); }
inline std::wstring ConvertAnsiToWide(const std::string&  _str)                           { return ConvertCpToWide(CODEPAGE_ACP, int(_str.size() + 1ull), _str.c_str()); }
inline std::string  ConvertWideToAnsi(const wchar_t*       _wstr)                         { return ConvertWideToCp(CODEPAGE_ACP, _wstr); }
inline std::string  ConvertWideToAnsi(const std::wstring& _wstr)                          { return ConvertWideToCp(CODEPAGE_ACP, int(_wstr.size() + 1ull), _wstr.c_str()); }

#pragma region valhelper

template <typename T>
inline constexpr T AlignUpWithMask(T _value, size_t _mask)
{
    return static_cast<T>((static_cast<size_t>(_value) + _mask) & ~_mask);
}

template <typename T>
inline constexpr T AlignDownWithMask(T _value, size_t _mask)
{
    return static_cast<T>(static_cast<size_t>(_value) & ~_mask);
}

template <typename T>
inline constexpr T AlignUp(T _value, size_t _alignment)
{
    return AlignUpWithMask(_value, _alignment - 1);
}

template <typename T>
inline constexpr T AlignDown(T _value, size_t _alignment)
{
    return AlignDownWithMask(_value, _alignment - 1);
}

template <typename T>
inline constexpr bool IsAligned(T _value, size_t _alignment)
{
    return (static_cast<size_t>(_value) & (_alignment - 1)) == 0;
}

template <typename T>
inline constexpr T DivideByMultiple(T _value, size_t _alignment)
{
    return static_cast<T>((static_cast<size_t>(_value) + _alignment - 1) / _alignment);
}

template<typename T, typename RetT = size_t>
inline constexpr RetT Get32BitValues()
{
    return AlignUp(sizeof(T), 4) / 4;
}

uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint32_t _bitmask);
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint64_t _bitmask);
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint32_t _bitmask);
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint64_t _bitmask);

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 0>
inline int GetFirstBitIndex(T _bits)
{
    unsigned long index = 0;
    auto res = Buma3DBitScanForward(&index, static_cast<uint32_t>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 0>
inline int GetFirstBitIndex(T _bits)
{
    unsigned long index = 0;
    auto res = Buma3DBitScanForward(&index, static_cast<uint64_t>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 0>
inline int GetLastBitIndex(T _bits)
{
    unsigned long index = 0;
    auto res = Buma3DBitScanReverse(&index, static_cast<uint32_t>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 0>
inline int GetLastBitIndex(T _bits)
{
    unsigned long index = 0;
    auto res = Buma3DBitScanReverse(&index, static_cast<uint64_t>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template<typename T>
inline T Log2(T _value)
{
    if (!_value) return 0;

    int mssb = GetLastBitIndex(_value);  // most significant set bit
    int lssb = GetFirstBitIndex(_value); // least significant set bit
    if (mssb == -1 || lssb == -1)
        return 0;

    // 2の累乗（1セットビットのみ）の場合、ビットのインデックスを返します。
    // それ以外の場合は、最上位のセットビットのインデックスに1を加算して、小数ログを切り上げます。
    return static_cast<T>(mssb) + static_cast<T>(mssb == lssb ? 0 : 1);
}

template<typename T>
inline constexpr T Log2Cexpr(T _value)
{
    if (!_value) return 0;
    int mssb = 0, lssb = 0, cnt = 0;

    cnt = (sizeof(T) * 8) - 1;
    while (cnt != -1) { if (_value & static_cast<T>(1ull << cnt)) break; cnt--; }
    mssb = cnt;

    cnt = 0;
    while (cnt < sizeof(T) * 8) { if (_value & static_cast<T>(1ull << cnt)) break; cnt++; }
    lssb = cnt;

    return static_cast<T>(mssb) + static_cast<T>(mssb == lssb ? 0 : 1);
}

template <typename T>
inline T NextPow2(T _value)
{
    return _value == 0 ? 0 : 1 << Log2(_value);
}

template <typename T>
inline bool IsPowOfTwo(T _value)
{
    return _value > 0 && (_value & (_value - 1)) == 0;
}

inline constexpr size_t Kib(size_t _x) { return 1024 * _x; }
inline constexpr size_t Mib(size_t _x) { return Kib(1024) * _x; }
inline constexpr size_t Gib(size_t _x) { return Mib(1024) * _x; }


#pragma endregion valhelper


}// namespace util
}// namespace buma
