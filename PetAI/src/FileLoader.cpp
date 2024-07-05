#include "FileLoader.hpp"
#include <cstring>
#include <algorithm>
#include <iterator>

template<typename T>
void FileUtils::FlipEndian(T& val) noexcept
{
    uint8_t tmpBlock[sizeof(val)];

    (void) ::std::memcpy(tmpBlock, &val, sizeof(val));
    ::std::reverse(::std::begin(tmpBlock), ::std::end(tmpBlock));
    (void) ::std::memcpy(&val, tmpBlock, sizeof(val));
}

FileBlockHandle FileBlock::Acquire(uint8_t*& receivingBlock) noexcept
{
    if(m_Possessed)
    {
        return FileBlockHandle {};
    }

    m_Possessed = true;

    return FileBlockHandle(*this, receivingBlock);
}

static FileBlock BlackboardKeyBlock;
static FileBlock BlackboardKeyStringBlock;

#pragma pack(push, 1)
struct BlackboardKeyIndice final
{
    int32_t Key;
    uint32_t Hash;
    uint32_t NameOffset;
};
#pragma pack(pop)

PetStatus BlackboardKeyLoader::Load(BlackboardKeyManager& keyManager, PetManager& petManager) noexcept
{
    if(!petManager.AppFunctions()->LoadPetState)
    {
        return PetSuccess;
    }

    FileBlockHandle block = BlackboardKeyBlock.Acquire(m_DataBlock);

    if(!block)
    {
        return PetFail;
    }

    m_FileLength = 0;
    m_CurrentOffset = 0;
    m_FileOffset = 0;
    m_Length = 0;

    PetStatus status = petManager.AppFunctions()->LoadPetState(petManager.AppHandle(), BlackboardKeyFileHandle, 0, nullptr, &m_FileLength);

    if(!IsStatusSuccess(status))
    {
        if(status == PetInvalidArg)
        {
            return PetFail;
        }

        return status;
    }

    status = petManager.AppFunctions()->LoadPetState(petManager.AppHandle(), BlackboardKeyFileHandle, m_FileOffset, m_DataBlock, &m_Length);

    if(!IsStatusSuccess(status))
    {
        if(status == PetInvalidArg)
        {
            return PetFail;
        }

        return status;
    }

    m_FileOffset += m_Length;

    if(m_Length < sizeof(BlackboardKeyFileBaseHeader))
    {
        return PetFail;
    }

    BlackboardKeyFileBaseHeader* const baseHeader = reinterpret_cast<BlackboardKeyFileBaseHeader*>(m_DataBlock);

    m_CurrentOffset += sizeof(BlackboardKeyFileBaseHeader);

    if(baseHeader->Magic[0] != FileMagic[0] ||
       baseHeader->Magic[1] != FileMagic[1] ||
       baseHeader->Magic[2] != FileMagic[2] ||
       baseHeader->Magic[3] != FileMagic[3])
    {
        return PetFail;
    }

    if(baseHeader->Endian != GoodEndian)
    {
        if(baseHeader->Endian != WrongEndian)
        {
            return PetFail;
        }

        FileUtils::FlipEndian(baseHeader->Version);
        FileUtils::FlipEndian(baseHeader->MinVersion);
        FileUtils::FlipEndian(baseHeader->Reserved[0]);
        FileUtils::FlipEndian(baseHeader->Reserved[1]);
        FileUtils::FlipEndian(baseHeader->Reserved[2]);
        FileUtils::FlipEndian(baseHeader->Reserved[3]);
        FileUtils::FlipEndian(baseHeader->Reserved[4]);
        FileUtils::FlipEndian(baseHeader->Reserved[5]);
        FileUtils::FlipEndian(baseHeader->Reserved[6]);
        FileUtils::FlipEndian(baseHeader->Reserved[7]);
        FileUtils::FlipEndian(baseHeader->KeyCount);
        FileUtils::FlipEndian(baseHeader->NextHeaderOffset);
        FileUtils::FlipEndian(baseHeader->KeyIndicesOffset);
        FileUtils::FlipEndian(baseHeader->StringBlobOffset);
    }

    m_BaseHeader = *baseHeader;

    switch(baseHeader->Version)
    {
        case FileVersion1_0: return LoadV1_0(keyManager, petManager);
        default: break;
    }

    switch(baseHeader->MinVersion)
    {
        case FileVersion1_0: return LoadV1_0(keyManager, petManager);
        default: break;
    }

    return PetFail;
}

PetStatus BlackboardKeyLoader::LoadV1_0(BlackboardKeyManager& keyManager, PetManager& petManager) noexcept
{
    size_t currentKeyOffset = m_BaseHeader.KeyIndicesOffset;

    if(currentKeyOffset >= m_FileLength)
    {
        return PetFail;
    }

    return PetSuccess;
}
