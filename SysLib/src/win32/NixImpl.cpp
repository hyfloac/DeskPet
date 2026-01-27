#ifndef _WIN32

#include "SysLib.h"
#include <cstdio>
#include <cstring>
#include <new>
#include <cstdarg>
#include <cmath>
#include <chrono>

static uint32_t Crc32Table[256];

static void GenerateCRC32Table() noexcept;

extern "C" {

int InitSys(void)
{
    GenerateCRC32Table();

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
    return ::std::chrono::duration_cast<::std::chrono::milliseconds>(::std::chrono::system_clock::now().time_since_epoch()).count();
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
    uint32_t crc32 = initialCrc ^ 0xFFFFFFFF;
    const uint8_t* const dataBytes = static_cast<const uint8_t*>(data);

    for(size_t i = 0; i < length; ++i)
    {
        crc32 = Crc32Table[(crc32 ^ dataBytes[i]) & 0xFF] ^ (crc32 >> 8);
    }

    return crc32 ^ 0xFFFFFFFF;
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

static unsigned int s_seed = 0;

void SeedRng(const int seed)
{
    s_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
static int fast_rand()
{
    s_seed = (214013 * s_seed + 2531011);
    return static_cast<int>((s_seed >> 16) & 0x7FFF);
}

int GenerateRandomInt(const int min, const int max)
{
    int rand = fast_rand();

    if(::std::abs(max - min) >= 32768)
    {
        rand |= fast_rand() << 16;
    }

    return rand % ::std::abs(max - min + 1) + min;
}

float GenerateRandomFloat(const float min, const float max)
{
    constexpr int maxValue = 0x3FFFFFFF;
    const int rand = fast_rand() << 15 | fast_rand();

    const float randF = static_cast<float>(rand) / static_cast<float>(maxValue);

    return (randF * (max - min + 1)) + min;
}

}

static void GenerateCRC32Table() noexcept
{
    constexpr uint32_t polynomial = 0xEDB88320;

    for(uint32_t i = 0; i < 256; ++i)
    {
        uint32_t c = i;
        for(size_t j = 0; j < 8; ++j)
        {
            if((c & 1) == 1)
            {
                c = polynomial ^ (c >> 1);
            }
            else
            {
                c >>= 1;
            }
        }

        Crc32Table[i] = c;
    }
}

#endif
