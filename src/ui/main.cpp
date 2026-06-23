#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#include "progression.h"
#include "app_ui.h"
#include "audio_engine.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")
#endif

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) return 1;

    // macOS: OpenGL 3.2 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 800,
                                          "Chord Settings", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ── DPI スケール取得（フォントサイズの物理ピクセル計算に使用）──
    // contentScale: 1.0 = 96 DPI, 2.0 = Retina / 192 DPI, 1.25 = Windows 125% など
    float xscale = 1.0f;
    glfwGetWindowContentScale(window, &xscale, nullptr);
    // 目標の論理サイズ 20px × DPI倍率 = 物理ピクセルでのロードサイズ
    // FontGlobalScale による後処理スケーリングを廃止し、ロード時に正確なサイズを指定する
    const float fontSize = floorf(10.0f * xscale);

    // ── ImGui 初期化 ──
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // ── フォント読み込み（OS 自動判定）──
    // 全 OS 共通のグリフ範囲定義
    static const ImWchar latin_ranges[]  = { 0x0020, 0x00FF, 0 };
    static const ImWchar symbol_ranges[] = { 0x2600, 0x26FF, 0 };  // ♭ (U+266D) を含む

    bool fontLoaded = false;

#ifdef __APPLE__
    // macOS: Helvetica（ラテン）+ Apple Symbols（♭ 等）をマージ
    if (io.Fonts->AddFontFromFileTTF(
            "/System/Library/Fonts/Helvetica.ttc", fontSize, nullptr, latin_ranges)) {
        ImFontConfig merge_cfg;
        merge_cfg.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(
            "/System/Library/Fonts/Apple Symbols.ttf", fontSize, &merge_cfg, symbol_ranges);
        fontLoaded = true;
    }

#elif defined(_WIN32)
    // Windows: Segoe UI（ラテン）+ Segoe UI Symbol（♭ 等）をマージ
    // Segoe UI だけでは ♭ (U+266D) が含まれないため、必ず Symbol フォントとセットで使う
    {
        char winDir[MAX_PATH] = {};
        GetWindowsDirectoryA(winDir, MAX_PATH);
        const std::string fontsDir = std::string(winDir) + "\\Fonts\\";

        // ラテン文字フォント（優先順）
        const char* latinCandidates[] = { "segoeui.ttf", "YuGothM.ttc", "msgothic.ttc" };
        for (auto name : latinCandidates) {
            const std::string path = fontsDir + name;
            if (io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize, nullptr, latin_ranges)) {
                fontLoaded = true;
                break;
            }
        }

        if (fontLoaded) {
            // 音楽記号フォント（♭ = U+266D を確実に含む Segoe UI Symbol）
            ImFontConfig merge_cfg;
            merge_cfg.MergeMode = true;
            const char* symCandidates[] = { "seguisym.ttf", "seguiemj.ttf" };
            for (auto name : symCandidates) {
                const std::string path = fontsDir + name;
                if (io.Fonts->AddFontFromFileTTF(
                        path.c_str(), fontSize, &merge_cfg, symbol_ranges)) {
                    break;
                }
            }
        }
    }

#else
    // Linux: DejaVu Sans → Noto Sans（どちらも ♭ を含む）
    {
        const char* candidates[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
            "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        };
        static const ImWchar linux_ranges[] = {
            0x0020, 0x00FF,
            0x2600, 0x26FF,
            0,
        };
        for (auto path : candidates) {
            if (io.Fonts->AddFontFromFileTTF(path, fontSize, nullptr, linux_ranges)) {
                fontLoaded = true;
                break;
            }
        }
    }
#endif

    if (!fontLoaded) {
        // フォントが見つからない場合は ImGui 内蔵フォントにフォールバック
        io.Fonts->AddFontDefault();
        std::cerr << "[warn] System font not found; using ImGui default font.\n";
    }

    // FontGlobalScale は廃止（ロード時に正確なサイズを渡すため後処理スケーリング不要）
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding   = 4.f;
    style.FrameRounding    = 4.f;
    style.GrabRounding     = 4.f;
    style.WindowBorderSize = 0.f;
    style.ItemSpacing      = ImVec2(8.f, 8.f);
    style.Colors[ImGuiCol_WindowBg]  = ImVec4(0.f, 0.f, 0.f, 1.f);
    style.Colors[ImGuiCol_ChildBg]   = ImVec4(0.f, 0.f, 0.f, 1.f);
    style.Colors[ImGuiCol_PopupBg]   = ImVec4(0.05f, 0.05f, 0.05f, 1.f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif

    // ── 設定ファイルパス（OS 別に標準的な場所を使用）──
    const std::string settingsPath = []() -> std::string {
#ifdef _WIN32
        // Windows: %APPDATA%\chord_settings.txt
        const char* appdata = std::getenv("APPDATA");
        if (appdata) return std::string(appdata) + "\\chord_settings.txt";
#else
        // macOS / Linux: ~/.chord_settings.txt
        const char* home = std::getenv("HOME");
        if (home) return std::string(home) + "/.chord_settings.txt";
#endif
        return "chord_settings.txt";
    }();

    // ImGui のウィンドウ配置を設定ファイルと同じ場所に保存
    // （実行ディレクトリに imgui.ini が散らばらないようにする）
    static std::string imguiIniPath = []() -> std::string {
#ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata) return std::string(appdata) + "\\chord_settings_imgui.ini";
#else
        const char* home = std::getenv("HOME");
        if (home) return std::string(home) + "/.chord_settings_imgui.ini";
#endif
        return "chord_settings_imgui.ini";
    }();
    io.IniFilename = imguiIniPath.c_str();

    // ── エンジン・UI・音声 ──
    MarkovEngine engine;
    AppUI        ui;
    AudioEngine  audio;

    // 保存済み設定を読み込む（ファイルがなければデフォルト値のまま）
    engine.loadSettings(settingsPath);

    // ── 生成スレッド（元の無限ループに相当） ──
    std::thread genThread([&]() {
        while (engine.running) {
            if (engine.isPaused()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::string chord = engine.currentChordName();
            double wait = engine.getWaitSeconds();
            engine.addToHistory(chord, wait);

            // コード情報取得 + 音声モジュールへ送信
            {
                auto t = engine.getState().triad;

                // コンソール出力: コード名 + 数値3つ [a, b, c]
                std::cout << chord
                          << "  ->  [" << t[0] << ", " << t[1] << ", " << t[2] << "]"
                          << "\n(next in " << wait << " sec)\n" << std::endl;

                // 音声モジュールへコード番号を渡す（ON/OFF 差分を自動計算）
                audio.setChord(t);
            }

            int waitMs = static_cast<int>(wait * 1000.0);
            for (int elapsed = 0; elapsed < waitMs && engine.running; elapsed += 50) {
                if (engine.isPaused()) break;
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(std::min(50, waitMs - elapsed)));
            }

            if (engine.running && !engine.isPaused()) {
                engine.step();
            }
        }
    });

    // ── メインループ ──
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui.render(engine);

        ImGui::Render();
        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.06f, 0.07f, 0.09f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ── 終了処理 ──
    engine.running = false;
    genThread.join();

    // 現在の設定を保存
    engine.saveSettings(settingsPath);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
