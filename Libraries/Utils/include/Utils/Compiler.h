#pragma once

#if !defined(NDEBUG)
#define BUMA_DEBUG
#endif // !defined(NDEBUG)

#define BUMA_UNREDERENCED(...) (__VA_ARGS__)

namespace buma
{

#ifdef BUMA_DEBUG
inline constexpr bool IS_DEBUG = true;
#else
inline constexpr bool IS_DEBUG = false;
#endif // BUMA_DEBUG

} // namespace buma
