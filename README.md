# RASM + Melody C++ System

## 概要

RASM + Melody システムを C++ で実装した、マルコフ連鎖によるコード進行生成アプリケーションです。
ImGui による GUI を備え、遷移確率の編集・再生速度の切り替え・履歴表示をリアルタイムに行えます。

> GLFW と Dear ImGui は CMake の FetchContent で自動ダウンロードされるため、手動インストール不要です。

---

## プロジェクト構成

```
rasm-cpp/
├── src/
│   ├── CMakeLists.txt      ← ビルド設定（ここで cmake を実行）
│   ├── core/               ← マルコフエンジン（コード進行生成）
│   ├── audio/              ← 音声出力モジュール
│   └── ui/                 ← GUI・エントリポイント
└── README.md
```

ビルド成果物は `src/build/` に生成されます。

---

## セットアップ

> **重要:** ビルド・実行はすべて `src/` ディレクトリ内で行います。

### Windows

**必要なもの**

| 項目 | 入手先 |
|------|--------|
| Visual Studio 2019+ | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)（C++ によるデスクトップ開発 ワークロードを選択） |
| CMake | [cmake.org](https://cmake.org/download/)（インストーラー版を推奨） |
| Git | [git-scm.com](https://git-scm.com/) |

**ビルド手順（コマンドプロンプト / PowerShell）**

```bat
git clone https://github.com/kotaro-42/rasm-cpp.git
cd rasm-cpp\src
cmake -B build
cmake --build build --config Release
```

**実行**

```bat
build\Release\chord_settings.exe
```

**Visual Studio から開く場合**

1. Visual Studio で「ローカル フォルダーを開く」→ `src` フォルダを選択
2. CMake が自動認識される
3. ビルド → デバッグ実行

---

### Linux

**必要なもの（Ubuntu / Debian 系）**

```bash
sudo apt install build-essential cmake git libgl1-mesa-dev
```

**ビルド手順**

```bash
git clone https://github.com/kotaro-42/rasm-cpp.git
cd rasm-cpp/src
cmake -B build
cmake --build build -j$(nproc)
```

**実行**

`src/` ディレクトリにいる状態で:

```bash
./build/chord_settings
```

---

### macOS

**必要なもの**

```bash
xcode-select --install
brew install cmake
```

**ビルド手順**

```bash
git clone https://github.com/kotaro-42/rasm-cpp.git
cd rasm-cpp/src
cmake -B build
cmake --build build -j$(sysctl -n hw.logicalcpu)
```

**実行**

ビルド後、**`src/` ディレクトリにいる状態**で実行します。

```bash
./build/chord_settings
```

プロジェクトルート（`rasm-cpp/`）にいる場合:

```bash
./src/build/chord_settings
```

---

## 再ビルド（コード更新後）

リポジトリを更新したあと、再ビルドする場合:

```bash
cd src
git pull origin main
cmake --build build
./build/chord_settings          # macOS / Linux
# build\Release\chord_settings.exe     # Windows
```

CMakeLists.txt を変更した場合は、最初から `cmake -B build` を実行し直してください。

---

## Git の使い方

### 作業を始める前に

> [!IMPORTANT]
> 作業を開始する前に、必ず最新の変更を取り込んでください。

```bash
git pull origin main
```

### 変更内容を GitHub へ反映する

> [!TIP]
> 作業が完了し、問題なく動作することを確認したら、以下のコマンドで GitHub へ反映してください。

```bash
git status
git add .
git commit -m "変更内容を簡潔に記述"
git push origin main
```

> `develop` ブランチは存在しません。プッシュ先は `main` を使用してください。

---

## OS 別の動作の違い

| 項目 | Windows | Linux | macOS |
|------|---------|-------|-------|
| フォント | Segoe UI / Yu Gothic UI / MS Gothic | DejaVu Sans / Noto Sans | Helvetica + Apple Symbols |
| ♭ 記号 | Segoe UI Symbol | DejaVu Sans に含まれる | Apple Symbols |
| 設定保存先 | `%APPDATA%\chord_settings.txt` | `~/.chord_settings.txt` | `~/.chord_settings.txt` |
| GUI レイアウト保存先 | `%APPDATA%\chord_settings_imgui.ini` | `~/.chord_settings_imgui.ini` | `~/.chord_settings_imgui.ini` |
| GLSL バージョン | `#version 130` | `#version 130` | `#version 150` |

> フォントが見つからない場合は ImGui 内蔵フォントに自動フォールバックします（♭ が表示されない可能性があります）。
>
> 設定ファイル・GUI レイアウトはユーザーのホームディレクトリ（または APPDATA）に保存されるため、プロジェクトフォルダ内には生成されません。
