#pragma once

class BehaviorTreeRepeatNode;
class BlackboardKeyManager;

extern BehaviorTreeRepeatNode g_RootNode;

void InitBlackboardKeys(BlackboardKeyManager& keyManager) noexcept;
