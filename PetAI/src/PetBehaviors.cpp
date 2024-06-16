// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "BehaviorTree.hpp"
#include "PetManager.hpp"
#include "Blackboard.hpp"
#include "SysLib.h"
#include <algorithm>
#include <array>


static bool Bark(void*& state, PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;
static bool Sleep5s(void*& state, PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;

static bool ContinueTree(void*& state, PetManager& petManager, const BehaviorTreeRepeatNode& node, Blackboard& blackboard) noexcept;

static BehaviorTreeActionNode s_BarkAction(Bark);
static BehaviorTreeActionNode s_SleepAction(Sleep5s);

static ::std::array<BehaviorTreeNode*, 2> s_BarkSequenceArray({ &s_BarkAction, &s_SleepAction });

static BehaviorTreeSequenceNode s_BarkSequence(s_BarkSequenceArray.size(), s_BarkSequenceArray.data());

BehaviorTreeRepeatNode g_RootNode(&s_BarkSequence, ContinueTree);

static bool Bark(void*& state, PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept
{
    (void) state;
    (void) petManager;
    (void) node;
    (void) blackboard;
    (void) deltaTime;

    DebugPrintF(u8"Bork bork!\n");

    return true;
}

static bool Sleep5s(void*& state, PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept
{
    (void) node;
    (void) blackboard;

    if(!petManager.AppFunctions()->Sleep)
    {
        return true;
    }

    static_assert(sizeof(intptr_t) == sizeof(void*), "void* does not match the size of intptr_t.");

    if(!state)
    {
        state = reinterpret_cast<void*>(5000);
    }

    intptr_t pTimeRemaining = reinterpret_cast<intptr_t>(state);

    pTimeRemaining -= static_cast<intptr_t>(deltaTime * 1000.0f);

    TimeMs_t sleepTime = ::std::min<TimeMs_t>(50, pTimeRemaining);
    const PetStatus status = petManager.AppFunctions()->Sleep(petManager.AppHandle(), &sleepTime);

    if(!IsStatusSuccess(status))
    {
        return true;
    }

    pTimeRemaining -= static_cast<intptr_t>(sleepTime);

    if(pTimeRemaining <= 0)
    {
        state = nullptr;
        return true;
    }

    state = reinterpret_cast<void*>(pTimeRemaining);  // NOLINT(performance-no-int-to-ptr)

    return false;
}

static bool ContinueTree(void*& state, PetManager& petManager, const BehaviorTreeRepeatNode& node, Blackboard& blackboard) noexcept
{
    (void) state;
    (void) petManager;
    (void) node;
    (void) blackboard;

    return true;
}
