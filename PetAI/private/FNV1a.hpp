#pragma once

#include <cstdint>

namespace Victoria {

/**
*   This function pre-computes the FNV-1a (32-bit) hash for a given string (all upper case).
*   @param aString The input string to be hashed.
*   @param val The initial hash value (default is 0x811C9DC5).
*   @return The computed 32-bit FNV-1a hash value.
*
*    The FNV-1a hash algorithm is a non-cryptographic hash function created by
*    Glenn Fowler, Landon Curt Noll, and Phong Vo.  It is designed to be fast
*    and simple while providing a good distribution of hash values for different
*    inputs.
*
*    The function takes a string and an optional initial hash value as input and
*    returns a 32-bit hash value.  The input string is processed character by
*    character, and each character is converted to uppercase if it is a lowercase
*    letter.  The hash value is updated by XORing it with the character value and
*    then multiplying it by the FNV prime (0x01000193).  The process continues
*    until the end of the string is reached, at which point the final hash value
*    is returned.
*
*    Source https://gist.github.com/filsinger/1255697/21762ea83a2d3c17561c8e6a29f44249a4626f9e
*
*    See https://en.m.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function for more details on this hash function.
*
*/
template<typename CharT>
constexpr static inline uint32_t FNV1A(const CharT* const aString, const uint32_t val = 0x811C9DC5)
{
    if(aString[0] == '\0')
    {
        return val;
    }

    const uint32_t c = static_cast<uint32_t>((aString[0] >= 'a' && aString[0] <= 'z') ? (aString[0] - 'a' + 'A') : aString[0]);

    // Convert the string to upper case and compute the FNV-1a hash.
    return FNV1A<CharT>(&aString[1], (val ^ c) * 0x01000193);
}

}
