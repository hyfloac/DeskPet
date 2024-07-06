#pragma once

#include "PetAI.h"
#include "Objects.hpp"
#include "Blackboard.hpp"

// I can't be bothered to handle this better right now...
#include <vector>

class PetEntity;

class PetManager final
{
    DEFAULT_CONSTRUCT_PU(PetManager);
    DEFAULT_DESTRUCT(PetManager);
    DELETE_CM(PetManager);
public:
    static PetManager* FromHandle(const PetAIHandle handle) noexcept
    {
        return static_cast<PetManager*>(handle.Ptr);
    }
public:
    using PetArray = ::std::vector<PetEntity*>;
public:
    [[nodiscard]] const PetFunctions*&        AppFunctions()       noexcept { return m_AppFunctions; }
    [[nodiscard]] const PetFunctions* const & AppFunctions() const noexcept { return m_AppFunctions; }

    [[nodiscard]] PetAppHandle& AppHandle()       noexcept { return m_AppHandle; }
    [[nodiscard]] PetAppHandle  AppHandle() const noexcept { return m_AppHandle; }

    [[nodiscard]] void*& PetCallbackHandle()       noexcept { return m_PetCallbackHandle; }
    [[nodiscard]] void*  PetCallbackHandle() const noexcept { return m_PetCallbackHandle; }

    [[nodiscard]]       ::BlackboardKeyManager& BlackboardKeyManager()       noexcept { return m_BlackboardKeyManager; }
    [[nodiscard]] const ::BlackboardKeyManager& BlackboardKeyManager() const noexcept { return m_BlackboardKeyManager; }

    [[nodiscard]] bool ShouldExit() const noexcept { return m_ShouldExit; }
    [[nodiscard]]       PetArray& Pets()       noexcept { return m_Pets; }
    [[nodiscard]] const PetArray& Pets() const noexcept { return m_Pets; }

    PetStatus NotifyExit() noexcept;
    PetStatus GetPetState(const PetHandle petHandle, void** const pState, uint32_t* const pSize) noexcept;
    PetStatus CreatePet(const CreatePetAIData* const pCreatePetData, PetHandle* const pPetHandle) noexcept;
private:
    const PetFunctions* m_AppFunctions;
    PetAppHandle m_AppHandle;
    void* m_PetCallbackHandle;
    ::BlackboardKeyManager m_BlackboardKeyManager;

    bool m_ShouldExit;
    PetArray m_Pets;
};
