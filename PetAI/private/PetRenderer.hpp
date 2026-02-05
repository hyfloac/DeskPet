#pragma once

#include "PetAI.h"
#include "Objects.hpp"

class DefaultPetRenderer final
{
    DELETE_CM(DefaultPetRenderer);
public:
    static DefaultPetRenderer* FromHandle(const PetRendererHandle handle) noexcept
    {
        return static_cast<DefaultPetRenderer*>(handle.Ptr);
    }
public:
    DefaultPetRenderer(
        const uint16_t width,
        const uint16_t height,
        uint8_t* const framebuffer
    ) noexcept
        : m_Width(width)
        , m_Height(height)
        , m_Framebuffer(framebuffer)
    { }

    ~DefaultPetRenderer() noexcept
    {
        delete[] m_Framebuffer;
    }

    [[nodiscard]] size_t FramebufferSize() const noexcept { return FramebufferSize(m_Width, m_Height); }

    PetStatus GetScreenSize(uint16_t* const pWidth, uint16_t* const pHeight) const noexcept;

    PetStatus ClearScreen(const uint8_t r, const uint8_t g, const uint8_t b, const uint16_t depth) noexcept;

    PetStatus DrawRectangle(const DrawRectData* pDrawData) noexcept;

    PetStatus DrawTriangle(const DrawTriangleData* pDrawData) noexcept;

    PetStatus CopyFramebuffer(uint8_t* const pOutFramebuffer, const size_t size) const noexcept;
public:
    static PetStatus CreateDefaultRenderer(const CreateDefaultPetRenderer* const pCreateDefaultRenderer) noexcept;
    static PetStatus DestroyDefaultRenderer(const PetRendererHandle handle) noexcept;

    static size_t FramebufferSize(const uint16_t width, const uint16_t height)
    {
        return static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
    }
private:
    uint16_t m_Width;
    uint16_t m_Height;
    uint8_t* m_Framebuffer;
};
