#pragma once
#include <Utils/Compiler.h>

#ifdef BUMA_DEBUG
#include <cassert>
#define BUMA_ASSERT(x) assert(x)
#else
#define BUMA_ASSERT(x) 
#endif
