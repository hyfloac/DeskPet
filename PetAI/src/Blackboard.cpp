#include "Blackboard.hpp"
#include "FNV1a.hpp"

bool operator==(const BlackboardKeyName& left, const BlackboardKeyName& right) noexcept
{
    if(left.Hash != right.Hash)
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
    , m_TotalSize(0)
{ }

BlackboardKey BlackboardKeyManager::CalculateKey(const KeyChar* const key, const size_t dataSize) noexcept
{
    const size_t length = StringLength(key);
    const uint32_t hash = Victoria::FNV1A(key);

    const BlackboardKeyName keyName(hash, static_cast<uint32_t>(length), key);

    const BlackboardKeyData* const foundKey = m_NameTree.Find(keyName);

    if(foundKey)
    {
        return foundKey->Key;
    }

    const BlackboardKey retKey(m_CurrentKeyIndex++);

    m_NameTree.Emplace(retKey, keyName, dataSize);
    m_TotalSize += dataSize;

    return retKey;
}

void BlackboardKeyManager::StoreKey(BlackboardKey key, const BlackboardKeyName& name, const size_t dataSize) noexcept
{
    m_NameTree.Emplace(key, name, dataSize);
    m_TotalSize += dataSize;
}

Blackboard::Blackboard(const BlackboardKeyManager& keyManager) noexcept
    : m_KeyManager(&keyManager)
    , m_BlackboardData(Alloc(keyManager.TotalSize()))
    , m_BlackboardSize(keyManager.TotalSize())
    , m_KeyCount(0)
    , m_KeyOffsets(nullptr)
    , m_CurrentOffset(0)
{
    keyManager.NameTree().Iterate(this, &Blackboard::CountKeyCallback);

    m_KeyOffsets = new size_t[m_KeyCount];

    keyManager.NameTree().Iterate<Blackboard, decltype(&Blackboard::AddOffsetsCallback), IteratorMethod::LowestToHighest>(this, &Blackboard::AddOffsetsCallback);
}

void Blackboard::CountKeyCallback(const BlackboardKeyManager::TreeT::Node* node) noexcept
{
    ++m_KeyCount;
}

void Blackboard::AddOffsetsCallback(const BlackboardKeyManager::TreeT::Node* node) noexcept
{
    if(!node)
    {
        return;
    }

    m_KeyOffsets[node->Value.Key.Key] = m_CurrentOffset;
    m_CurrentOffset += node->Value.DataSize;
}

void* Blackboard::Get(const BlackboardKey key) noexcept
{
    if(key.Key >= m_KeyCount)
    {
        return nullptr;
    }

    const size_t offset = m_KeyOffsets[key.Key];

    if(offset >= m_BlackboardSize)
    {
        return nullptr;
    }

    unsigned char* byteStream = static_cast<unsigned char*>(m_BlackboardData);

    return &byteStream[offset];
}
