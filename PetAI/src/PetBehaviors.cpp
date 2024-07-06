// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "BehaviorTree.hpp"
#include "PetManager.hpp"
#include "Blackboard.hpp"
#include "SysLib.h"
#include <array>

static bool Bark(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;
static bool Eat(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;
static ::std::int32_t SelectRandomAction(PetManager& petManager, const BehaviorTreeSelectorNode& node, Blackboard& blackboard) noexcept;
static bool Sleep3s(PetManager& petManager, const BehaviorTreeActionNode& node, Blackboard& blackboard, float deltaTime) noexcept;

static ::std::int32_t SelectLifeStageTree(PetManager& petManager, const BehaviorTreeSelectorNode& node, Blackboard& blackboard) noexcept;
static bool ContinueTree(PetManager& petManager, const BehaviorTreeRepeatNode& node, Blackboard& blackboard) noexcept;

static BlackboardKey s_SleepTimeKey;
static const BlackboardKeyName::KeyChar* s_SleepTimeKeyName = CSTR("Sleep5s.Time");

static BlackboardKey s_BarkSequenceKey;
static const BlackboardKeyName::KeyChar* s_BarkSequenceKeyName = CSTR("BarkSequence.SequenceKey");

static BlackboardKey s_ActionSelectorKey;
static const BlackboardKeyName::KeyChar* s_ActionSelectorKeyName = CSTR("ActionSelector.SelectorKey");

static BlackboardKey s_LifeStageSelectorKey;
static const BlackboardKeyName::KeyChar* s_LifeStageSelectorKeyName = CSTR("LifeStage.SelectorKey");

static BlackboardKey s_LifeStageKey;
static const BlackboardKeyName::KeyChar* s_LifeStageKeyName = CSTR("LifeStage");

static BehaviorTreeActionNode s_BarkAction(Bark);
static BehaviorTreeActionNode s_EatAction(Eat);
static BehaviorTreeActionNode s_SleepAction(Sleep3s);

static ::std::array<BehaviorTreeNode*, 2> s_RandomActionSelectionArray({ &s_BarkAction, &s_EatAction });
static BehaviorTreeSelectorNode s_RandomActionSelector(s_RandomActionSelectionArray.size(), s_RandomActionSelectionArray.data(), SelectRandomAction, s_ActionSelectorKey);

static ::std::array<BehaviorTreeNode*, 2> s_BarkSequenceArray({ &s_RandomActionSelector, &s_SleepAction });
static BehaviorTreeSequenceNode s_BarkSequence(s_BarkSequenceArray.size(), s_BarkSequenceArray.data(), s_BarkSequenceKey);

static ::std::array<BehaviorTreeNode*, 5> s_LifeStageTrees({ &s_BarkSequence, &s_BarkSequence, &s_BarkSequence, &s_BarkSequence, &s_BarkSequence });
static BehaviorTreeSelectorNode s_LifeStageSelector(s_LifeStageTrees.size(), s_LifeStageTrees.data(), SelectLifeStageTree, s_LifeStageSelectorKey);

BehaviorTreeRepeatNode g_RootNode(&s_LifeStageSelector, ContinueTree);

enum class LifeStage : uint8_t
{
    Infant = 0,
    Childhood,
    Adolescent,
    Adult,
    Elder,
    MaxValue = Elder
};

void InitBlackboardKeys(BlackboardKeyManager& keyManager) noexcept
{
    s_SleepTimeKey = keyManager.CalculateKey(s_SleepTimeKeyName, sizeof(int32_t));
    s_BarkSequenceKey = keyManager.CalculateKey(s_BarkSequenceKeyName, sizeof(BehaviorTreeSequenceNode::SequenceKeyT));
    s_BarkSequence.SequenceKey() = s_BarkSequenceKey;
    s_ActionSelectorKey = keyManager.CalculateKey(s_ActionSelectorKeyName, sizeof(BehaviorTreeSelectorNode::SelectorKeyT));
    s_RandomActionSelector.SelectorKey() = s_ActionSelectorKey;

    s_LifeStageSelectorKey = keyManager.CalculateKey(s_LifeStageSelectorKeyName, sizeof(BehaviorTreeSelectorNode::SelectorKeyT));
    s_LifeStageSelector.SelectorKey() = s_LifeStageSelectorKey;

    s_LifeStageKey = keyManager.CalculateKey(s_LifeStageKeyName, sizeof(LifeStage));
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

    const ::std::uint32_t randNum = static_cast<::std::uint32_t>(GenerateRandomInt(1, 100));

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

    ::std::int32_t* const pTimeRemaining = blackboard.GetT<::std::int32_t>(s_SleepTimeKey);

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

static ::std::int32_t SelectLifeStageTree(PetManager& petManager, const BehaviorTreeSelectorNode& node, Blackboard& blackboard) noexcept
{
    (void) petManager;
    (void) node;

    if(node.ChildCount() != static_cast<::std::uint32_t>(LifeStage::MaxValue) + 1)
    {
        DebugPrintF(CSTR("[SelectLifeStageTree]: Number of children for life stage selector does not match the number of life stages %u."), static_cast<::std::uint32_t>(LifeStage::MaxValue) + 1);
        return 0;
    }

    LifeStage* const pLifeStage = blackboard.GetT<LifeStage>(s_LifeStageKey);

    // If this is a valid stage simply convert to an int and return that as the index,
    // otherwise reset to LifeStage::Infant.
    switch(*pLifeStage)
    {
        case LifeStage::Infant:
        case LifeStage::Childhood:
        case LifeStage::Adolescent:
        case LifeStage::Adult:
        case LifeStage::Elder:
            break;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            *pLifeStage = LifeStage::Infant;
            break;
    }

    return static_cast<::std::int32_t>(*pLifeStage);
}

static bool ContinueTree(PetManager& petManager, const BehaviorTreeRepeatNode& node, Blackboard& blackboard) noexcept
{
    (void) petManager;
    (void) node;
    (void) blackboard;

    return true;
}
