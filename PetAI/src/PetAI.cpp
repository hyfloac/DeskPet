#include "PetAI.h"
#include <SysLib.h>
#include <new>

const PetFunctions* g_PetFunctions;
PetAppHandle g_PetHandle;
void* g_PetCallbackHandle;

extern "C" PetStatus InitPetAI(const PetFunctions* const pFunctions)
{
    if(!pFunctions)
    {
        DebugPrintF(u8"[InitPetAI]: pFunctions was null.\n");
        return PetInvalidArg;
    }

    if(!pFunctions->CreatePetApp)
    {
        DebugPrintF(u8"[InitPetAI]: pFunctions->CreatePet was null.\n");
        return PetInvalidArg;
    }

    g_PetFunctions = pFunctions;
    g_PetHandle.Ptr = nullptr;
    g_PetCallbackHandle = nullptr;

    return PetSuccess;
}

static PetStatus LoadState(const uint8_t** const ppBuffer) noexcept;

extern "C" PetStatus TAU_UTILS_LIB RunPetAI()
{
    if(!g_PetFunctions)
    {
        return PetSuccess;
    }

    PetAICallbacks callbacks {};
    callbacks.Handle.Ptr = g_PetCallbackHandle;

    PetStatus status = g_PetFunctions->CreatePetApp(&g_PetHandle, &callbacks);

    if(!IsStatusSuccess(status))
    {
        DebugPrintF(u8"[RunPetAI]: pFunctions->CreatePet returned status 0x%08X.\n", status);
        return status;
    }

    const uint8_t* stateBuffer;
    status = LoadState(&stateBuffer);

    if(!IsStatusSuccess(status))
    {
        stateBuffer = nullptr;
    }

    delete[] stateBuffer;

    status = g_PetFunctions->DestroyPetApp(g_PetHandle);

    if(!IsStatusSuccess(status))
    {
        DebugPrintF(u8"[RunPetAI]: pFunctions->DestroyPet returned status 0x%08X.\n", status);
        return status;
    }

    return PetSuccess;
}

static PetStatus LoadState(const uint8_t** const ppBuffer) noexcept
{
    constexpr PetFileHandle dummyFileHandle = 1337;

    if(!g_PetFunctions->LoadPetState)
    {
        return PetNotImplemented;
    }

    size_t stateSize;
    PetStatus status = g_PetFunctions->LoadPetState(g_PetHandle, dummyFileHandle, 0, nullptr, &stateSize);

    if(!IsStatusSuccess(status))
    {
        DebugPrintF(u8"[LoadState]: pFunctions->LoadPetState returned status 0x%08X.\n", status);
        return PetFail;
    }

    if(stateSize == 0)
    {
        return PetSuccess;
    }

    uint8_t* buffer = new(::std::nothrow) uint8_t[stateSize];

    size_t stateSizeRead;
    status = g_PetFunctions->LoadPetState(g_PetHandle, dummyFileHandle, 0, buffer, &stateSizeRead);

    if(!IsStatusSuccess(status))
    {
        delete[] buffer;
        DebugPrintF(u8"[LoadState]: pFunctions->LoadPetState returned status 0x%08X.\n", status);
        return PetFail;
    }

    if(stateSizeRead != stateSize)
    {
        delete[] buffer;
        return PetFail;
    }

    *ppBuffer = buffer;

    return PetSuccess;
}

