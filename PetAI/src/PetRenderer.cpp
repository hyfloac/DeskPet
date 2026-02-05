#include "PetRenderer.hpp"

#include <cassert>
#include <cstring>
#include <new>

static PetStatus GetScreenSize(const PetRendererHandle rendererHandle, uint16_t* const pWidth, uint16_t* const pHeight);
static PetStatus ClearScreen(const PetRendererHandle rendererHandle, const uint8_t r, const uint8_t g, const uint8_t b, const uint16_t depth);
static PetStatus DrawRectangle(const PetRendererHandle rendererHandle, const DrawRectData* const pDrawData);
static PetStatus DrawTriangle(const PetRendererHandle rendererHandle, const DrawTriangleData* const pDrawData);
static PetStatus CopyFramebuffer(const PetRendererHandle rendererHandle, uint8_t* const pOutFramebuffer, const size_t size);

PetStatus DefaultPetRenderer::GetScreenSize(uint16_t* pWidth, uint16_t* pHeight) const noexcept
{
    if(!pWidth)
    {
        return PetInvalidArg;
    }

    if(!pHeight)
    {
        return PetInvalidArg;
    }

    *pWidth = m_Width;
    *pHeight = m_Height;

    return PetSuccess;
}

PetStatus DefaultPetRenderer::ClearScreen(const uint8_t r, const uint8_t g, const uint8_t b, const uint16_t depth) noexcept
{
    const size_t framebufferSize = FramebufferSize();

    for(size_t i = 0; i < framebufferSize; i += 3)
    {
        m_Framebuffer[i + 0] = r;
        m_Framebuffer[i + 1] = g;
        m_Framebuffer[i + 2] = b;
    }

    return PetSuccess;
}

PetStatus DefaultPetRenderer::DrawRectangle(const DrawRectData* pDrawData) noexcept
{
    if(!pDrawData)
    {
        return PetInvalidArg;
    }

    const uint16_t startY = pDrawData->Points[0].Y < pDrawData->Points[1].Y ? pDrawData->Points[0].Y : pDrawData->Points[1].Y;
    const uint16_t endY   = pDrawData->Points[0].Y < pDrawData->Points[1].Y ? pDrawData->Points[1].Y : pDrawData->Points[0].Y;
    const uint16_t startX = pDrawData->Points[0].X < pDrawData->Points[1].X ? pDrawData->Points[0].X : pDrawData->Points[1].X;
    const uint16_t endX   = pDrawData->Points[0].X < pDrawData->Points[1].X ? pDrawData->Points[1].X : pDrawData->Points[0].X;

    for(uint16_t y = startY; y < endY; ++y)
    {
        for(uint16_t x = startX; x < endX; ++x)
        {
            const size_t i = (static_cast<size_t>(m_Width) * static_cast<size_t>(y) + static_cast<size_t>(x)) * 3;
            m_Framebuffer[i + 0] = pDrawData->Color.R;
            m_Framebuffer[i + 1] = pDrawData->Color.G;
            m_Framebuffer[i + 2] = pDrawData->Color.B;
        }
    }

    return PetSuccess;
}

/**
 *   This function orders the points first based on their y-coordinate,
 * and then their x-coordinate. This sorts from the lowest point to the
 * highest point, in terms of its y value, rather than perceptual point
 * on the screen.
 *
 *   The return value is 3 nibbles, in Big Endian order, of the order
 * of points, so 0x012 has point 0 as the lowest point, point 1 in the
 * middle, and point 2 as the highest. 0x201 would have point 2 as the
 * lowest, point 0 in the middle, and point 1 as the highest.
 *
 *   This can probably be simplified, but for now these test cases
 * clearly explain each possible arrangement of points.
 *
 * @param points The 3 points to order.
 * @return 3 nibbles from MSB-LSB order of points.
 */
static uint16_t PickPoint(const ScreenPoint points[3]) noexcept
{
    if(points[0].Y < points[1].Y) // y_0 < y_1
    {
        if(points[0].Y < points[2].Y) // y_0 < y_1 && y_0 < y_2
        {
            if(points[1].Y < points[2].Y) // y_0 < y_1 < y_2
            {
                return 0x012;
            }
            else if(points[2].Y < points[1].Y) // y_0 < y_2 < y_1
            {
                return 0x021;
            }
            else // y_0 < y_1 == y_2
            {
                if(points[1].X < points[2].X) // y_0 < y_1 == y_2 && x_1 < x_2
                {
                    return 0x012;
                }
                else // y_0 < y_1 == y_2 && x_2 <= x_1
                {
                    return 0x021;
                }
            }
        }
        else if(points[2].Y < points[0].Y) // y_2 < y_0 < y_1
        {
            return 0x201;
        }
        else // y_2 == y_0 < y_1
        {
            if(points[0].X < points[2].X) // y_2 == y_0 < y_1 && x_0 < x_2
            {
                return 0x021;
            }
            else // y_2 == y_0 < y_1 && x_2 <= x_0
            {
                return 0x201;
            }
        }
    }
    else if(points[1].Y < points[0].Y) // y_1 < y_0
    {
        if(points[1].Y < points[2].Y) // y_1 < y_0 && y_1 < y_2
        {
            if(points[0].Y < points[2].Y) // y_1 < y_0 < y_2
            {
                return 0x102;
            }
            else if(points[2].Y < points[0].Y) // y_1 < y_2 < y_0
            {
                return 0x120;
            }
            else // y_1 < y_0 == y_2
            {
                if(points[0].X < points[2].X) // y_1 < y_0 == y_2 && x_0 < x_2
                {
                    return 0x102;
                }
                else // y_1 < y_0 == y_2 && x_2 <= x_0
                {
                    return 0x120;
                }
            }
        }
        else if(points[2].Y < points[1].Y) // y_2 < y_1 < y_0
        {
            return 0x021;
        }
        else // y_2 == y_1 < y_0
        {
            if(points[2].X < points[1].X) // y_2 == y_1 < y_0 && x_2 < x_1
            {
                return 0x210;
            }
            else // y_2 == y_1 < y_0 && x_1 <= x_2
            {
                return 0x120;
            }
        }
    }
    else // y_0 == y_1
    {
        if(points[0].Y < points[2].Y) // y_0 == y_1 < y_2
        {
            if(points[0].X < points[1].X) // y_0 == y_1 < y_2 && x_0 < x_1
            {
                return 0x012;
            }
            else // y_0 == y_1 < y_2 && x_1 < x_0
            {
                return 0x102;
            }
        }
        else if(points[2].Y < points[0].Y) // y_2 < y_0 == y_1
        {
            if(points[0].X < points[1].X) // y_2 < y_0 == y_1 && x_0 < x_1
            {
                return 0x201;
            }
            else // y_2 < y_0 == y_1 && x_1 < x_0
            {
                return 0x210;
            }
        }
        else // y_0 == y_1 == y_2
        {
            if(points[0].X < points[1].X) // y_0 == y_1 == y_2 && x_0 < x_1
            {
                if(points[0].X < points[2].X) // y_0 == y_1 == y_2 && x_0 < x_1 && x_0 < x_2
                {
                    if(points[1].X < points[2].X) // y_0 == y_1 == y_2 && x_0 < x_1 < x_2
                    {
                        return 0x012;
                    }
                    else // y_0 == y_1 == y_2 && x_0 < x_2 <= x_1
                    {
                        return 0x021;
                    }
                }
                else // y_0 == y_1 == y_2 && x_2 <= x_0 < x_1
                {
                    return 0x201;
                }
            }
            else if(points[1].X < points[0].X) // y_0 == y_1 == y_2 && x_1 < x_0
            {
                if(points[1].X < points[2].X) // y_0 == y_1 == y_2 && x_1 < x_0 && x_1 < x_2
                {
                    if(points[2].X < points[0].X) // y_0 == y_1 == y_2 && x_1 < x_2 < x_0
                    {
                        return 0x120;
                    }
                    else // y_0 == y_1 == y_2 && x_1 < x_0 <= x_2
                    {
                        return 0x102;
                    }
                }
                else // y_0 == y_1 == y_2 && x_1 < x_0 && x_2 <= x_1
                {
                    if(points[1].X < points[0].X) // y_0 == y_1 == y_2 && x_2 <= x_1 < x_0
                    {
                        return 0x210;
                    }
                    else // y_0 == y_1 == y_2 && x_2 <= x_0 <= x_1
                    {
                        return 0x201;
                    }
                }
            }
        }
    }

    assert(false);
    return 0xFFFF;
}

PetStatus DefaultPetRenderer::DrawTriangle(const DrawTriangleData* pDrawData) noexcept
{
    const uint16_t points = PickPoint(pDrawData->Points);

    if(points == 0xFFFF)
    {
        return PetFail;
    }

    const ScreenPoint& p0 = pDrawData->Points[points >> 8];
    const ScreenPoint& p1 = pDrawData->Points[(points >> 4) & 0x0F];
    const ScreenPoint& p2 = pDrawData->Points[points & 0x0F];

    const ScreenPoint& xp1 = p1.X < p2.X ? p1 : p2;
    const ScreenPoint& xp2 = p1.X < p2.X ? p2 : p1;

    const ScreenPoint& xp20 = p0.X < p1.X ? p0 : p1;
    const ScreenPoint& xp21 = p0.X < p1.X ? p1 : p0;

    const int32_t t01Rise = (xp1.Y - p0.Y);
    const int32_t t01Run = (xp1.X - p0.X);
    const int32_t t02Rise = (xp2.Y - p0.Y);
    const int32_t t02Run = (xp2.X - p0.X);

    const int32_t t20Rise = (xp20.Y - p2.Y);
    const int32_t t20Run = (xp20.X - p2.X);
    const int32_t t21Rise = (xp21.Y - p2.Y);
    const int32_t t21Run = (xp21.X - p2.X);

    // Split the triangle horizontally at the height of p1.
    //   This forms a triangle with a horizontal bottom made up of p0, p1,
    // and a point on the line between p0 and p2.
    for(uint16_t y = p0.Y; y < p1.Y; ++y)
    {
        // y - h = m(x - k)
        // x = (y - h)/m + k
        const int32_t startX = (y - p0.Y) * t01Run / t01Rise + p0.X;
        const int32_t endX   = (y - p0.Y) * t02Run / t02Rise + p0.X;

        //   Using the slope of the two lines that make up the triangle, we'll
        // find the x-coordinate from the y-coordinate, and then fill the
        // points between.
        for(uint16_t x = startX; x < endX; ++x)
        {
            const size_t i = (static_cast<size_t>(m_Width) * static_cast<size_t>(y) + static_cast<size_t>(x)) * 3;
            m_Framebuffer[i + 0] = pDrawData->Color.R;
            m_Framebuffer[i + 1] = pDrawData->Color.G;
            m_Framebuffer[i + 2] = pDrawData->Color.B;
        }
    }

    // Rasterize the second half of the triangle.
    //   This forms a triangle with a horizontal top made up of p2, p1,
    // and a point on the line between p0 and p2.
    for(uint16_t y = p1.Y; y <= p2.Y; ++y)
    {
        // y - h = m(x - k)
        // x = (y - h)/m + k
        const int32_t startX = (y - p2.Y) * t20Run / t20Rise + p2.X;
        const int32_t endX   = (y - p2.Y) * t21Run / t21Rise + p2.X;

        //   Using the slope of the two lines that make up the triangle, we'll
        // find the x-coordinate from the y-coordinate, and then fill the
        // points between.
        for(uint16_t x = startX; x < endX; ++x)
        {
            const size_t i = (static_cast<size_t>(m_Width) * static_cast<size_t>(y) + static_cast<size_t>(x)) * 3;
            m_Framebuffer[i + 0] = pDrawData->Color.R;
            m_Framebuffer[i + 1] = pDrawData->Color.G;
            m_Framebuffer[i + 2] = pDrawData->Color.B;
        }
    }

    return PetSuccess;
}

PetStatus DefaultPetRenderer::CopyFramebuffer(uint8_t* const pOutFramebuffer, const size_t size) const noexcept
{
    if(!pOutFramebuffer)
    {
        return PetInvalidArg;
    }

    if(size < FramebufferSize())
    {
        return PetInvalidArg;
    }

    (void) ::memcpy(pOutFramebuffer, m_Framebuffer, FramebufferSize());

    return PetSuccess;
}

PetStatus DefaultPetRenderer::CreateDefaultRenderer(const CreateDefaultPetRenderer* const pCreateDefaultRenderer) noexcept
{
    if(!pCreateDefaultRenderer)
    {
        return PetInvalidArg;
    }

    if(!pCreateDefaultRenderer->pOutRendererHandle)
    {
        return PetInvalidArg;
    }

    if(!pCreateDefaultRenderer->pOutRendererFunctions)
    {
        return PetInvalidArg;
    }

    if(pCreateDefaultRenderer->Version < PET_RENDERER_VERSION_1_0)
    {
        return PetInvalidArg;
    }

    uint8_t* const framebuffer = new(::std::nothrow) uint8_t[pCreateDefaultRenderer->Width * pCreateDefaultRenderer->Height * 3];

    if(!framebuffer)
    {
        return PetOutOfMemory;
    }

    DefaultPetRenderer* renderer = new(::std::nothrow) DefaultPetRenderer(
        pCreateDefaultRenderer->Width,
        pCreateDefaultRenderer->Height,
        framebuffer
    );

    if(!renderer)
    {
        delete[] framebuffer;
        return PetOutOfMemory;
    }

    pCreateDefaultRenderer->pOutRendererHandle->Ptr = renderer;

    pCreateDefaultRenderer->pOutRendererFunctions->GetScreenSize = ::GetScreenSize;
    pCreateDefaultRenderer->pOutRendererFunctions->ClearScreen = ::ClearScreen;
    pCreateDefaultRenderer->pOutRendererFunctions->DrawRectangle = ::DrawRectangle;
    pCreateDefaultRenderer->pOutRendererFunctions->DrawTriangle = ::DrawTriangle;
    pCreateDefaultRenderer->pOutRendererFunctions->CopyFramebuffer = ::CopyFramebuffer;

    return PetSuccess;
}

PetStatus DefaultPetRenderer::DestroyDefaultRenderer(const PetRendererHandle handle) noexcept
{
    if(!handle.Ptr)
    {
        return PetInvalidArg;
    }

    delete FromHandle(handle);

    return PetSuccess;
}

static PetStatus GetScreenSize(const PetRendererHandle rendererHandle, uint16_t* const pWidth, uint16_t* const pHeight)
{
    if(!rendererHandle.Ptr)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::FromHandle(rendererHandle)->GetScreenSize(pWidth, pHeight);
}

static PetStatus ClearScreen(const PetRendererHandle rendererHandle, const uint8_t r, const uint8_t g, const uint8_t b, const uint16_t depth)
{
    if(!rendererHandle.Ptr)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::FromHandle(rendererHandle)->ClearScreen(r, g, b, depth);
}

static PetStatus DrawRectangle(const PetRendererHandle rendererHandle, const DrawRectData* const pDrawData)
{
    if(!rendererHandle.Ptr)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::FromHandle(rendererHandle)->DrawRectangle(pDrawData);
}

static PetStatus DrawTriangle(const PetRendererHandle rendererHandle, const DrawTriangleData* const pDrawData)
{
    if(!rendererHandle.Ptr)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::FromHandle(rendererHandle)->DrawTriangle(pDrawData);
}

static PetStatus CopyFramebuffer(const PetRendererHandle rendererHandle, uint8_t* const pOutFramebuffer, const size_t size)
{
    if(!rendererHandle.Ptr)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::FromHandle(rendererHandle)->CopyFramebuffer(pOutFramebuffer, size);
}
