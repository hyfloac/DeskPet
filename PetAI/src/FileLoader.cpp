#include "FileLoader.hpp"
#include <cstring>
#include <algorithm>
#include <iterator>
#include <limits>

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

FileReader::~FileReader() noexcept
{
    if(m_DataBlock)
    {
        delete[] m_DataBlock;
        m_DataBlock = nullptr;
        m_DataBlockSize = 0;
    }
}

PetStatus FileReader::OpenFile(
    PetManager& petManager, 
    FileReader* const pFileReader, 
    const PetFileHandle fileHandle, 
    const uint32_t targetBlockSize
) noexcept
{
    if(!pFileReader)
    {
        return PetInvalidArg;
    }

    if(fileHandle == 0)
    {
        return PetInvalidArg;
    }

    if(targetBlockSize == 0)
    {
        return PetInvalidArg;
    }

    pFileReader->~FileReader();

    ::new(pFileReader) FileReader(
        petManager, 
        fileHandle, 
        targetBlockSize
    );

    const PetStatus status = pFileReader->Init();

    if(!IsStatusSuccess(status))
    {
        pFileReader->~FileReader();
        return status;
    }

    return PetSuccess;
}

PetStatus FileReader::Init() noexcept
{
    m_FileLength = 0;
    m_FileOffset = InvalidFileOffset;

    PetStatus status = m_PetManager->AppFunctions()->LoadPetState(m_PetManager->AppHandle(), BlackboardKeyFileHandle, 0, nullptr, &m_FileLength);

    if(!IsStatusSuccess(status))
    {
        if(status == PetInvalidArg)
        {
            return PetFail;
        }

        return status;
    }

    // If the size of the file is smaller than the data block size, just allocate enough for the file.
    m_TargetBlockSize = ::std::min(m_FileLength, m_TargetBlockSize);

    m_DataBlockSize = m_TargetBlockSize;

    m_DataBlock = new uint8_t[m_DataBlockSize];

    return PetSuccess;
}

PetStatus FileReader::ReadBytes(const size_t offset, void* const data, size_t* const pSize) noexcept
{
    if(!pSize)
    {
        return PetInvalidArg;
    }

    if(offset + *pSize > m_FileLength)
    {
        return PetInvalidArg;
    }

    if(!data)
    {
        return PetInvalidArg;
    }

    PetStatus status = PetSuccess;

    // Do we need to read in a new block?
    if(m_FileOffset == InvalidFileOffset || offset < m_FileOffset || offset > m_FileOffset + m_DataBlockSize)
    {
        m_DataBlockSize = m_TargetBlockSize;
        status = m_PetManager->AppFunctions()->LoadPetState(m_PetManager->AppHandle(), BlackboardKeyFileHandle, offset, m_DataBlock, &m_DataBlockSize);

        if(!IsStatusSuccess(status))
        {
            if(status == PetInvalidArg)
            {
                return PetFail;
            }

            return status;
        }

        m_FileOffset = offset;
    }

    size_t blockOffset = offset - m_FileOffset;

    // If the size is smaller than our block, then we can just trivially copy it.
    if(blockOffset + *pSize < m_DataBlockSize)
    {
        (void) ::std::memcpy(data, m_DataBlock + blockOffset, *pSize);

        return PetSuccess;
    }

    size_t sizeRemaining = *pSize;
    size_t dataOffset = 0;
    size_t readSize = m_DataBlockSize - blockOffset;
    //   This is used to indicate that we can no longer read more of the
    // file.
    //   We will carry around to the next loop iteration, and exit after we
    // copy from the data block;
    bool exitNext = false;

    while(sizeRemaining > 0)
    {
        (void) ::std::memcpy(static_cast<uint8_t*>(data) + dataOffset, m_DataBlock + blockOffset, readSize);

        sizeRemaining -= readSize;
        dataOffset += readSize;

        if(exitNext)
        {
            *pSize -= sizeRemaining;
        }

        m_FileOffset += m_DataBlockSize;

        m_DataBlockSize = m_TargetBlockSize;
        status = m_PetManager->AppFunctions()->LoadPetState(m_PetManager->AppHandle(), BlackboardKeyFileHandle, m_FileOffset, m_DataBlock, &m_DataBlockSize);

        if(!IsStatusSuccess(status))
        {
            if(status == PetInvalidArg)
            {
                return PetFail;
            }

            return status;
        }

        blockOffset = 0;
        readSize = m_DataBlockSize;

        if(m_DataBlockSize < m_TargetBlockSize)
        {
            exitNext = true;
        }
    }

    return PetSuccess;
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

BlackboardKeyLoader::~BlackboardKeyLoader() noexcept
{
    if(m_DataBlock)
    {
        delete[] m_DataBlock;
        m_DataBlock = nullptr;
        m_DataBlockSize = 0;
    }
}

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
    m_FileOffset = 0;

    PetStatus status = petManager.AppFunctions()->LoadPetState(petManager.AppHandle(), BlackboardKeyFileHandle, 0, nullptr, &m_FileLength);

    if(!IsStatusSuccess(status))
    {
        if(status == PetInvalidArg)
        {
            return PetFail;
        }

        return status;
    }

    // If the size of the file is smaller than the data block size, just allocate enough for the file.
    const size_t newDataBlockSize = ::std::min(m_FileLength, m_TargetBlockSize);

    // If the new block size is smaller than our existing block, don't bother making it bigger.
    if(newDataBlockSize > m_DataBlockSize)
    {
        delete[] m_DataBlock;

        m_DataBlock = new uint8_t[m_DataBlockSize];
        m_DataBlockSize = newDataBlockSize;
    }

    size_t length = m_DataBlockSize;

    status = petManager.AppFunctions()->LoadPetState(petManager.AppHandle(), BlackboardKeyFileHandle, m_FileOffset, m_DataBlock, &length);

    if(!IsStatusSuccess(status))
    {
        if(status == PetInvalidArg)
        {
            return PetFail;
        }

        return status;
    }

    m_FileOffset += length;

    if(length < sizeof(BlackboardKeyFileBaseHeader))
    {
        return PetFail;
    }

    BlackboardKeyFileBaseHeader* const baseHeader = reinterpret_cast<BlackboardKeyFileBaseHeader*>(m_DataBlock);

    m_DataBlockOffset += sizeof(BlackboardKeyFileBaseHeader);

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

        FlipEndian(*baseHeader);
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

void BlackboardKeyLoader::FlipEndian(BlackboardKeyFileBaseHeader& baseHeader) noexcept
{
    FileUtils::FlipEndian(baseHeader.Version);
    FileUtils::FlipEndian(baseHeader.MinVersion);
    FileUtils::FlipEndian(baseHeader.Reserved[0]);
    FileUtils::FlipEndian(baseHeader.Reserved[1]);
    FileUtils::FlipEndian(baseHeader.Reserved[2]);
    FileUtils::FlipEndian(baseHeader.Reserved[3]);
    FileUtils::FlipEndian(baseHeader.Reserved[4]);
    FileUtils::FlipEndian(baseHeader.Reserved[5]);
    FileUtils::FlipEndian(baseHeader.Reserved[6]);
    FileUtils::FlipEndian(baseHeader.Reserved[7]);
    FileUtils::FlipEndian(baseHeader.KeyCount);
    FileUtils::FlipEndian(baseHeader.NextHeaderOffset);
    FileUtils::FlipEndian(baseHeader.KeyIndicesOffset);
    FileUtils::FlipEndian(baseHeader.StringBlobOffset);
}
