#pragma once

#include "Objects.hpp"
#include "PetAI.h"
#include "Blackboard.hpp"
#include "PetManager.hpp"

constexpr PetFileHandle BlackboardKeyFileHandle = 7;

class FileUtils final
{
    DELETE_CONSTRUCT(FileUtils);
    DELETE_DESTRUCT(FileUtils);
    DELETE_CM(FileUtils);
public:
    static constexpr uint16_t MakeVersion(const uint8_t majorVersion, const uint8_t minorVersion) noexcept
    {
        return static_cast<uint16_t>((static_cast<uint16_t>(majorVersion) << 8) | static_cast<uint16_t>(minorVersion));
    }

    template<typename T>
    static void FlipEndian(T& val) noexcept;
};

class FileBlockHandle;

class FileBlock final
{
public:
    static inline consteval size_t BlockSize = 512;
public:
    FileBlock() noexcept
        : m_Block{ }
        , m_Possessed(false)
    { }

    [[nodiscard]]       uint8_t* Block()       noexcept { return m_Block; }
    [[nodiscard]] const uint8_t* Block() const noexcept { return m_Block; }

    FileBlockHandle Acquire(uint8_t*& receivingBlock) noexcept;
private:
    void Release() noexcept
    {
        m_Possessed = false;
    }
private:
    uint8_t m_Block[BlockSize];
    bool m_Possessed;
private:
    friend class FileBlockHandle;
};

class FileBlockHandle final
{
    DELETE_COPY(FileBlockHandle);
public:
    FileBlockHandle() noexcept
        : m_Block(nullptr)
        , m_ReceivingBlock(nullptr)
    { }
private:
    FileBlockHandle(FileBlock& block, uint8_t*& receivingBlock) noexcept
        : m_Block(&block)
        , m_ReceivingBlock(&receivingBlock)
    { }
public:
    ~FileBlockHandle() noexcept
    {
        if(m_Block)
        {
            m_Block->Release();
        }

        if(m_ReceivingBlock)
        {
            *m_ReceivingBlock = nullptr;
        }
    }

    FileBlockHandle(FileBlockHandle&& move) noexcept
        : m_Block(move.m_Block)
        , m_ReceivingBlock(move.m_ReceivingBlock)
    {
        move.m_Block = nullptr;
        move.m_ReceivingBlock = nullptr;
    }

    FileBlockHandle& operator=(FileBlockHandle&& move) noexcept
    {
        if(this == &move)
        {
            return *this;
        }

        m_Block = move.m_Block;
        m_ReceivingBlock = move.m_ReceivingBlock;

        move.m_Block = nullptr;
        move.m_ReceivingBlock = nullptr;

        return *this;
    }

    [[nodiscard]] operator bool() const noexcept { return m_Block; }

    [[nodiscard]]       uint8_t* Block()       noexcept { return m_Block->Block(); }
    [[nodiscard]] const uint8_t* Block() const noexcept { return m_Block->Block(); }

    [[nodiscard]] operator       uint8_t*()       noexcept { return Block(); }
    [[nodiscard]] operator const uint8_t*() const noexcept { return Block(); }
private:
    FileBlock* m_Block;
    uint8_t** m_ReceivingBlock;
private:
    friend class FileBlock;
};

class BlackboardKeyLoader final
{
    DEFAULT_DESTRUCT(BlackboardKeyLoader);
    DELETE_CM(BlackboardKeyLoader);
public:
    static inline consteval char FileMagic[4] = { 'B', 'l', 'k', 'K' };
    static inline consteval uint16_t GoodEndian = 0xFFFE;
    static inline consteval uint16_t FileVersion1_0 = FileUtils::MakeVersion(1, 0);
public:
    BlackboardKeyLoader() noexcept
        : m_DataBlock(nullptr)
        , m_CurrentOffset(0)
        , m_Length(0)
        , m_FileOffset(0)
    { }

    PetStatus Load(BlackboardKeyManager& keyManager, PetManager& petManager) noexcept;
private:
    PetStatus LoadV1_0(BlackboardKeyManager& keyManager, PetManager& petManager, FileBlockHandle block) noexcept;
private:
    uint8_t* m_DataBlock;
    uint32_t m_CurrentOffset;
    size_t m_Length;
    size_t m_FileOffset;
};
