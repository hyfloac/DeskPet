#pragma once

#include "Objects.hpp"
#include "PetAI.h"
#include "Blackboard.hpp"
#include "BehaviorTree.hpp"

class PetEntity final
{
    DEFAULT_DESTRUCT(PetEntity);
    DELETE_CM(PetEntity);
public:
    static PetEntity* FromHandle(const PetHandle handle) noexcept
    {
        return static_cast<PetEntity*>(handle.Ptr);
    }
public:
    PetEntity(
        void* const state,
        const ::std::uint32_t stateSize,
        const BlackboardKeyManager& keyManager,
        BehaviorTreeRepeatNode* const root,
        PetManager* const petManager
    ) noexcept
        : m_ParentMale(nullptr)
        , m_ParentFemale(nullptr)
        , m_State(state)
        , m_StateSize(stateSize)
        , m_Blackboard(keyManager)
        , m_BehaviorTreeExecutor(root, &m_Blackboard, petManager)
    { }

    [[nodiscard]]       PetEntity*& ParentMale()       noexcept { return m_ParentMale; }
    [[nodiscard]] const PetEntity*  ParentMale() const noexcept { return m_ParentMale; }
    
    [[nodiscard]]       PetEntity*& ParentFemale()       noexcept { return m_ParentFemale; }
    [[nodiscard]] const PetEntity*  ParentFemale() const noexcept { return m_ParentFemale; }
    
    [[nodiscard]] PetGender& Gender()       noexcept { return m_Gender; }
    [[nodiscard]] PetGender  Gender() const noexcept { return m_Gender; }

    [[nodiscard]] void* State() const noexcept { return m_State; }
    [[nodiscard]] ::std::uint32_t StateSize() const noexcept { return m_StateSize; }
    [[nodiscard]] ::Blackboard& Blackboard() noexcept { return m_Blackboard; }
    [[nodiscard]] ::BehaviorTreeExecutor& BehaviorTreeExecutor() noexcept { return m_BehaviorTreeExecutor; }
private:
    PetEntity* m_ParentMale;
    PetEntity* m_ParentFemale;
    PetGender m_Gender;
    void* m_State;
    ::std::uint32_t m_StateSize;
    ::Blackboard m_Blackboard;
    ::BehaviorTreeExecutor m_BehaviorTreeExecutor;
};
