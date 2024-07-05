#include "BehaviorTree.hpp"

void BehaviorTreeNode::Execute(BehaviorTreeExecutor& executor) const noexcept
{
}

void BehaviorTreeSequenceNode::Execute(BehaviorTreeExecutor& executor) const noexcept
{
    executor.Execute(*this);
}

void BehaviorTreeSelectorNode::Execute(BehaviorTreeExecutor& executor) const noexcept
{
    executor.Execute(*this);
}

void BehaviorTreeRepeatNode::Execute(BehaviorTreeExecutor& executor) const noexcept
{
    executor.Execute(*this);
}

void BehaviorTreeActionNode::Execute(BehaviorTreeExecutor& executor) const noexcept
{
    executor.Execute(*this);
}

void BehaviorTreeExecutor::Tick(const float deltaTime) noexcept
{
    if(!m_Root)
    {
        return;
    }

    if(m_CurrentState == Uninitialized)
    {
        InitState();

        if(!m_Root->Continuation()(*m_PetManager, *m_Root, *m_Blackboard))
        {
            return;
        }

        m_Current = m_Root->Children()[0];

        m_CurrentState = FinishedNode;
    }

    if(!m_Current)
    {
        return;
    }

    m_CurrentDeltaTime = deltaTime;

    if(m_CurrentState == Running)
    {
        m_Current->Execute(*this);
    }

    if(m_CurrentState == FinishedNode)
    {
        const BehaviorTreeNode* parent = m_Current->Parent();

        parent->Execute(*this);

        m_CurrentState = Running;
    }
}

static ::std::int32_t SequenceActionHandler(const BehaviorTreeSequenceNode& node, Blackboard& blackboard, const BlackboardKey sequenceKey) noexcept
{
    BehaviorTreeSequenceNode::SequenceKeyT* currentIndex = blackboard.GetT<BehaviorTreeSequenceNode::SequenceKeyT>(sequenceKey);

    const BehaviorTreeSequenceNode::SequenceKeyT retVal = *currentIndex;

    ++(*currentIndex);

    if(static_cast<::std::uint32_t>(*currentIndex) >= node.ChildCount())
    {
        *currentIndex = 0;
    }

    return retVal;
}

void BehaviorTreeExecutor::Execute(const BehaviorTreeNode* const node) noexcept
{
    if(node)
    {
        Execute(*node);
    }
}

void BehaviorTreeExecutor::Execute(const BehaviorTreeNode& node) noexcept
{
    node.Execute(*this);
}

void BehaviorTreeExecutor::Execute(const BehaviorTreeSequenceNode& node) noexcept
{
    const ::std::int32_t nodeIndex = SequenceActionHandler(node, *m_Blackboard, node.SequenceKey());

    if(nodeIndex < 0 || static_cast<::std::uint32_t>(nodeIndex) >= node.ChildCount())
    {
        Execute(node.Parent());
        return;
    }

    Execute(node.Children()[nodeIndex]);
}

void BehaviorTreeExecutor::Execute(const BehaviorTreeSelectorNode& node) noexcept
{
    BehaviorTreeSelectorNode::SelectorKeyT* selector = m_Blackboard->GetT<BehaviorTreeSelectorNode::SelectorKeyT>(node.SelectorKey());
    if(*selector)
    {
        *selector = false;
        Execute(node.Parent());
        return;
    }

    const ::std::int32_t nodeIndex = node.Selector()(*m_PetManager, node, *m_Blackboard);

    if(nodeIndex < 0 || static_cast<::std::uint32_t>(nodeIndex) >= node.ChildCount())
    {
        Execute(node.Parent());
        return;
    }

    *selector = true;

    Execute(node.Children()[nodeIndex]);
}

void BehaviorTreeExecutor::Execute(const BehaviorTreeRepeatNode& node) noexcept
{
    if(!node.Continuation()(*m_PetManager, node, *m_Blackboard))
    {
        Execute(node.Parent());
        return;
    }

    Execute(node.Children()[0]);
}

void BehaviorTreeExecutor::Execute(const BehaviorTreeActionNode& node) noexcept
{
    if(m_CurrentState == FinishedNode)
    {
        m_Current = &node;
        return;
    }

    if(node.Handler()(*m_PetManager, node, *m_Blackboard, m_CurrentDeltaTime))
    {
        m_CurrentState = FinishedNode;
    }
}

void BehaviorTreeExecutor::InitState() noexcept
{
    if(!m_Root)
    {
        return;
    }

    if(m_Root->StateIndex() < 0)
    {
        InitChildren(m_Root, 0);
    }
}

::std::int32_t BehaviorTreeExecutor::CountChildren(const BehaviorTreeNode* const node) const noexcept
{
    if(!node)
    {
        return 0;
    }

    ::std::int32_t count = 1;

    for(::std::uint32_t i = 0; i < node->ChildCount(); ++i)
    {
        count += CountChildren(node->Children()[i]);
    }

    return count;
}

::std::int32_t BehaviorTreeExecutor::InitChildren(BehaviorTreeNode* const node, const ::std::int32_t startIndex) const noexcept
{
    if(!node)
    {
        return 0;
    }

    ::std::int32_t index = startIndex;

    node->StateIndex() = index++;

    for(::std::uint32_t i = 0; i < node->ChildCount(); ++i)
    {
        index = InitChildren(node->Children()[i], index);
    }

    return index;
}
