#pragma once

#include "imgui.h"
#include "progression.h"
#include <string>

class AppUI {
public:
    void render(MarkovEngine& engine);

private:
    void renderRuleSection  (const char* title, bool isMajor,
                             const std::vector<TransitionEntry>& rules,
                             const std::vector<float>& probs,
                             const std::vector<std::string>& notes,
                             MarkovEngine& engine);
    void renderTransitionRow(int idx, const std::string& targetName,
                             const TransitionEntry& entry, float probPct,
                             bool isMajor, MarkovEngine& engine);
    void renderControls     (MarkovEngine& engine, const AppState& state);

    static ImVec4 levelColor(ProbLevel level);
    static ImVec4 levelBg   (ProbLevel level);
};
