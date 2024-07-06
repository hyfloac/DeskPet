#include "PetManager.hpp"
#include "PetEntity.hpp"
#include "PetBehaviors.hpp"
#include <new>

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
