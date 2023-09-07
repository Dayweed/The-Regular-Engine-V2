#pragma once
#include "pch.h"
#include "Core/Logger.h"

namespace TRE
{

#ifdef DEBUG
#define TRE_ENG_ASSERT(expr) \
        if (expr) {} \
        else \
        { \
            TRE_CORE_CRITICAL("Engine assertion failed! Expression: {} at {}:{}", #expr, __FILE__, __LINE__); \
            __debugbreak(); \
        }
#else
#define TRE_ENG_ASSERT(expr, msg)
#endif

#define TRE_ASSERT(expr, msg) \
    if (expr) {} \
    else \
    { \
        TRE_CRITICAL("[TRE_ASSERT] Assertion failed! ("#msg")" __FILE__" ("#expr") line: ", __LINE__);\
            __debugbreak(); \
        }

}
