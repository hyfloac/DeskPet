#include "Blackboard.hpp"

bool operator==(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    if(left.Crc32 != right.Crc32)
    {
        return false;
    }

    if(left.Length != right.Length)
    {
        return false;
    }

    return StringCompare(left.String, right.String) == 0;
}

bool operator!=(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    return !(left == right);
}

bool operator>(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    return right < left;
}

bool operator<(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    return StringCompare(left.String, right.String) < 0;
}

bool operator>=(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    return !(left < right);
}

bool operator<=(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    return !(right < left);
}

bool operator==(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept
{
    return left.Name == right.Name;
}

bool operator==(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept
{
    return left.Name == right;
}

bool operator==(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept
{
    return left == right.Name;
}

bool operator!=(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept
{
    return !(left == right);
}

bool operator!=(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept
{
    return !(left == right);
}

bool operator!=(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept
{
    return !(left == right);
}

bool operator>(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept
{
    return right < left;
}

bool operator>(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept
{
    return right < left;
}

bool operator>(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept
{
    return right < left;
}

bool operator<(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept
{
    return left.Name < right.Name;
}

bool operator<(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept
{
    return left.Name < right;
}

bool operator<(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept
{
    return left < right.Name;
}

bool operator>=(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept
{
    return !(left < right);
}

bool operator>=(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept
{
    return !(left < right);
}

bool operator>=(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept
{
    return !(left < right);
}

bool operator<=(const BlackboardKeyData& left, const BlackboardKeyData& right) noexcept
{
    return !(right < left);
}

bool operator<=(const BlackboardKeyData& left, const BlackboardKeyName& right) noexcept
{
    return !(right < left);
}

bool operator<=(const BlackboardKeyName& left, const BlackboardKeyData& right) noexcept
{
    return !(right < left);
}

BlackboardKeyManager::BlackboardKeyManager() noexcept
    : m_NameTree{}
    , m_CurrentKeyIndex(0)
{ }

BlackboardKey BlackboardKeyManager::CalculateKey(const KeyChar* const key) noexcept
{
    const size_t length = StringLength(key);
    const uint32_t crc32 = UpdateCRC32(0xFFFFFFFF, key, length);

    const BlackboardKeyName keyName(crc32, static_cast<uint32_t>(length), key);

    const BlackboardKeyData* const foundKey = m_NameTree.Find(keyName);

    if(foundKey)
    {
        return foundKey->Key;
    }

    const BlackboardKey retKey(m_CurrentKeyIndex++);

    m_NameTree.Emplace(retKey, keyName);

    return retKey;
}

void BlackboardKeyManager::StoreKey(BlackboardKey key, const BlackboardKeyName& name) noexcept
{
    m_NameTree.Emplace(key, name);
}
