#include "PetAI.h"
#include "PetManager.hpp"
#include "PetBehaviors.hpp"
#include "BehaviorTree.hpp"
#include <SysLib.h>
#include <new>

PetManager g_PetManager;

static bool s_ShouldExit = false;

// ReSharper disable once CppFunctionIsNotImplemented
static PetStatus NotifyExit(const PetAIHandle petAIHandle);

extern "C" PetStatus InitPetAI(const PetFunctions* const pFunctions)
{
    InitSys();

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

    g_PetManager.AppFunctions() = pFunctions;
    g_PetManager.AppHandle().Ptr = nullptr;
    g_PetManager.PetCallbackHandle() = nullptr;

    return PetSuccess;
}

// ReSharper disable once CppFunctionIsNotImplemented
static PetStatus LoadState(const uint8_t** const ppBuffer) noexcept;

extern "C" PetStatus TAU_UTILS_LIB RunPetAI()
{
    if(!g_PetManager.AppFunctions())
    {
        return PetSuccess;
    }

    PetAICallbacks callbacks {};
    callbacks.Handle.Ptr = g_PetManager.PetCallbackHandle();
    callbacks.NotifyExit = NotifyExit;

    PetStatus status = g_PetManager.AppFunctions()->CreatePetApp(&g_PetManager.AppHandle(), &callbacks);

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

    BlackboardKeyManager blackboardKeyManager;

    Blackboard blackboard(blackboardKeyManager);
    BehaviorTreeExecutor behaviorTreeExecutor(&g_RootNode, &blackboard, &g_PetManager);

    TimeMs_t lastTime = GetCurrentTimeMs();

    while(!s_ShouldExit)
    {
        const TimeMs_t currentTime = GetCurrentTimeMs();

        const float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f;

        if(g_PetManager.AppFunctions()->Update)
        {
            g_PetManager.AppFunctions()->Update(g_PetManager.AppHandle(), deltaTime);
        }

        if(s_ShouldExit)
        {
            break;
        }

        behaviorTreeExecutor.Tick(deltaTime);

        if(g_PetManager.AppFunctions()->Sleep)
        {
            TimeMs_t sleepTime = 5;
            g_PetManager.AppFunctions()->Sleep(g_PetManager.AppHandle(), &sleepTime);
        }

        lastTime = currentTime;
    }


    status = g_PetManager.AppFunctions()->DestroyPetApp(g_PetManager.AppHandle());

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

    if(!g_PetManager.AppFunctions()->LoadPetState)
    {
        return PetNotImplemented;
    }

    size_t stateSize;
    PetStatus status = g_PetManager.AppFunctions()->LoadPetState(g_PetManager.AppHandle(), dummyFileHandle, 0, nullptr, &stateSize);

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
    status = g_PetManager.AppFunctions()->LoadPetState(g_PetManager.AppHandle(), dummyFileHandle, 0, buffer, &stateSizeRead);

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

static PetStatus NotifyExit(const PetAIHandle petAIHandle)
{
    (void) petAIHandle;

    s_ShouldExit = true;
    return PetSuccess;
}

