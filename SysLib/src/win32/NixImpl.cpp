#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__MACH__)

#include "SysLib.h"
#include "CPPBase.hpp"
#include <cstdio>
#include <cstring>
#include <new>
#include <cstdarg>
#include <cmath>
#include <chrono>

extern "C" {

int InitSys(void)
{
    internal_GenerateCRC32Table();

    return 0;
}

void DestroySys(void)
{
}

void* Alloc(const size_t bytes)
{
    // Proxy to C malloc.
    return malloc(bytes);
}

void Free(void* data)
{
    // Proxy to C free
    return free(data);
}

TimeMs_t GetCurrentTimeMs(void)
{
    return internal_GetCurrentTimeMs();
}

void DebugPrintF(const PrintChar_t* fmt, ...)
{
    // Convert from a potential char8_t to char
#if USE_CHAR8_T
    const char* fmtC = reinterpret_cast<const char*>(fmt);
#else
    const char* fmtC = fmt;
#endif

    // The variadic accessor.
    va_list args;
    // Initialize the variadic accessor.
    va_start(args, fmt);

    vprintf(fmtC, args);

    // Finalize the variadic accessor.
    va_end(args);
}

uint32_t UpdateCRC32(const uint32_t initialCrc, const void* const data, const size_t length)
{
    return internal_UpdateCRC32(initialCrc, data, length);
}

size_t StringLengthC(const char* const string)
{
    return strlen(string);
}

#if USE_CHAR8_T
size_t StringLength8(const char8_t* const string)
{
    return strlen(reinterpret_cast<const char*>(string));
}
#endif

int StringCompareC(const char* const left, const char* const right)
{
    return strcmp(left, right);
}

#if USE_CHAR8_T
int StringCompare8(const char8_t* const left, const char8_t* const right)
{
    return strcmp(reinterpret_cast<const char*>(left), reinterpret_cast<const char*>(right));
}
#endif

void ZeroMem(void* data, const size_t length)
{
    (void) memset(data, 0, length);
}

void SeedRng(const int seed)
{
    internal_SeedRng(seed);
}

int GenerateRandomInt(const int min, const int max)
{
    return internal_GenerateRandomInt(min, max);
}

float GenerateRandomFloat(const float min, const float max)
{
    return internal_GenerateRandomFloat(min, max);
}

}

#endif
