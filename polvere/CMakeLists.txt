cmake_minimum_required(VERSION 3.22)
project(POLVERE VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

# JUCE: usa una copia locale (-DJUCE_DIR=/path/JUCE) oppure la scarica in automatico
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} juce)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
    set(POLVERE_FORMATS AU VST3 Standalone)
    set(POLVERE_COPY TRUE)
else()
    set(POLVERE_FORMATS VST3 Standalone)
    set(POLVERE_COPY FALSE)
endif()

juce_add_plugin(Polvere
    COMPANY_NAME "Leandro Taibbi"
    BUNDLE_ID "com.leandrotaibbi.polvere"
    PLUGIN_MANUFACTURER_CODE Ltbb
    PLUGIN_CODE Plvr
    FORMATS ${POLVERE_FORMATS}
    PRODUCT_NAME "Polvere"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    VST3_CATEGORIES Instrument Synth
    COPY_PLUGIN_AFTER_BUILD ${POLVERE_COPY})

target_sources(Polvere PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Polvere PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Polvere
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Test offline (render audio + screenshot GUI): cmake -DPOLVERE_BUILD_TESTS=ON
option(POLVERE_BUILD_TESTS "Build offline render test" OFF)
if(POLVERE_BUILD_TESTS)
    juce_add_console_app(PolvereRenderTest PRODUCT_NAME "PolvereRenderTest")
    target_sources(PolvereRenderTest PRIVATE tests/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(PolvereRenderTest PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(PolvereRenderTest PRIVATE juce::juce_audio_utils juce::juce_audio_formats juce::juce_dsp
                          juce::juce_recommended_config_flags)
endif()
