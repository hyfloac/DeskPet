#pragma once

#include "Objects.hpp"
#include <SysLib.h>
#include "AVLTree.hpp"

struct BlackboardKey final
{
    DEFAULT_CONSTRUCT_PU(BlackboardKey);
    DEFAULT_DESTRUCT(BlackboardKey);
    DEFAULT_CM_PU(BlackboardKey);
public:
    BlackboardKey(const int32_t key) noexcept
        : Key(key)
    { }
public:
    int32_t Key;
};

struct BlackboardKeyName final
{
    DEFAULT_CONSTRUCT_PU(BlackboardKeyName);
    DEFAULT_DESTRUCT(BlackboardKeyName);
    DEFAULT_CM_PU(BlackboardKeyName);
public:
#if USE_CHAR8_T
    using KeyChar = char8_t;
#else
    using KeyChar = char;
#endif
public:
    BlackboardKeyName(const uint32_t crc32, const uint32_t length, const KeyChar* const string) noexcept
        : Crc32(crc32)
        , Length(length)
        , String(string)
    { }
public:
    uint32_t Crc32;
    uint32_t Length;
    const KeyChar* String;
};

[[nodiscard]] static bool operator==(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator!=(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator> (const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator< (const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator>=(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator<=(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept;

struct BlackboardKeyData final
{
    DEFAULT_CONSTRUCT_PU(BlackboardKeyData);
    DEFAULT_DESTRUCT(BlackboardKeyData);
    DEFAULT_CM_PU(BlackboardKeyData);
public:
    BlackboardKeyData(const BlackboardKey key, const BlackboardKeyName& name) noexcept
        : Key(key)
        , Name(name)
    { }
public:
    BlackboardKey Key;
    BlackboardKeyName Name;
};

[[nodiscard]] static bool operator==(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept;
[[nodiscard]] static bool operator==(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator==(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept;

[[nodiscard]] static bool operator!=(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept;
[[nodiscard]] static bool operator!=(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator!=(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept;

[[nodiscard]] static bool operator>(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept;
[[nodiscard]] static bool operator>(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator>(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept;

[[nodiscard]] static bool operator<(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept;
[[nodiscard]] static bool operator<(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator<(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept;

[[nodiscard]] static bool operator>=(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept;
[[nodiscard]] static bool operator>=(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator>=(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept;

[[nodiscard]] static bool operator<=(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept;
[[nodiscard]] static bool operator<=(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept;
[[nodiscard]] static bool operator<=(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept;

class BlackboardKeyManager final
{
    DEFAULT_DESTRUCT(BlackboardKeyManager);
    DELETE_CM(BlackboardKeyManager);
public:
    using KeyChar = BlackboardKeyName::KeyChar;
public:
    BlackboardKeyManager() noexcept;

    BlackboardKey CalculateKey(const KeyChar* key) noexcept;

    void StoreKey(BlackboardKey key, const BlackboardKeyName& name) noexcept;
private:
    FastAVLTree<BlackboardKeyData, InsertMethod::Greater> m_NameTree;
    int32_t m_CurrentKeyIndex;
};

class Blackboard final
{
    DEFAULT_CONSTRUCT_PU(Blackboard);
    DEFAULT_DESTRUCT(Blackboard);
    DELETE_CM(Blackboard);
public:

};
