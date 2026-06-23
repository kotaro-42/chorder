#include "audio_engine.h"

#include <iostream>
#include <algorithm>

// ─── ノート名（デバッグ出力用）──────────────────
static const char* kNoteNames[12] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

inline static const char* noteName(int note) {
    if (note < 1 || note > 12) return "?";
    return kNoteNames[note - 1];
}

// ─── コンストラクタ ───────────────────────────

AudioEngine::AudioEngine() {
    // 全チャンネルを初期化（note 1〜12 を割り当て、全て OFF）
    for (int i = 0; i < CHROMATIC_NOTES; ++i) {
        channels_[i].note   = i + 1;  // 1-indexed
        channels_[i].active = false;
    }
}

// ─── コールバック登録 ─────────────────────────

void AudioEngine::setNoteOnCallback(std::function<void(int)> cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    noteOnCallback_ = std::move(cb);
}

void AudioEngine::setNoteOffCallback(std::function<void(int)> cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    noteOffCallback_ = std::move(cb);
}

// ─── コードセット（差分計算 + ON/OFF 更新）──────
//
// Max/MSP の switch.cpp (RNBO listfilter) と同じロジック:
//   - in1 = 現在鳴っているノート (activeTriad_)
//   - in2 = 新しいコードのノート (triad)
//   - in1 に含まれるが in2 に含まれないもの → noteOff
//   - in2 に含まれるが in1 に含まれないもの → noteOn
//   - 両方に含まれるもの → そのまま（再トリガーしない）

void AudioEngine::setChord(const std::array<int, 3>& triad) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 現在のトライアドにあって新しいトライアドにないノートをOFF
    for (int prev : activeTriad_) {
        if (prev == 0) continue;
        bool stillActive = (triad[0] == prev || triad[1] == prev || triad[2] == prev);
        if (!stillActive) {
            noteOff(prev);
        }
    }

    // 新しいトライアドにあって現在のトライアドにないノートをON
    for (int next : triad) {
        if (next == 0) continue;
        bool alreadyOn = (activeTriad_[0] == next ||
                          activeTriad_[1] == next ||
                          activeTriad_[2] == next);
        if (!alreadyOn) {
            noteOn(next);
        }
    }

    activeTriad_ = triad;
}

// ─── ノート ON ────────────────────────────────

void AudioEngine::noteOn(int note) {
    if (note < 1 || note > CHROMATIC_NOTES) return;

    VoiceChannel& ch = channels_[note - 1];
    ch.active = true;

    // コンソール出力（動作確認用）
    std::cout << "[AudioEngine] NOTE ON  : " << noteName(note)
              << " (note=" << note << ")\n";

    // === 将来の拡張エリア ===========================
    // ch.amplitude = 0.0f;           // エンベロープのアタック開始
    // ch.phase     = 0.0f;           // オシレータ位相リセット
    // ch.frequency = noteToFreq(note);
    // oscPool_.activate(ch);
    // ===================================================

    if (noteOnCallback_) {
        noteOnCallback_(note);
    }
}

// ─── ノート OFF ───────────────────────────────

void AudioEngine::noteOff(int note) {
    if (note < 1 || note > CHROMATIC_NOTES) return;

    VoiceChannel& ch = channels_[note - 1];
    ch.active = false;

    std::cout << "[AudioEngine] NOTE OFF : " << noteName(note)
              << " (note=" << note << ")\n";

    // === 将来の拡張エリア ===========================
    // ch.amplitude = 1.0f;           // エンベロープのリリース開始
    // oscPool_.scheduleRelease(ch);
    // ===================================================

    if (noteOffCallback_) {
        noteOffCallback_(note);
    }
}

// ─── 状態取得（スレッドセーフ）────────────────

std::array<VoiceChannel, AudioEngine::CHROMATIC_NOTES>
AudioEngine::getChannelStates() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return channels_;
}

std::vector<int> AudioEngine::getActiveNotes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> active;
    active.reserve(3);
    for (const auto& ch : channels_) {
        if (ch.active) active.push_back(ch.note);
    }
    return active;
}
