#include "progression.h"
#include <algorithm>

MarkovEngine::MarkovEngine() {
    notes_ = {"C","D♭","D","E♭","E","F","G♭","G","A♭","A","B♭","B"};

    majorRule_ = {
        {5,  "maj", ProbLevel::High},
        {7,  "maj", ProbLevel::High},
        {4,  "maj", ProbLevel::High},
        {10, "maj", ProbLevel::Middle},
        {8,  "maj", ProbLevel::Middle},
        {1,  "maj", ProbLevel::Middle},
        {11, "maj", ProbLevel::Middle},
        {4,  "min", ProbLevel::Middle},
        {9,  "min", ProbLevel::Middle},
        {2,  "min", ProbLevel::Middle},
    };

    minorRule_ = {
        {0,  "maj", ProbLevel::High},
        {5,  "maj", ProbLevel::High},
        {7,  "maj", ProbLevel::High},
        {10, "maj", ProbLevel::High},
        {4,  "min", ProbLevel::Middle},
        {2,  "min", ProbLevel::Middle},
        {7,  "min", ProbLevel::Middle},
        {8,  "maj", ProbLevel::Middle},
    };

    // ウェーバー・フェヒナー的時間分布（元コードと同一）
    for (int i = 0; i < 50; i++) {
        double x = double(i) / 49.0;
        baseTimeChoices_.push_back(std::pow(60.0, x));
    }

    std::random_device rd;
    gen_ = std::mt19937(rd());

    // 最初のコードは 12 のメジャーコードからランダムに選択
    std::uniform_int_distribution<> rootDist(0, 11);
    current_ = {rootDist(gen_), "maj"};
}

// ─── 2段階確率計算 ──────────────────────────
//
//  Step 1: Special 1件あたり specialPercent% を確保
//          totalSpecial = nSpecial × specialPercent（最大100%に制限）
//  Step 2: 残り (100 - totalSpecial)% を
//          High:Middle:Low の比率で分配
//
std::vector<float> MarkovEngine::computeProbabilities(
    const std::vector<TransitionEntry>& rules,
    const ProbConfig& config)
{
    int   nSpecial        = 0;
    float normalWeightSum = 0.f;

    for (const auto& r : rules) {
        if (r.level == ProbLevel::Special)
            nSpecial++;
        else
            normalWeightSum += config.getRatio(r.level);
    }

    // Special 1件あたり specialPercent%、合計が100%を超えないよう制限
    float totalSpecialPct = std::min(float(nSpecial) * config.specialPercent, 100.f);
    float remainingPct    = 100.f - totalSpecialPct;

    std::vector<float> probs(rules.size());
    for (size_t i = 0; i < rules.size(); i++) {
        if (rules[i].level == ProbLevel::Special) {
            probs[i] = config.specialPercent;   // 1件あたり固定
        } else {
            float ratio = config.getRatio(rules[i].level);
            probs[i] = (normalWeightSum > 0.f)
                        ? remainingPct * ratio / normalWeightSum
                        : 0.f;
        }
    }
    return probs;
}

// ─── 生成ステップ ────────────────────────────

void MarkovEngine::step() {
    std::lock_guard<std::mutex> lock(mtx_);

    auto* rule = (current_.quality == "maj") ? &majorRule_ : &minorRule_;
    auto probs = computeProbabilities(*rule, probConfig_);

    std::discrete_distribution<> dist(probs.begin(), probs.end());
    int idx = dist(gen_);
    const TransitionEntry& next = (*rule)[idx];

    current_.root    = (current_.root + next.interval) % 12;
    current_.quality = next.quality;
}

double MarkovEngine::getWaitSeconds() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::uniform_int_distribution<> ti(0, (int)baseTimeChoices_.size() - 1);
    return baseTimeChoices_[ti(gen_)] * speedConfig_.getMultiplier(speed_);
}

std::string MarkovEngine::currentChordName() {
    std::lock_guard<std::mutex> lock(mtx_);
    return current_.name(notes_);
}

void MarkovEngine::addToHistory(const std::string& chord, double wait) {
    std::lock_guard<std::mutex> lock(mtx_);
    history_.push_back({chord, wait});
    if (history_.size() > MAX_HISTORY)
        history_.erase(history_.begin());
}

// ─── アプリ状態スナップショット ──────────────

AppState MarkovEngine::getState() {
    std::lock_guard<std::mutex> lock(mtx_);
    return {
        current_,
        current_.toTriad(),
        majorRule_,
        minorRule_,
        computeProbabilities(majorRule_, probConfig_),
        computeProbabilities(minorRule_, probConfig_),
        probConfig_,
        speed_,
        speedConfig_,
        history_,
        notes_,
        paused_
    };
}

// ─── UI → エンジンへの変更 ──────────────────

void MarkovEngine::setTransitionLevel(bool isMajor, int index, ProbLevel level) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto& rule = isMajor ? majorRule_ : minorRule_;
    if (index >= 0 && index < (int)rule.size())
        rule[index].level = level;
}

void MarkovEngine::setProbConfig(const ProbConfig& cfg) {
    std::lock_guard<std::mutex> lock(mtx_);
    probConfig_ = cfg;
}

void MarkovEngine::setSpeed(SpeedSetting s) {
    std::lock_guard<std::mutex> lock(mtx_);
    speed_ = s;
}

void MarkovEngine::setSpeedConfig(const SpeedConfig& cfg) {
    std::lock_guard<std::mutex> lock(mtx_);
    speedConfig_ = cfg;
}

void MarkovEngine::setPaused(bool p) {
    std::lock_guard<std::mutex> lock(mtx_);
    paused_ = p;
}

bool MarkovEngine::isPaused() {
    std::lock_guard<std::mutex> lock(mtx_);
    return paused_;
}

// ─── 設定保存・復元 ──────────────────────────
//
//  ファイル形式（key=value テキスト）:
//    speed=<0|1|2>              0=Lo, 1=Middle, 2=Hi
//    major_<i>=<0|1|2|3>       0=High,1=Middle,2=Low,3=Special
//    minor_<i>=<0|1|2|3>
//

static int levelToInt(ProbLevel l)       { return static_cast<int>(l); }
static ProbLevel intToLevel(int i)       {
    if (i < 0 || i > 3) return ProbLevel::Middle;
    return static_cast<ProbLevel>(i);
}
static int speedToInt(SpeedSetting s)    { return static_cast<int>(s); }
static SpeedSetting intToSpeed(int i)    {
    if (i < 0 || i > 2) return SpeedSetting::Middle;
    return static_cast<SpeedSetting>(i);
}

bool MarkovEngine::saveSettings(const std::string& path) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::ofstream f(path);
    if (!f) return false;

    f << "speed=" << speedToInt(speed_) << "\n";

    for (int i = 0; i < (int)majorRule_.size(); i++)
        f << "major_" << i << "=" << levelToInt(majorRule_[i].level) << "\n";

    for (int i = 0; i < (int)minorRule_.size(); i++)
        f << "minor_" << i << "=" << levelToInt(minorRule_[i].level) << "\n";

    return true;
}

bool MarkovEngine::loadSettings(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;   // ファイルなし → デフォルト値のまま

    std::lock_guard<std::mutex> lock(mtx_);
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        const std::string key = line.substr(0, eq);
        int val = 0;
        try { val = std::stoi(line.substr(eq + 1)); }
        catch (...) { continue; }

        if (key == "speed") {
            speed_ = intToSpeed(val);
        } else if (key.rfind("major_", 0) == 0) {
            int idx = std::stoi(key.substr(6));
            if (idx >= 0 && idx < (int)majorRule_.size())
                majorRule_[idx].level = intToLevel(val);
        } else if (key.rfind("minor_", 0) == 0) {
            int idx = std::stoi(key.substr(6));
            if (idx >= 0 && idx < (int)minorRule_.size())
                minorRule_[idx].level = intToLevel(val);
        }
    }
    return true;
}
