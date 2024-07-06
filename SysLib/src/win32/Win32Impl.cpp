#ifdef _WIN32

#include "SysLib.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include <new>
#include <varargs.h>
#include <cmath>

static UINT OriginalConsoleOutputCP = 0;

static HANDLE OriginalConsoleOutputHandle = nullptr;
static HANDLE OriginalConsoleErrorHandle = nullptr;

static DWORD OriginalConsoleOutputMode = 0;
static DWORD OriginalConsoleErrorMode = 0;

// This is not always defined.
#ifndef CP_UTF16LE
  #define CP_UTF16LE (1200)
#endif

// We want line-feeds handled, line wrapping and colors.
// Colors aren't necessary, but it supports it, so might as well.
#define CONSOLE_OUT_DEFAULT_ARGS \
   (ENABLE_PROCESSED_OUTPUT | \
    ENABLE_WRAP_AT_EOL_OUTPUT | \
    ENABLE_VIRTUAL_TERMINAL_PROCESSING)

static uint32_t Crc32Table[256];

static void GenerateCRC32Table() noexcept;

extern "C" {

int InitSys(void)
{
    // Cache the original console code page.
    OriginalConsoleOutputCP = GetConsoleOutputCP();

    // Get the handle to the regular console output.
    OriginalConsoleOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    // Get the handle to the error console output.
    OriginalConsoleErrorHandle = GetStdHandle(STD_ERROR_HANDLE);

    // Cache the original regular console mode.
    (void) GetConsoleMode(OriginalConsoleOutputHandle, &OriginalConsoleOutputMode);
    // Cache the original error console mode.
    (void) GetConsoleMode(OriginalConsoleErrorHandle, &OriginalConsoleErrorMode);

    // Set the code page to UTF-16 (Little Endian).
    // Ideally we should use UTF-8, support is pretty good, for output at least.
    (void) SetConsoleOutputCP(CP_UTF16LE);

    // Set our regular console mode.
    (void) SetConsoleMode(OriginalConsoleOutputHandle, CONSOLE_OUT_DEFAULT_ARGS);
    // Set our error console mode.
    (void) SetConsoleMode(OriginalConsoleErrorHandle, CONSOLE_OUT_DEFAULT_ARGS);

    GenerateCRC32Table();

    return 0;
}

void DestroySys(void)
{
    (void) SetConsoleMode(OriginalConsoleErrorHandle, OriginalConsoleErrorMode);
    (void) SetConsoleMode(OriginalConsoleOutputHandle, OriginalConsoleOutputMode);

    (void) SetConsoleOutputCP(OriginalConsoleOutputCP);
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
    // The composite 64-bit integer for the file time.
    FILETIME fileTime;
    // Get the system time at a 100ns resolution.
    GetSystemTimePreciseAsFileTime(&fileTime);

    // Merge the 2 32-bit integers into a single 64-bit integer.
    const int64_t time64Ns100 = (static_cast<int64_t>(fileTime.dwHighDateTime) << 32) | static_cast<int64_t>(fileTime.dwLowDateTime);
    // Convert the timestamp from a resolution of 100ns to 1ms.
    const TimeMs_t time64Ms = static_cast<TimeMs_t>(time64Ns100)/ static_cast<TimeMs_t>(10 * 1000);

    return time64Ms;
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

    // Get the required buffer size for the formatted string.
    const size_t bufferSize = _vscprintf(fmtC, args) + 1;

    // Finalize the variadic accessor.
    va_end(args);

    // Allocate a buffer for the formatted string.
    char* formatBuffer = reinterpret_cast<char*>(Alloc(bufferSize * sizeof(char)));
    // Set the last character to a null terminator.
    // I don't feel like looking up the nuance of how vsprintf_s handles null termination...
    formatBuffer[bufferSize - 1] = '\0';

    // Re-initialize the variadic accessor.
    va_start(args, fmt);

    // Format the string.
    (void) vsprintf_s(formatBuffer, bufferSize, fmtC, args);

    // Finalize the variadic accessor.
    va_end(args);

    // Get the required buffer size for the UTF-16 string.
    const int wideBufferSize = MultiByteToWideChar(CP_UTF8, 0, formatBuffer, static_cast<int>(bufferSize), nullptr, 0);

    // Allocate a buffer for the UTF-16 string.
    wchar_t* wideBuffer = reinterpret_cast<wchar_t*>(Alloc((wideBufferSize + 1) * sizeof(wchar_t)));
    // Set the last character to a null terminator.
    // I don't feel like looking up the nuance of how MultiByteToWideChar handles null termination...
    wideBuffer[wideBufferSize] = u'\0';

    // Convert the UTF-8ish formatted string to UTF-16/UCS2.
    (void) MultiByteToWideChar(CP_UTF8, 0, formatBuffer, static_cast<int>(bufferSize), wideBuffer, wideBufferSize);

    // Release the formatted string, as it is no longer needed.
    Free(formatBuffer);

    // Write the UTF-16 string to the console.
    (void) WriteConsoleW(OriginalConsoleOutputHandle, wideBuffer, wideBufferSize, nullptr, nullptr);

    // Release the UTF-16 string.
    Free(wideBuffer);
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
