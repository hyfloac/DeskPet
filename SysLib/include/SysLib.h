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

#ifdef __cplusplus
}
#endif
