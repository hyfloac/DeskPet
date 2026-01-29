#pragma once

#if defined(_WIN32) || defined(unix) || defined(__unix__) || defined(__unix) || defined(__MACH__)

#include "SysLib.h"

[[nodiscard]] TimeMs_t internal_GetCurrentTimeMs();

[[nodiscard]] uint32_t internal_UpdateCRC32(const uint32_t initialCrc, const void* const data, const size_t length) noexcept;

void internal_GenerateCRC32Table() noexcept;

void internal_SeedRng(const int seed) noexcept;

[[nodiscard]] int internal_FastRand() noexcept;

[[nodiscard]] int internal_GenerateRandomInt(const int min, const int max) noexcept;

[[nodiscard]] float internal_GenerateRandomFloat(const float min, const float max) noexcept;


#endif
