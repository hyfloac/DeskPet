#if defined(_WIN32) || defined(unix) || defined(__unix__) || defined(__unix) || defined(__MACH__)

#include "CPPBase.hpp"
#include <cstdlib>
#include <chrono>

static uint32_t s_Crc32Table[256];
static unsigned int s_Seed = 0;

[[nodiscard]] TimeMs_t internal_GetCurrentTimeMs()
{
    return ::std::chrono::duration_cast<::std::chrono::milliseconds>(::std::chrono::system_clock::now().time_since_epoch()).count();
}

[[nodiscard]] uint32_t internal_UpdateCRC32(const uint32_t initialCrc, const void* const data, const size_t length) noexcept
{
    uint32_t crc32 = initialCrc ^ 0xFFFFFFFF;
    const uint8_t* const dataBytes = static_cast<const uint8_t*>(data);

    for(size_t i = 0; i < length; ++i)
    {
        crc32 = s_Crc32Table[(crc32 ^ dataBytes[i]) & 0xFF] ^ (crc32 >> 8);
    }

    return crc32 ^ 0xFFFFFFFF;
}

void internal_GenerateCRC32Table() noexcept
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

        s_Crc32Table[i] = c;
    }
}

void internal_SeedRng(const int seed) noexcept
{
    s_Seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
[[nodiscard]] int internal_FastRand() noexcept
{
    s_Seed = (214013 * s_Seed + 2531011);
    return static_cast<int>((s_Seed >> 16) & 0x7FFF);
}

[[nodiscard]] int internal_GenerateRandomInt(const int min, const int max) noexcept
{
    int rand = internal_FastRand();

    if(::std::abs(max - min) >= 32768)
    {
        rand |= internal_FastRand() << 16;
    }

    return rand % ::std::abs(max - min + 1) + min;
}

[[nodiscard]] float internal_GenerateRandomFloat(const float min, const float max) noexcept
{
    constexpr int maxValue = 0x3FFFFFFF;
    const int rand = internal_FastRand() << 15 | internal_FastRand();

    const float randF = static_cast<float>(rand) / static_cast<float>(maxValue);

    return (randF * (max - min + 1)) + min;
}

#endif
