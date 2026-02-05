#include "PetManager.hpp"
#include "PetEntity.hpp"
#include "PetBehaviors.hpp"
#include <new>

#include "PetRenderer.hpp"

PetManager::PetManager() noexcept
    : m_AppFunctions()
    , m_AppHandle { nullptr }
    , m_PetCallbackHandle(nullptr)
    , m_BlackboardKeyManager()
    , m_RendererHandle { nullptr }
    , m_RendererFunctions()
    , m_ShouldExit(false)
    , m_HasRenderer(false)
    , m_Pets()
{ }

PetStatus PetManager::NotifyExit() noexcept
{
    m_ShouldExit = true;
    return PetSuccess;
}

PetStatus PetManager::GetPetState(const PetHandle petHandle, void** const pState, uint32_t* const pSize) noexcept
{
    if(!petHandle.Ptr)
    {
        return PetInvalidArg;
    }

    if(!pState)
    {
        return PetInvalidArg;
    }

    if(!pSize)
    {
        return PetInvalidArg;
    }

    const PetEntity* pet = PetEntity::FromHandle(petHandle);

    *pState = pet->State();
    *pSize = pet->StateSize();

    return PetSuccess;
}

PetStatus PetManager::CreatePet(const CreatePetAIData* const pCreatePetData, PetHandle* const pPetHandle) noexcept
{
    if(!pCreatePetData)
    {
        return PetInvalidArg;
    }

    PetEntity* pet = new(::std::nothrow) PetEntity(
        pCreatePetData->State, 
        pCreatePetData->StateSize, 
        m_BlackboardKeyManager,
        &g_RootNode,
        this
    );
    pet->ParentMale() = PetEntity::FromHandle(pCreatePetData->ParentMale);
    pet->ParentFemale() = PetEntity::FromHandle(pCreatePetData->ParentFemale);
    pet->Gender() = pCreatePetData->Gender;

    m_Pets.push_back(pet);

    if(pPetHandle)
    {
        pPetHandle->Ptr = pet;
    }

    return PetSuccess;
}

PetStatus PetManager::CreateDefaultRenderer(const CreateDefaultPetRenderer* pCreateDefaultRenderer) const noexcept
{
    if(!pCreateDefaultRenderer)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::CreateDefaultRenderer(pCreateDefaultRenderer);
}

PetStatus PetManager::DestroyDefaultRenderer(const PetRendererHandle rendererHandle) const noexcept
{
    if(!rendererHandle.Ptr)
    {
        return PetInvalidArg;
    }

    return DefaultPetRenderer::DestroyDefaultRenderer(rendererHandle);
}

PetStatus PetManager::CreateRenderer() noexcept
{
    if(!m_AppFunctions.CreateRenderer)
    {
        return PetNotImplemented;
    }

    m_RendererFunctions.Version = PET_RENDERER_VERSION_1_0;
    PetStatus status = m_AppFunctions.CreateRenderer(m_AppHandle, &m_RendererHandle, &m_RendererFunctions);

    if(status == PetNotImplemented)
    {
        return PetNotImplemented;
    }

    if(IsStatusError(status))
    {
        DebugPrintF(u8"[PetManager::CreateRenderer]: m_AppFunctions->CreateRenderer returned status 0x%08X.\n", status);
        return status;
    }

    if(m_RendererFunctions.Version < PET_RENDERER_VERSION_1_0)
    {
        return PetNotImplemented;
    }

    if(!m_RendererFunctions.GetScreenSize)
    {
        return PetNotImplemented;
    }

    uint16_t width = 0;
    uint16_t height = 0;

    status = m_RendererFunctions.GetScreenSize(m_RendererHandle, &width, &height);

    if(IsStatusError(status))
    {
        DebugPrintF(u8"[PetManager::CreateRenderer]: m_RendererFunctions->GetScreenSize returned status 0x%08X.\n", status);
        return status;
    }

    if(width == 0 || height == 0)
    {
        return PetNotImplemented;
    }

    m_HasRenderer = true;

    if(m_RendererFunctions.ClearScreen)
    {
        m_RendererFunctions.ClearScreen(m_RendererHandle, 200, 200, 200, 0);
    }

    if(m_RendererFunctions.DrawRectangle)
    {
        DrawRectData drawData {};
        drawData.Points[0].X = 0;
        drawData.Points[0].Y = 0;
        drawData.Points[1].X = width / 4;
        drawData.Points[1].Y = height / 2;
        drawData.Depth = 0xFFFF;
        drawData.Color.R = 0x00;
        drawData.Color.G = 0xFF;
        drawData.Color.B = 0xFF;

        m_RendererFunctions.DrawRectangle(m_RendererHandle, &drawData);
    }

    if(m_RendererFunctions.DrawTriangle)
    {
        DrawTriangleData drawData {};
        drawData.Points[0].X = width / 4;
        drawData.Points[0].Y = 0;
        drawData.Points[1].X = 0;
        drawData.Points[1].Y = height - 1;
        drawData.Points[2].X = width - 1;
        drawData.Points[2].Y = height - 4;
        drawData.Depth = 0xFFFF;
        drawData.Color.R = 0xFF;
        drawData.Color.G = 0x00;
        drawData.Color.B = 0xFF;

        m_RendererFunctions.DrawTriangle(m_RendererHandle, &drawData);
    }

    return PetSuccess;
}
