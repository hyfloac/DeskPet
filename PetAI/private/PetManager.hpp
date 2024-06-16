#pragma once

#include "PetAI.h"
#include "Objects.hpp"

class PetManager final
{
    DEFAULT_CONSTRUCT_PU(PetManager);
    DEFAULT_DESTRUCT(PetManager);
    DELETE_CM(PetManager);
public:
    [[nodiscard]] const PetFunctions*&        AppFunctions()       noexcept { return m_AppFunctions; }
    [[nodiscard]] const PetFunctions* const & AppFunctions() const noexcept { return m_AppFunctions; }

    [[nodiscard]] PetAppHandle& AppHandle()       noexcept { return m_AppHandle; }
    [[nodiscard]] PetAppHandle  AppHandle() const noexcept { return m_AppHandle; }

    [[nodiscard]] void*& PetCallbackHandle()       noexcept { return m_PetCallbackHandle; }
    [[nodiscard]] void*  PetCallbackHandle() const noexcept { return m_PetCallbackHandle; }
private:
    const PetFunctions* m_AppFunctions;
    PetAppHandle m_AppHandle;
    void* m_PetCallbackHandle;
};
