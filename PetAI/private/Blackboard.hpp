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
    BlackboardKeyName(const uint32_t hash, const uint32_t length, const KeyChar* const string) noexcept
        : Hash(hash)
        , Length(length)
        , String(string)
    { }
public:
    uint32_t Hash;
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
    BlackboardKeyData(const BlackboardKey key, const BlackboardKeyName& name, const size_t dataSize) noexcept
        : Key(key)
        , Name(name)
        , DataSize(dataSize)
    { }
public:
    BlackboardKey Key;
    BlackboardKeyName Name;
    size_t DataSize;
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
    using TreeT = FastAVLTree<BlackboardKeyData, InsertMethod::Greater>;
public:
    BlackboardKeyManager() noexcept;

    BlackboardKey CalculateKey(const KeyChar* key, const size_t dataSize) noexcept;

    void StoreKey(BlackboardKey key, const BlackboardKeyName& name, const size_t dataSize) noexcept;

    [[nodiscard]] size_t TotalSize() const noexcept { return m_TotalSize; }
    [[nodiscard]] const TreeT& NameTree() const noexcept { return m_NameTree; }
private:
    TreeT m_NameTree;
    int32_t m_CurrentKeyIndex;
    size_t m_TotalSize;
};

class Blackboard final
{
    DELETE_CM(Blackboard);
public:
    Blackboard(const BlackboardKeyManager& keyManager) noexcept;

    ~Blackboard() noexcept;

    template<typename T>
    [[nodiscard]] T* GetT(const BlackboardKey key) noexcept
    {
        return static_cast<T*>(Get(key));
    }

    [[nodiscard]] void* Get(const BlackboardKey key) noexcept;

    [[nodiscard]] void* operator[](const BlackboardKey key) noexcept
    {
        return Get(key);
    }
private:
    void CountKeyCallback(const BlackboardKeyManager::TreeT::Node* node) noexcept;
    void AddOffsetsCallback(const BlackboardKeyManager::TreeT::Node* node) noexcept;
private:
    const BlackboardKeyManager* m_KeyManager;
    void* m_BlackboardData;
    size_t m_BlackboardSize;
    int32_t m_KeyCount;
    size_t* m_KeyOffsets;
    size_t m_CurrentOffset;
};
