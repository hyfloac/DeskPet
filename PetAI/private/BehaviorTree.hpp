#pragma once

#include <cstdint>
#include <functional>
#include "Objects.hpp"
#include "Blackboard.hpp"

class BehaviorTreeSequenceNode;
class BehaviorTreeSelectorNode;
class BehaviorTreeRepeatNode;
class BehaviorTreeActionNode;

class BehaviorTreeExecutor;

class PetManager;

class BehaviorTreeNode
{
    DEFAULT_CONSTRUCT_PO(BehaviorTreeNode);
    DEFAULT_CM_PO(BehaviorTreeNode);
    DEFAULT_DESTRUCT_VI(BehaviorTreeNode);
protected:
    BehaviorTreeNode(BehaviorTreeNode* const parent) noexcept
        : m_Parent(parent)
        , m_StateIndex(-1)
    { }
public:
    [[nodiscard]]       BehaviorTreeNode*& Parent()       noexcept { return m_Parent; }
    [[nodiscard]] const BehaviorTreeNode*  Parent() const noexcept { return m_Parent; }

    [[nodiscard]] ::std::int32_t& StateIndex()       noexcept { return m_StateIndex; }
    [[nodiscard]] ::std::int32_t  StateIndex() const noexcept { return m_StateIndex; }

    [[nodiscard]] virtual ::std::uint32_t ChildCount() const noexcept { return 0; }
    [[nodiscard]] virtual       BehaviorTreeNode* const* Children()	      noexcept { return nullptr; }
    [[nodiscard]] virtual const BehaviorTreeNode* const* Children() const noexcept { return const_cast<BehaviorTreeNode*>(this)->Children(); }

    [[nodiscard]] virtual       BehaviorTreeSequenceNode* AsSequence()       noexcept { return nullptr; }
    [[nodiscard]] virtual const BehaviorTreeSequenceNode* AsSequence() const noexcept { return const_cast<BehaviorTreeNode*>(this)->AsSequence(); }

    [[nodiscard]] virtual       BehaviorTreeSelectorNode* AsSelector()       noexcept { return nullptr; }
    [[nodiscard]] virtual const BehaviorTreeSelectorNode* AsSelector() const noexcept { return const_cast<BehaviorTreeNode*>(this)->AsSelector(); }

    [[nodiscard]] virtual       BehaviorTreeRepeatNode* AsRepeat()       noexcept { return nullptr; }
    [[nodiscard]] virtual const BehaviorTreeRepeatNode* AsRepeat() const noexcept { return const_cast<BehaviorTreeNode*>(this)->AsRepeat(); }

    [[nodiscard]] virtual       BehaviorTreeActionNode* AsAction()       noexcept { return nullptr; }
    [[nodiscard]] virtual const BehaviorTreeActionNode* AsAction() const noexcept { return const_cast<BehaviorTreeNode*>(this)->AsAction(); }

    virtual void Execute(BehaviorTreeExecutor& executor) const noexcept;
private:
    BehaviorTreeNode* m_Parent;
    ::std::int32_t m_StateIndex;
};

class BehaviorTreeContainerNode : public BehaviorTreeNode
{
    DEFAULT_CONSTRUCT_PO(BehaviorTreeContainerNode);
    DEFAULT_CM_PO(BehaviorTreeContainerNode);
    DEFAULT_DESTRUCT_O(BehaviorTreeContainerNode);
protected:
    BehaviorTreeContainerNode(
        const ::std::uint32_t childCount, 
        BehaviorTreeNode** const children
    ) noexcept
        : m_ChildCount(childCount)
        , m_Children(children)
    {
        InitChildren();
    }
public:
    void InitChildren()
    {
        if(!m_Children)
        {
            return;
        }

        for(::std::uint32_t i = 0; i < m_ChildCount; ++i)
        {
            if(m_Children[i])
            {
                m_Children[i]->Parent() = this;
            }
        }
    }

    [[nodiscard]] ::std::uint32_t ChildCount() const noexcept override { return m_ChildCount; }
    [[nodiscard]]       BehaviorTreeNode* const* Children()       noexcept override { return m_Children; }
    [[nodiscard]] const BehaviorTreeNode* const* Children() const noexcept override { return m_Children; }

private:
    ::std::uint32_t m_ChildCount;
    BehaviorTreeNode** m_Children;
};

class BehaviorTreeSequenceNode : public BehaviorTreeContainerNode
{
    DEFAULT_CONSTRUCT_PU(BehaviorTreeSequenceNode);
    DEFAULT_CM_PU(BehaviorTreeSequenceNode);
    DEFAULT_DESTRUCT_O(BehaviorTreeSequenceNode);
public:
    BehaviorTreeSequenceNode(
        const ::std::uint32_t childCount,
        BehaviorTreeNode** const children
    ) noexcept
        : BehaviorTreeContainerNode(childCount, children)
    { }

    [[nodiscard]]       BehaviorTreeSequenceNode* AsSequence()       noexcept override { return this; }
    [[nodiscard]] const BehaviorTreeSequenceNode* AsSequence() const noexcept override { return this; }

    void Execute(BehaviorTreeExecutor& executor) const noexcept override;
};

class BehaviorTreeSelectorNode : public BehaviorTreeContainerNode
{
    DEFAULT_CONSTRUCT_PU(BehaviorTreeSelectorNode);
    DEFAULT_CM_PU(BehaviorTreeSelectorNode);
    DEFAULT_DESTRUCT_O(BehaviorTreeSelectorNode);
public:
    using SelectorFunc = ::std::function<::std::int32_t(void*&, PetManager&, const BehaviorTreeSelectorNode&, Blackboard&)>;
public:
    BehaviorTreeSelectorNode(
        const ::std::uint32_t childCount,
        BehaviorTreeNode** const children,
        const SelectorFunc& selector
    ) noexcept
        : BehaviorTreeContainerNode(childCount, children)
        , m_Selector(selector)
    { }

    [[nodiscard]]       BehaviorTreeSelectorNode* AsSelector()       noexcept override { return this; }
    [[nodiscard]] const BehaviorTreeSelectorNode* AsSelector() const noexcept override { return this; }

    void Execute(BehaviorTreeExecutor& executor) const noexcept override;
    
    [[nodiscard]]       SelectorFunc& Selector()       noexcept { return m_Selector; }
    [[nodiscard]] const SelectorFunc& Selector() const noexcept { return m_Selector; }
private:
    SelectorFunc m_Selector;
};

class BehaviorTreeRepeatNode : public BehaviorTreeNode
{
    DEFAULT_CONSTRUCT_PU(BehaviorTreeRepeatNode);
    DEFAULT_CM_PU(BehaviorTreeRepeatNode);
    DEFAULT_DESTRUCT_O(BehaviorTreeRepeatNode);
public:
    using ContinuationFunc = ::std::function<bool(void*&, PetManager&, const BehaviorTreeRepeatNode&, Blackboard&)>;
public:
    BehaviorTreeRepeatNode(
        BehaviorTreeNode* const child,
        const ContinuationFunc& continuation
    ) noexcept
        : BehaviorTreeNode(nullptr)
        , m_Child(child)
        , m_Continuation(continuation)
    {
        if(m_Child)
        {
            m_Child->Parent() = this;
        }

        StateIndex() = -1;
    }

    [[nodiscard]] ::std::uint32_t ChildCount() const noexcept override { return 1; }
    [[nodiscard]]       BehaviorTreeNode* const* Children()       noexcept override { return &m_Child; }
    [[nodiscard]] const BehaviorTreeNode* const* Children() const noexcept override { return &m_Child; }

    [[nodiscard]]       BehaviorTreeRepeatNode* AsRepeat()       noexcept override { return this; }
    [[nodiscard]] const BehaviorTreeRepeatNode* AsRepeat() const noexcept override { return this; }

    void Execute(BehaviorTreeExecutor& executor) const noexcept override;
    
    [[nodiscard]]       ContinuationFunc& Continuation()       noexcept { return m_Continuation; }
    [[nodiscard]] const ContinuationFunc& Continuation() const noexcept { return m_Continuation; }
private:
    BehaviorTreeNode* m_Child;
    ContinuationFunc m_Continuation;
};

class BehaviorTreeActionNode : public BehaviorTreeNode
{
    DEFAULT_CONSTRUCT_PU(BehaviorTreeActionNode);
    DEFAULT_CM_PU(BehaviorTreeActionNode);
    DEFAULT_DESTRUCT_O(BehaviorTreeActionNode);
public:
    using ActionHandler = ::std::function<bool(void*&, PetManager&, const BehaviorTreeActionNode&, Blackboard&, float deltaTime)>;
public:
    BehaviorTreeActionNode(
        const ActionHandler& handler
    ) noexcept
        : BehaviorTreeNode()
        , m_Handler(handler)
    { }

    [[nodiscard]]       BehaviorTreeActionNode* AsAction()       noexcept override { return this; }
    [[nodiscard]] const BehaviorTreeActionNode* AsAction() const noexcept override { return this; }

    void Execute(BehaviorTreeExecutor& executor) const noexcept override;
    
    [[nodiscard]]       ActionHandler& Handler()       noexcept { return m_Handler; }
    [[nodiscard]] const ActionHandler& Handler() const noexcept { return m_Handler; }
private:
    ActionHandler m_Handler;
};

class BehaviorTreeExecutor
{
    DEFAULT_DESTRUCT(BehaviorTreeExecutor);
    DEFAULT_CM_PU(BehaviorTreeExecutor);
public:
    BehaviorTreeExecutor(
        BehaviorTreeRepeatNode* const root,
        Blackboard* const blackboard,
        PetManager* const petManager
    ) noexcept
        : m_Root(root)
        , m_Current(nullptr)
        , m_Blackboard(blackboard)
        , m_PetManager(petManager)
        , m_CurrentState(Uninitialized)
        , m_CurrentDeltaTime(0.0f)
        , m_StateCount(0)
        , m_State(nullptr)
    { }

    void Tick(const float deltaTime) noexcept;

    void Execute(const BehaviorTreeNode* const node) noexcept;
    void Execute(const BehaviorTreeNode& node) noexcept;

    void Execute(const BehaviorTreeSequenceNode& node) noexcept;
    void Execute(const BehaviorTreeSelectorNode& node) noexcept;
    void Execute(const BehaviorTreeRepeatNode& node) noexcept;
    void Execute(const BehaviorTreeActionNode& node) noexcept;
private:
    void InitState() noexcept;
    ::std::int32_t CountChildren(const BehaviorTreeNode* const node) const noexcept;
    ::std::int32_t InitChildren(BehaviorTreeNode* const node, const ::std::int32_t startIndex) const noexcept;
private:
    enum State
    {
        Uninitialized = 0,
        Running,
        FinishedNode
    };
private:
    BehaviorTreeRepeatNode* m_Root;
    const BehaviorTreeNode* m_Current;
    Blackboard* m_Blackboard;
    PetManager* m_PetManager;
    State m_CurrentState;
    float m_CurrentDeltaTime;
    ::std::size_t m_StateCount;
    void** m_State;
};
