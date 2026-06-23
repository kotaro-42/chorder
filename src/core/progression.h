#pragma once

#include <vector>
#include <string>
#include <array>
#include <random>
#include <mutex>
#include <atomic>
#include <cmath>
#include <fstream>
#include <sstream>

// ─── 確率レベル（4段階）──────────────────────

enum class ProbLevel { High, Middle, Low, Special };

constexpr int PROB_LEVEL_COUNT = 4;

inline const char* probLevelName(ProbLevel l) {
    switch (l) {
        case ProbLevel::High:    return "High";
        case ProbLevel::Middle:  return "Middle";
        case ProbLevel::Low:     return "Low";
        case ProbLevel::Special: return "Special";
    }
    return "?";
}

// ─── 確率設定（2段階計算）────────────────────
//
//  1) Special 1件あたり specialPercent% を確保
//     → Special が n件なら合計 n × specialPercent%（最大100%に制限）
//  2) 残り (100 - n×specialPercent)% を
//     High : Middle : Low の比率で分配
//

struct ProbConfig {
    float specialPercent = 1.0f;   // Special 1件あたりの割合 (%)
    float highRatio      = 9.0f;   // High/Middle/Low の比率
    float middleRatio    = 3.0f;
    float lowRatio       = 1.0f;

    float getRatio(ProbLevel level) const {
        switch (level) {
            case ProbLevel::High:   return highRatio;
            case ProbLevel::Middle: return middleRatio;
            case ProbLevel::Low:    return lowRatio;
            default:                return 0.f;
        }
    }
};

// ─── 速度設定 ────────────────────────────────

enum class SpeedSetting { Lo, Middle, Hi };

inline const char* speedName(SpeedSetting s) {
    switch (s) {
        case SpeedSetting::Lo:     return "lo";
        case SpeedSetting::Middle: return "middle";
        case SpeedSetting::Hi:     return "hi";
    }
    return "?";
}

struct SpeedConfig {
    float lo  = 3.0f;
    float mid = 1.0f;
    float hi  = 1.0f / 3.0f;

    float getMultiplier(SpeedSetting s) const {
        switch (s) {
            case SpeedSetting::Lo:     return lo;
            case SpeedSetting::Middle: return mid;
            case SpeedSetting::Hi:     return hi;
        }
        return mid;
    }
};

// ─── 遷移エントリ（相対・interval ベース）──────

struct TransitionEntry {
    int         interval;    // 半音数（0–11）
    std::string quality;     // "maj" or "min"
    ProbLevel   level;
};

// ─── コード状態 ──────────────────────────────

struct ChordState {
    int         root    = 0;
    std::string quality = "min";

    std::string name(const std::vector<std::string>& notes) const {
        return notes[root] + (quality == "maj" ? " Major" : " Minor");
    }

    // コードを3つの整数 [root, 3rd, 5th] に変換する
    // クロマチックスケール 1始まり (C=1 … B=12) で表現
    // Major: intervals +0, +4, +7  例: C major → [1, 5, 8]
    // Minor: intervals +0, +3, +7  例: C minor → [1, 4, 8]
    std::array<int, 3> toTriad() const {
        const int third = (quality == "maj") ? 4 : 3;
        return {
            (root % 12) + 1,
            ((root + third) % 12) + 1,
            ((root + 7)     % 12) + 1
        };
    }

    bool operator==(const ChordState& o) const {
        return root == o.root && quality == o.quality;
    }
};

// ─── 履歴 ────────────────────────────────────

struct HistoryEntry {
    std::string chordName;
    double      waitSeconds;
};

// ─── アプリ状態（UIスレッド読み取り用スナップショット）──

struct AppState {
    ChordState                   current;
    std::array<int, 3>           triad;        // 現在コードの数値表現 [root, 3rd, 5th]
    std::vector<TransitionEntry> majorRule;
    std::vector<TransitionEntry> minorRule;
    std::vector<float>           majorProbs;   // 計算済み確率 (%)
    std::vector<float>           minorProbs;
    ProbConfig                   probConfig;
    SpeedSetting                 speed;
    SpeedConfig                  speedConfig;
    std::vector<HistoryEntry>    history;
    std::vector<std::string>     notes;
    bool                         paused;
};

// ─── マルコフ生成エンジン ────────────────────

class MarkovEngine {
public:
    static constexpr size_t MAX_HISTORY = 40;

    MarkovEngine();

    void          step();
    double        getWaitSeconds();
    std::string   currentChordName();
    void          addToHistory(const std::string& chord, double wait);

    AppState getState();

    // UI → エンジンへの変更
    void setTransitionLevel(bool isMajor, int index, ProbLevel level);
    void setProbConfig(const ProbConfig& cfg);
    void setSpeed(SpeedSetting s);
    void setSpeedConfig(const SpeedConfig& cfg);
    void setPaused(bool p);
    bool isPaused();

    // 設定の保存・復元
    // 保存対象: 各遷移のProbLevel + Speed
    // ファイルが存在しない場合は何もせず false を返す
    bool saveSettings(const std::string& path) const;
    bool loadSettings(const std::string& path);

    // 2段階確率計算（static: UIからも利用可能）
    static std::vector<float> computeProbabilities(
        const std::vector<TransitionEntry>& rules,
        const ProbConfig& config);

    std::atomic<bool> running{true};

private:
    std::vector<std::string>     notes_;
    std::vector<TransitionEntry> majorRule_;
    std::vector<TransitionEntry> minorRule_;
    ProbConfig                   probConfig_;
    SpeedSetting                 speed_  = SpeedSetting::Middle;
    SpeedConfig                  speedConfig_;
    ChordState                   current_;
    bool                         paused_ = false;
    std::vector<double>          baseTimeChoices_;
    std::vector<HistoryEntry>    history_;
    std::mt19937                 gen_;
    mutable std::mutex           mtx_;
};
