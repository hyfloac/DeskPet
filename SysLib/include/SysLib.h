/**
 * @file 
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USE_CHAR8_T
  #define USE_CHAR8_T (1)
#endif

/**
 * \brief Represents a timestamp in milliseconds.
 */
typedef int64_t TimeMs_t;

#if USE_CHAR8_T
// ReSharper disable once CppUnusedIncludeDirective
#include <uchar.h>
typedef char8_t internal_PrintChar_t;
#else
typedef char internal_PrintChar_t;
#endif

/**
 * \brief The character type used for printing.
 *
 * The goal is for this to be UTF-8.
 */
typedef internal_PrintChar_t PrintChar_t;

/**
 * \brief Initializes the System Interface.
 * \return 0 on success.
 */
int InitSys(void);

/**
 * \brief Destroys the System Interface.
 */
void DestroySys(void);

/**
 * \brief Dynamically allocates a block of memory.
 * \param bytes The number of bytes to allocate.
 * \return The allocated buffer.
 */
void* Alloc(const size_t bytes);

/**
 * \brief Releases a dynamically allocated block of memory.
 * \param data The buffer to free.
 */
void Free(void* data);

/**
 * \return The current timestamp in milliseconds.
 */
TimeMs_t GetCurrentTimeMs(void);

void DebugPrintF(const PrintChar_t* fmt, ...);

uint32_t UpdateCRC32(uint32_t initial, const void* data, const size_t length);

size_t StringLengthC(const char* string);
#if USE_CHAR8_T
size_t StringLength8(const char8_t* string);
#endif

int StringCompareC(const char* left, const char* right);
#if USE_CHAR8_T
int StringCompare8(const char8_t* left, const char8_t* right);
#endif

void ZeroMem(void* data, const size_t length);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C++" {
[[nodiscard]] static inline size_t StringLength(const char* const string) noexcept
{
    return StringLengthC(string);
}
[[nodiscard]] static inline size_t StringLength(const char8_t* const string) noexcept
{
    return StringLength8(string);
}
template<typename CharT, size_t Length>
[[nodiscard]] static inline size_t StringLength(const CharT(&)[Length]) noexcept
{
    return Length - 1;
}

[[nodiscard]] static inline int StringCompare(const char* const left, const char* const right) noexcept
{
    return StringCompareC(left, right);
}
[[nodiscard]] static inline int StringCompare(const char8_t* const left, const char8_t* const right) noexcept
{
    return StringCompare8(left, right);
}
}
#endif
