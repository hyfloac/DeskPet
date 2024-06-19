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

#pragma pack(push, 1)
struct BlackboardKeyFileBaseHeader final
{
    char Magic[4];
    uint16_t Endian;
    uint16_t Version;
    uint16_t MinVersion;
    uint32_t Reserved[8];
    uint32_t NextHeaderOffset;
    uint32_t KeyIndicesOffset;
    uint32_t StringBlobOffset;
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

    m_CurrentOffset = 0;
    m_FileOffset = 0;
    m_Length = 0;

    petManager.AppFunctions()->LoadPetState(petManager.AppHandle(), BlackboardKeyFileHandle, m_FileOffset, m_DataBlock, &m_Length);
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
        FileUtils::FlipEndian(baseHeader->NextHeaderOffset);
        FileUtils::FlipEndian(baseHeader->KeyIndicesOffset);
        FileUtils::FlipEndian(baseHeader->StringBlobOffset);
    }

    switch(baseHeader->Version)
    {
        case FileVersion1_0: return LoadV1_0(keyManager, petManager, ::std::move(block));
        default: break;
    }

    switch(baseHeader->MinVersion)
    {
        case FileVersion1_0: return LoadV1_0(keyManager, petManager, ::std::move(block));
        default: break;
    }

    return PetFail;
}

PetStatus BlackboardKeyLoader::LoadV1_0(BlackboardKeyManager& keyManager, PetManager& petManager, FileBlockHandle block) noexcept
{
    return PetSuccess;
}
