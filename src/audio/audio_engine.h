#pragma once

#include <array>
#include <vector>
#include <mutex>
#include <functional>

// ─── ノート番号の定義 ──────────────────────────
// クロマチックスケール 1始まり: C=1, C#=2, D=3, ..., B=12
// MarkovEngine の ChordState::toTriad() と同じ基準

// ─── ボイスチャンネル ─────────────────────────
// 12音それぞれの ON/OFF 状態を保持するユニット
// 将来: ここにオシレータ・エンベロープ・フィルタ等を追加予定

struct VoiceChannel {
    int  note   = 0;       // ノート番号 (1-12)。0 = 未割り当て
    bool active = false;   // 現在発音中かどうか

    // === 将来の拡張エリア ===========================
    // float amplitude = 0.0f;    // 現在の振幅（エンベロープ制御用）
    // float phase     = 0.0f;    // オシレータ位相
    // float frequency = 0.0f;    // 周波数 (Hz)
    // =================================================
};

// ─── 音声エンジン ─────────────────────────────
//
// 役割:
//   - Markov エンジンから受け取ったコード（3音）をもとに
//     各ボイスチャンネルの ON / OFF を管理する
//   - コード変化時に差分を計算し、不要な音をOFF・新しい音をONにする
//     （Max/MSP の switch.cpp の listfilter ロジックに相当）
//
// 拡張方針:
//   - 実際の音声合成は process() を実装することで追加可能
//   - VoiceChannel にオシレータ等を追加してポリフォニー音源に発展可能

class AudioEngine {
public:
    static constexpr int CHROMATIC_NOTES = 12;  // C〜Bの12音

    AudioEngine();
    ~AudioEngine() = default;

    // コードをセット（Markov エンジンから呼び出される）
    // triad: [root, 3rd, 5th] (1-12)
    // 前のコードとの差分を計算し ON/OFF を更新する
    void setChord(const std::array<int, 3>& triad);

    // 全ボイスチャンネルの現在状態を取得（スレッドセーフ）
    std::array<VoiceChannel, CHROMATIC_NOTES> getChannelStates() const;

    // 現在 ON になっているノート番号リストを取得（スレッドセーフ）
    std::vector<int> getActiveNotes() const;

    // ON/OFF イベントのコールバックを登録（外部通知用）
    // 将来: DSP スレッドや MIDI 出力への通知に利用可能
    void setNoteOnCallback (std::function<void(int note)> cb);
    void setNoteOffCallback(std::function<void(int note)> cb);

    // === 将来の拡張インターフェース ====================
    // void setSampleRate(double sr);
    // void process(float* outL, float* outR, int numSamples);
    // void setVolume(float vol);
    // ===================================================

private:
    mutable std::mutex mutex_;

    std::array<VoiceChannel, CHROMATIC_NOTES> channels_;
    std::array<int, 3> activeTriad_ = {0, 0, 0};  // 現在発音中のトライアド

    std::function<void(int)> noteOnCallback_;
    std::function<void(int)> noteOffCallback_;

    void noteOn (int noteNumber);   // note: 1-12
    void noteOff(int noteNumber);   // note: 1-12
};
