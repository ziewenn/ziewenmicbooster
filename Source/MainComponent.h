#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "AudioEngine.h"
#include "UpdateChecker.h"

class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void loadPluginClicked();
    void drawCard(juce::Graphics& g, juce::Rectangle<int> bounds, float cornerRadius = 12.0f);
    void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level, juce::Colour color);
    
    void saveSettings();
    void loadSettings();
    std::unique_ptr<juce::PropertiesFile> getPropertiesFile();
    
    bool isStartupEnabled();
    void setStartupEnabled(bool enabled);
    
    AudioEngine audioEngine;
    UpdateChecker updateChecker;
    
    // Header
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label versionLabel;
    
    // Update banner
    bool updateAvailable = false;
    juce::String updateVersion;
    juce::String updateDownloadUrl;
    juce::TextButton updateButton;
    juce::Label updateLabel;
    bool isDownloading = false;
    
    // Device selection
    juce::Label inputLabel, outputLabel;
    juce::ComboBox inputDeviceCombo, outputDeviceCombo;
    
    // Boost
    juce::Slider boostSlider;
    juce::Label boostLabel;
    juce::Label boostValueLabel;
    
    // EQ controls
    juce::Label eqLabel;
    juce::Slider bassSlider, midSlider, trebleSlider;
    juce::Label bassLabel, midLabel, trebleLabel;
    juce::Label bassValueLabel, midValueLabel, trebleValueLabel;
    
    // Plugin
    juce::TextButton loadPluginButton;
    juce::TextButton removePluginButton;
    juce::Label pluginLabel;
    juce::Label pluginStatusLabel;
    
    // Settings
    juce::ToggleButton startupToggle;
    
    // Meters
    float smoothedInputLevel = 0.0f;
    float smoothedOutputLevel = 0.0f;
    
    // Colors
    juce::Colour bgGradientTop    { 0xff0f0f1a };
    juce::Colour bgGradientBottom { 0xff1a1a2e };
    juce::Colour cardColor        { 0xff16213e };
    juce::Colour cardBorderColor  { 0xff1f3460 };
    juce::Colour accentColor      { 0xff00d2ff };
    juce::Colour accentAlt        { 0xff7b2ff7 };
    juce::Colour successColor     { 0xff00e676 };
    juce::Colour warningColor     { 0xffffab40 };
    juce::Colour errorColor       { 0xffff5252 };
    juce::Colour textPrimary      { 0xffe0e0e0 };
    juce::Colour textSecondary    { 0xff8892b0 };
    juce::Colour surfaceColor     { 0xff0d1b2a };
    juce::Colour bassColor        { 0xffff6b6b };
    juce::Colour midColor         { 0xffffd93d };
    juce::Colour trebleColor      { 0xff6bcb77 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
