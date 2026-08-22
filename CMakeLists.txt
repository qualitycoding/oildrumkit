cmake_minimum_required (VERSION 3.22)
project (OilDrumKit VERSION 1.0.0)

#set (CMAKE_CXX_STANDARD 17)
#set (CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Point this to exactly where you extracted JUCE!
# Use forward slashes (/) even on Windows.
# Ensure JUCE does not try to build VST2 or pick up a VST2 SDK on this machine.
# We only want to build VST3 (and Standalone) for this project.
set(JUCE_BUILD_VST2 OFF CACHE BOOL "Disable building VST2 support in JUCE")
set(JUCE_VST2_SDK_PATH "" CACHE PATH "Clear any VST2 SDK path so JUCE won't enable VST2")

# Also add global compile definitions so JUCE's module sources see that we only
# want VST3. This helps avoid the VST2/VST3 parameter-automation conflict check
# which can be triggered when both are detected.
add_compile_definitions(JucePlugin_Build_VST3=1 JucePlugin_Build_VST2=0)
add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)
add_subdirectory("D:/JUCE" JUCE)

# --- Plugin target -----------------------------------------------------------
juce_add_plugin (OilDrumKit
    COMPANY_NAME "Galactic Headquarters For Space Cadets"
    BUNDLE_ID "org.GalacticHQ.OilDrumKit"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE GaHq
    PLUGIN_CODE             Oidk
    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "Oil Drum Kit")

target_sources (OilDrumKit PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions (OilDrumKit PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries (OilDrumKit PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
