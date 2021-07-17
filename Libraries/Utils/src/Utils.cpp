#include<Utils/Utils.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#error TODO: Support non-Windows enviroment
#endif // _WIN32


namespace buma
{
namespace util
{

std::string ConvertWideToCp(CODEPAGE _code_page, int _len_with_null_term, const wchar_t* _wstr)
{
#ifdef _WIN32
    auto l = WideCharToMultiByte(_code_page, 0, _wstr, _len_with_null_term, nullptr, 0, nullptr, FALSE);
#else

#endif // _WIN32
    if (l == 0)
        return std::string();

    std::string str(l - 1, '\0');// 結果のUnicode文字列にはnull終端文字があり、関数によって返される長さにはこの文字が含まれます。
    if (WideCharToMultiByte(_code_page, 0, _wstr, _len_with_null_term, str.data(), l, nullptr, FALSE) == 0)
        return std::string();

    return str;
}

std::wstring ConvertCpToWide(CODEPAGE _code_page, int _len_with_null_term, const char* _str)
{
#ifdef _WIN32
    auto l = MultiByteToWideChar(_code_page, 0, _str, _len_with_null_term, nullptr, 0);
#else

#endif // _WIN32
    if (l == 0)
        return std::wstring();

    std::wstring str(l - 1, L'\0');// 結果のUnicode文字列にはnull終端文字があり、関数によって返される長さにはこの文字が含まれます。
    if (MultiByteToWideChar(_code_page, 0, _str, _len_with_null_term, str.data(), l) == 0)
        return std::wstring();

    return str;
}


#if (defined BitScanForward && defined BitScanForward64 && defined BitScanReverse && defined BitScanReverse64)
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint32_t _bitmask) { return static_cast<uint8_t>(BitScanForward(_result_index, static_cast<unsigned long>(_bitmask))); }
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint64_t _bitmask) { return static_cast<uint8_t>(BitScanForward64(_result_index, static_cast<unsigned long long>(_bitmask))); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint32_t _bitmask) { return static_cast<uint8_t>(BitScanReverse(_result_index, static_cast<unsigned long>(_bitmask))); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint64_t _bitmask) { return static_cast<uint8_t>(BitScanReverse64(_result_index, static_cast<unsigned long long>(_bitmask))); }

#else
template <typename T>
inline uint8_t Buma3DBitScanForwardT(unsigned long* _result_index, T _bitmask)
{
    if (!_bitmask)
        return 0;

    unsigned long index = 0;
    while (index < sizeof(T) * 8)
    {
        if (_bitmask & (static_cast<T>(1) << index))
            break;
        index++;
    }

    *_result_index = index;
    return 1;
}

template <typename T>
inline uint8_t Buma3DBitScanReverseT(unsigned long* _result_index, T _bitmask)
{
    if (!_bitmask)
        return 0;

    unsigned long index = sizeof(T) * 8 - 1;
    while (index != ULONG_MAX)
    {
        if (_bitmask & (static_cast<T>(1) << index))
            break;
        index--;// オーバーフロー時に終了
    }

    *_result_index = index;
    return 1;
}

uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint32_t _bitmask) { return Buma3DBitScanForwardT<uint32_t>(_result_index, _bitmask); }
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint64_t _bitmask) { return Buma3DBitScanForwardT<uint64_t>(_result_index, _bitmask); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint32_t _bitmask) { return Buma3DBitScanReverseT<uint32_t>(_result_index, _bitmask); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint64_t _bitmask) { return Buma3DBitScanReverseT<uint64_t>(_result_index, _bitmask); }

#endif


}// namespace util
}// namespace buma
