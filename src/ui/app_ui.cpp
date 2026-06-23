#include "app_ui.h"
#include <cstdio>
#include <algorithm>

static const ImVec4 WHITE = {1.f, 1.f, 1.f, 1.f};
static const ImVec4 DIM   = {0.6f, 0.6f, 0.6f, 1.f};

// ─── レベルカラー ────────────────────────────────────────

ImVec4 AppUI::levelColor(ProbLevel level) {
    // HSL 彩度 50%（元の 100% から半減）
    switch (level) {
        case ProbLevel::High:    return {0.83f, 0.48f, 0.48f, 1.f};  // 赤 (desaturated)
        case ProbLevel::Middle:  return {0.83f, 0.74f, 0.48f, 1.f};  // 黄 (desaturated)
        case ProbLevel::Low:     return {0.66f, 0.79f, 0.89f, 1.f};  // 青 (desaturated)
        case ProbLevel::Special: return {0.55f, 0.55f, 0.55f, 1.f};  // 灰 (変更なし)
    }
    return {0.6f, 0.6f, 0.6f, 1.f};
}

ImVec4 AppUI::levelBg(ProbLevel level) {
    switch (level) {
        case ProbLevel::High:    return {0.55f, 0.12f, 0.12f, 0.7f};  // 赤
        case ProbLevel::Middle:  return {0.55f, 0.42f, 0.1f,  0.7f};  // 黄
        case ProbLevel::Low:     return {0.15f, 0.3f,  0.55f, 0.7f};  // 青
        case ProbLevel::Special: return {0.25f, 0.25f, 0.25f, 0.7f};  // 灰
    }
    return {0.25f, 0.25f, 0.25f, 0.7f};
}

// ─── メイン描画 ──────────────────────────────────────────
//
//  レイアウト構造:
//    [ヘッダー行] Progression ........ [現在コード名]
//    ─────────────────────────────────────────────────
//    [C Major]    [C Minor]    [Speed + History]
//

void AppUI::render(MarkovEngine& engine) {
    AppState state = engine.getState();

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("##Main", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);

    const float sp     = ImGui::GetStyle().ItemSpacing.x;
    const float totalW = ImGui::GetContentRegionAvail().x;
    const float bodyH  = ImGui::GetContentRegionAvail().y;

    // 右パネル幅・各進行カラム幅
    const float rightW = 360.f;
    const float colW   = (totalW - rightW - 2.f * sp) / 2.f;

    // ── 列順: [Progression・Speed・History] [C Major] [C Minor] ──
    // 共通スタイル: 白枠 (85%) + 暗グレー背景
    const ImVec4 panelBg  = ImVec4(0.07f, 0.07f, 0.07f, 1.f);
    const ImVec4 panelBdr = ImVec4(1.f, 1.f, 1.f, 0.85f);

    // 左: Progression・Speed・History（外枠なし、内部各ボックスが独立）
    ImGui::BeginChild("##Ctrl", ImVec2(rightW, bodyH), false);
    renderControls(engine, state);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, panelBg);
    ImGui::PushStyleColor(ImGuiCol_Border,  panelBdr);
    ImGui::BeginChild("##Maj", ImVec2(colW, bodyH), true);
    renderRuleSection("C Major", true,
                      state.majorRule, state.majorProbs, state.notes, engine);
    ImGui::EndChild();
    ImGui::PopStyleColor(2);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, panelBg);
    ImGui::PushStyleColor(ImGuiCol_Border,  panelBdr);
    ImGui::BeginChild("##Min", ImVec2(colW, bodyH), true);
    renderRuleSection("C Minor", false,
                      state.minorRule, state.minorProbs, state.notes, engine);
    ImGui::EndChild();
    ImGui::PopStyleColor(2);

    ImGui::End();
}

// ─── ルールセクション ────────────────────────────────────

void AppUI::renderRuleSection(const char* title, bool isMajor,
                               const std::vector<TransitionEntry>& rules,
                               const std::vector<float>& probs,
                               const std::vector<std::string>& notes,
                               MarkovEngine& engine) {
    ImGui::PushID(isMajor ? "maj" : "min");

    ImGui::TextColored(DIM, "%s", title);
    ImGui::Separator();
    ImGui::Spacing();

    // 行間を少し広げてバッジと次の行が重ならないように
    const ImVec2 origSp = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(origSp.x, origSp.y + 4.f));

    int refRoot = 0;
    for (int i = 0; i < (int)rules.size(); i++) {
        int target = (refRoot + rules[i].interval) % 12;
        std::string targetName =
            notes[target] + (rules[i].quality == "maj" ? " Major" : " Minor");

        float prob = (i < (int)probs.size()) ? probs[i] : 0.f;
        renderTransitionRow(i, targetName, rules[i], prob, isMajor, engine);
    }

    ImGui::PopStyleVar();

    ImGui::PopID();
}

// ─── 遷移1行 ────────────────────────────────────────────

void AppUI::renderTransitionRow(int idx, const std::string& targetName,
                                 const TransitionEntry& entry, float probPct,
                                 bool isMajor, MarkovEngine& engine) {
    ImGui::PushID(idx);

    ImGui::TextColored(WHITE, "%s", targetName.c_str());
    ImGui::SameLine(200.f);

    // ── レベルバッジ: ピル型プルダウン ──────────────────
    // Speed と同じ仕組み: 背景は常に黒、ホバーは薄い白オーバーレイのみ
    {
        const ImVec4 lc  = levelColor(entry.level);
        const ImVec4 bg  = { 0.f, 0.f, 0.f, 1.f };
        const ImVec4 bgH = { 1.f, 1.f, 1.f, 0.08f };   // Speed ホバーと同系統
        const ImVec4 bgA = { 1.f, 1.f, 1.f, 0.15f };
        const ImVec4 bdr = { 1.f, 1.f, 1.f, 0.85f };

        ImGui::PushStyleColor(ImGuiCol_Button,        bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgH);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bgA);
        ImGui::PushStyleColor(ImGuiCol_Text,          lc);
        ImGui::PushStyleColor(ImGuiCol_Border,        bdr);
        // FramePadding を大きくしてボタン領域を 1.2 倍に
        const ImVec2 origPad = ImGui::GetStyle().FramePadding;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(origPad.x * 1.5f, origPad.y * 1.4f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   100.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

        char btnLabel[32];
        snprintf(btnLabel, sizeof(btnLabel), " %s   ", probLevelName(entry.level));
        if (ImGui::Button(btnLabel))
            ImGui::OpenPopup("PickLevel");

        // ▼ 小三角: 常に白
        {
            const ImVec2 rMin = ImGui::GetItemRectMin();
            const ImVec2 rMax = ImGui::GetItemRectMax();
            const float  cy   = (rMin.y + rMax.y) * 0.5f + 0.5f;
            const float  cx   = rMax.x - 9.f;
            const float  r    = 3.f;
            ImGui::GetWindowDrawList()->AddTriangleFilled(
                { cx - r, cy - r * 0.5f },
                { cx + r, cy - r * 0.5f },
                { cx,     cy + r * 0.8f },
                IM_COL32(255, 255, 255, 210));
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(5);
    }

    ImGui::SameLine();

    char probStr[16];
    snprintf(probStr, sizeof(probStr), "%.1f%%", probPct);
    ImGui::TextColored(WHITE, "%s", probStr);

    if (ImGui::BeginPopup("PickLevel")) {
        ImGui::TextColored(WHITE, "%s", targetName.c_str());
        ImGui::Separator();

        ProbLevel levels[] = {ProbLevel::High, ProbLevel::Middle,
                              ProbLevel::Low,  ProbLevel::Special};
        for (auto lv : levels) {
            bool sel = (entry.level == lv);
            ImGui::PushStyleColor(ImGuiCol_Text,
                sel ? levelColor(lv) : ImVec4(0.8f, 0.8f, 0.8f, 1.f));
            char buf[48];
            snprintf(buf, sizeof(buf), "%s%s", sel ? "> " : "  ", probLevelName(lv));
            if (ImGui::Selectable(buf, sel))
                engine.setTransitionLevel(isMajor, idx, lv);
            ImGui::PopStyleColor();
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();
}

// ─── コントロールパネル ──────────────────────────────────

void AppUI::renderControls(MarkovEngine& engine, const AppState& state) {
    // ── Progression ボックス ────────────────────────────────
    // ラベル + 区切り線 + コード名 + Triad を矩形で囲んで表示
    {
        const std::string chordName = state.current.name(state.notes);
        const auto& t = state.triad;
        const float lh  = ImGui::GetTextLineHeightWithSpacing();
        const float boxH = lh * 4.0f;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.07f, 0.07f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border,  ImVec4(1.f, 1.f, 1.f, 0.85f));
        if (ImGui::BeginChild("##ProgBox", ImVec2(-1, boxH), true,
                              ImGuiWindowFlags_NoScrollbar)) {
            ImGui::TextColored(DIM, "Progression");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(WHITE, "%s", chordName.c_str());
            char triadBuf[32];
            snprintf(triadBuf, sizeof(triadBuf), "[%d, %d, %d]", t[0], t[1], t[2]);
            ImGui::TextColored(DIM, "%s", triadBuf);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
    }

    const ImVec4 boxBg  = ImVec4(0.07f, 0.07f, 0.07f, 1.f);
    const ImVec4 boxBdr = ImVec4(1.f, 1.f, 1.f, 0.85f);
    const float  lh     = ImGui::GetTextLineHeightWithSpacing();
    const float  spY    = ImGui::GetStyle().ItemSpacing.y;
    const float  padY   = ImGui::GetStyle().WindowPadding.y;

    ImGui::Spacing();

    // ── Speed ボックス ─────────────────────────────────────
    {
        const float btnH   = 34.f;
        const float boxH   = lh + spY + btnH + padY * 2.f;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, boxBg);
        ImGui::PushStyleColor(ImGuiCol_Border,  boxBdr);
        if (ImGui::BeginChild("##SpeedBox", ImVec2(-1, boxH), true,
                              ImGuiWindowFlags_NoScrollbar)) {
            ImGui::TextColored(DIM, "Speed");
            ImGui::Spacing();

            bool paused = state.paused;
            const float pauseW = btnH;
            const float itemSp = ImGui::GetStyle().ItemSpacing.x;
            const float totalW = ImGui::GetContentRegionAvail().x;
            const float pillW  = totalW - pauseW - itemSp;
            const float segW   = pillW / 3.f;
            const float pillR  = btnH * 0.5f;

            const SpeedSetting speeds[] = {SpeedSetting::Lo, SpeedSetting::Middle, SpeedSetting::Hi};
            const ImVec2 pillMin = ImGui::GetCursorScreenPos();
            const ImVec2 pillMax = { pillMin.x + pillW, pillMin.y + btnH };
            ImDrawList* dl = ImGui::GetWindowDrawList();

            dl->AddRectFilled(pillMin, pillMax, IM_COL32(22, 22, 22, 240), pillR);
            for (int i = 1; i < 3; i++) {
                float x = pillMin.x + segW * i;
                dl->AddLine({ x, pillMin.y + 6.f }, { x, pillMax.y - 6.f },
                            IM_COL32(80, 80, 80, 160), 1.f);
            }
            dl->AddRect(pillMin, pillMax, IM_COL32(70, 70, 70, 180), pillR, 0, 1.f);
            for (int i = 0; i < 3; i++) {
                if (state.speed != speeds[i]) continue;
                ImVec2 sMin = { pillMin.x + segW * i,       pillMin.y };
                ImVec2 sMax = { pillMin.x + segW * (i + 1), pillMax.y };
                ImDrawFlags cf = (i == 0) ? ImDrawFlags_RoundCornersLeft  :
                                 (i == 2) ? ImDrawFlags_RoundCornersRight  :
                                            ImDrawFlags_RoundCornersNone;
                dl->AddRect(sMin, sMax, IM_COL32(255, 255, 255, 220), pillR, cf, 1.5f);
            }

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1.f, 1.f, 1.f, 0.07f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(1.f, 1.f, 1.f, 0.14f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,   0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(0.f, itemSp));
            for (int i = 0; i < 3; i++) {
                const bool sel = (state.speed == speeds[i]);
                ImGui::PushStyleColor(ImGuiCol_Text,
                    sel ? ImVec4(1.f, 1.f, 1.f, 1.f) : ImVec4(0.45f, 0.45f, 0.45f, 1.f));
                if (ImGui::Button(speedName(speeds[i]), ImVec2(segW, btnH)))
                    engine.setSpeed(speeds[i]);
                ImGui::PopStyleColor();
                if (i < 2) ImGui::SameLine();
            }
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            // Pause ボタン（枠なし・アイコンのみ）
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
            ImGui::Button("##pause", ImVec2(pauseW, btnH));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            bool pauseClicked = ImGui::IsItemClicked();
            {
                ImVec2 p  = ImGui::GetItemRectMin();
                ImVec2 p2 = ImGui::GetItemRectMax();
                float cx  = (p.x + p2.x) * 0.5f;
                float cy  = (p.y + p2.y) * 0.5f;
                const ImU32 ic = IM_COL32(255, 255, 255, 230);
                if (paused) {
                    float r = btnH * 0.28f;
                    dl->AddTriangleFilled({cx - r*0.7f, cy - r}, {cx - r*0.7f, cy + r}, {cx + r, cy}, ic);
                } else {
                    float bw = btnH * 0.1f, bh = btnH * 0.32f, g = btnH * 0.1f;
                    dl->AddRectFilled({cx-g-bw, cy-bh}, {cx-g,    cy+bh}, ic);
                    dl->AddRectFilled({cx+g,    cy-bh}, {cx+g+bw, cy+bh}, ic);
                }
            }
            if (pauseClicked) engine.setPaused(!state.paused);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();

    // ── History ボックス ────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, boxBg);
    ImGui::PushStyleColor(ImGuiCol_Border,  boxBdr);
    if (ImGui::BeginChild("##HistBox", ImVec2(-1, 0), true)) {
        ImGui::TextColored(DIM, "History");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::BeginChild("HistScroll", ImVec2(0, 0), false);
        for (int i = (int)state.history.size() - 1; i >= 0; i--) {
            const auto& h = state.history[i];
            float alpha = 0.3f + 0.7f * float(i) / std::max(1.f, float(state.history.size() - 1));
            char buf[64];
            snprintf(buf, sizeof(buf), "%-6s  (%.1fs)", h.chordName.c_str(), h.waitSeconds);
            ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, alpha), "%s", buf);
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
}
