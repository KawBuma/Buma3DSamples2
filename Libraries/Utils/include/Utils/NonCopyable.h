#pragma once

#define DECLARE_NON_COPYABLE(T) T(const T&) = delete; T& operator=(const T&) = delete

namespace buma
{
namespace util
{

class NonCopyable
{
public:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

};


} // namespace util
} // namespace buma
