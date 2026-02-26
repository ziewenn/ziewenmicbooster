cmake_minimum_required(VERSION 3.15)
project(MicBooster VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Static link MSVC runtime so the exe is self-contained
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# JUCE setup
add_subdirectory(JUCE)

juce_add_gui_app(MicBooster
    PRODUCT_NAME "Mic Booster"
    COMPANY_NAME "YourCompany"
    BUNDLE_ID "com.yourcompany.micbooster"
)

target_sources(MicBooster PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/AudioEngine.cpp
    Source/AudioEngine.h
    Source/UpdateChecker.h
)

target_compile_definitions(MicBooster PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_DIRECT2D=0
)

target_link_libraries(MicBooster PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

juce_generate_juce_header(MicBooster)
