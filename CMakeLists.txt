cmake_minimum_required(VERSION 3.22)
project(Chorder VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# JUCE の取得
# JUCE_PATH に手動でパスを渡すか、空のままにすると自動ダウンロード
# ============================================================
set(JUCE_PATH "" CACHE PATH "Path to local JUCE source (leave empty to auto-download)")

if(JUCE_PATH AND EXISTS "${JUCE_PATH}")
    message(STATUS "Using local JUCE: ${JUCE_PATH}")
    add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/juce")
else()
    message(STATUS "Downloading JUCE via FetchContent ...")
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()

# ============================================================
# rnbo_description.h の生成（description.json から）
# ============================================================
set(RNBO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rnbo")
list(APPEND CMAKE_MODULE_PATH "${RNBO_DIR}/cmake")
include(RNBODescriptionHeader)

set(DESC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${DESC_INCLUDE_DIR}")

rnbo_write_description_header_if_exists(
    "${CMAKE_CURRENT_SOURCE_DIR}/description.json"
    "${DESC_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/presets.json"
)

# ============================================================
# プラグインターゲット（VST3 / AU / Standalone）
# ============================================================
juce_add_plugin(Chorder
    PLUGIN_MANUFACTURER_CODE Cyc7
    PLUGIN_CODE     Chrd
    FORMATS         VST3 AU Standalone
    PRODUCT_NAME    "Chorder"
    COMPANY_NAME    "Cycling74"
    BUNDLE_ID       "com.cycling74.chorder"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             "Instrument|Synth"
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
)

target_sources(Chorder
    PRIVATE
        rnbo_source.cpp
        ${RNBO_DIR}/RNBO.cpp
        ${RNBO_DIR}/adapters/juce/RNBO_JuceAudioProcessor.cpp
        ${RNBO_DIR}/adapters/juce/RNBO_JuceAudioProcessorEditor.cpp
        ${RNBO_DIR}/adapters/juce/RNBO_JuceAudioProcessorUtils.cpp
)

target_include_directories(Chorder
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${RNBO_DIR}
        ${RNBO_DIR}/common
        ${RNBO_DIR}/src
        ${RNBO_DIR}/adapters/juce
        ${RNBO_DIR}/src/3rdparty/json
        ${RNBO_DIR}/src/3rdparty/concurrentqueue
        ${RNBO_DIR}/src/3rdparty/readerwriterqueue
        ${RNBO_DIR}/src/3rdparty/MPark_variant
        ${DESC_INCLUDE_DIR}
)

target_compile_definitions(Chorder
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(Chorder
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
