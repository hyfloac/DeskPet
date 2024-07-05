// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "BehaviorTree.hpp"
#include "PetManager.hpp"
#include "Blackboard.hpp"
#include "SysLib.h"
#include <array>

static unsigned int g_seed = 0;

// Used to seed the generator.           
inline void fast_srand(int seed) noexcept
{
    g_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
inline int fast_rand()
{
    g_seed = (214013 * g_seed + 2531011);
    return static_cast<int>((g_seed >> 16) & 0x7FFF);
}

static bool Bark(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;
static bool Eat(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;
static ::std::int32_t SelectRandomAction(PetManager& petManager, const BehaviorTreeSelectorNode& node, Blackboard& blackboard) noexcept;
static bool Sleep3s(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;

static bool ContinueTree(PetManager& petManager, const BehaviorTreeRepeatNode& node, Blackboard& blackboard) noexcept;

static BlackboardKey s_SleepTimeKey;
static const BlackboardKeyName::KeyChar* s_SleepTimeKeyName = u8"Sleep5s.Time";

static BlackboardKey s_BarkSequenceKey;
static const BlackboardKeyName::KeyChar* s_BarkSequenceKeyName = u8"BarkSequence.SequenceKey";

static BlackboardKey s_ActionSelectorKey;
static const BlackboardKeyName::KeyChar* s_ActionSelectorKeyName = u8"ActionSelector.SelectorKey";

static BehaviorTreeActionNode s_BarkAction(Bark);
static BehaviorTreeActionNode s_EatAction(Eat);
static BehaviorTreeActionNode s_SleepAction(Sleep3s);

static ::std::array<BehaviorTreeNode*, 2> s_RandomActionSelectionArray({ &s_BarkAction, &s_EatAction });

static BehaviorTreeSelectorNode s_RandomActionSelector(s_RandomActionSelectionArray.size(), s_RandomActionSelectionArray.data(), SelectRandomAction, s_ActionSelectorKey);

static ::std::array<BehaviorTreeNode*, 2> s_BarkSequenceArray({ &s_RandomActionSelector, &s_SleepAction });

static BehaviorTreeSequenceNode s_BarkSequence(s_BarkSequenceArray.size(), s_BarkSequenceArray.data(), s_BarkSequenceKey);

BehaviorTreeRepeatNode g_RootNode(&s_BarkSequence, ContinueTree);

void InitBlackboardKeys(BlackboardKeyManager& keyManager) noexcept
{
    s_SleepTimeKey = keyManager.CalculateKey(s_SleepTimeKeyName, sizeof(int32_t));
    s_BarkSequenceKey = keyManager.CalculateKey(s_BarkSequenceKeyName, sizeof(BehaviorTreeSequenceNode::SequenceKeyT));
    s_BarkSequence.SequenceKey() = s_BarkSequenceKey;
    s_ActionSelectorKey = keyManager.CalculateKey(s_ActionSelectorKeyName, sizeof(BehaviorTreeSelectorNode::SelectorKeyT));
    s_RandomActionSelector.SelectorKey() = s_ActionSelectorKey;
}

static bool Bark(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, const float deltaTime) noexcept
{
    (void) petManager;
    (void) node;
    (void) blackboard;
    (void) deltaTime;

    DebugPrintF(u8"Bork bork!\n");

    return true;
}

static bool Eat(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, const float deltaTime) noexcept
{
    (void) petManager;
    (void) node;
    (void) blackboard;
    (void) deltaTime;

    DebugPrintF(u8"Nom nom!\n");

    return true;
}

static ::std::int32_t SelectRandomAction(PetManager& petManager, const BehaviorTreeSelectorNode& node, Blackboard& blackboard) noexcept
{
    (void) petManager;
    (void) node;
    (void) blackboard;

    const ::std::uint32_t randNum = static_cast<::std::uint32_t>(fast_rand()) % 100;

    if(randNum < 80)
    {
        return 0;
    }

    return 1;
}

static bool Sleep3s(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, const float deltaTime) noexcept
{
    (void) petManager;
    (void) node;

    ::std::int32_t* pTimeRemaining = blackboard.GetT<::std::int32_t>(s_SleepTimeKey);

    if(!pTimeRemaining)
    {
        return true;
    }

    if(*pTimeRemaining == 0)
    {
        *pTimeRemaining = 3000;
    }

    *pTimeRemaining -= static_cast<::std::int32_t>(deltaTime * 1000.0f);

    if(*pTimeRemaining <= 0)
    {
        *pTimeRemaining = 0;
        return true;
    }

    return false;
}

static bool ContinueTree(PetManager& petManager, const BehaviorTreeRepeatNode& node, Blackboard& blackboard) noexcept
{
    (void) petManager;
    (void) node;
    (void) blackboard;

    return true;
}
