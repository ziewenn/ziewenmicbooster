#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

#ifdef _WIN32
#include <windows.h>
#endif

MainComponent::MainComponent()
{
    setSize(540, 750);
    
    // Header
    titleLabel.setText("Mic Booster", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(30.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, accentColor);
    addAndMakeVisible(titleLabel);
    
    subtitleLabel.setText("EQ + Plugin Chain", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(12.0f));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(subtitleLabel);
    
    versionLabel.setText(juce::String("v") + UpdateChecker::CURRENT_VERSION, juce::dontSendNotification);
    versionLabel.setFont(juce::Font(9.0f));
    versionLabel.setJustificationType(juce::Justification::centredRight);
    versionLabel.setColour(juce::Label::textColourId, textSecondary.withAlpha(0.5f));
    addAndMakeVisible(versionLabel);
    
    // Update banner (hidden initially)
    updateLabel.setText("", juce::dontSendNotification);
    updateLabel.setFont(juce::Font(11.0f));
    updateLabel.setColour(juce::Label::textColourId, successColor);
    addChildComponent(updateLabel);
    
    updateButton.setButtonText("Update Now");
    updateButton.setColour(juce::TextButton::buttonColourId, successColor.withAlpha(0.2f));
    updateButton.setColour(juce::TextButton::textColourOffId, successColor);
    updateButton.onClick = [this] {
        if (!isDownloading && updateDownloadUrl.isNotEmpty())
        {
            isDownloading = true;
            updateButton.setButtonText("Downloading...");
            updateButton.setEnabled(false);
            
            std::thread([this]() {
                updateChecker.downloadUpdate(updateDownloadUrl);
            }).detach();
        }
    };
    addChildComponent(updateButton);
    
    // Input Device
    inputLabel.setText("INPUT DEVICE", juce::dontSendNotification);
    inputLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    inputLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(inputLabel);
    
    inputDeviceCombo.setColour(juce::ComboBox::backgroundColourId, surfaceColor);
    inputDeviceCombo.setColour(juce::ComboBox::outlineColourId, cardBorderColor);
    inputDeviceCombo.setColour(juce::ComboBox::textColourId, textPrimary);
    inputDeviceCombo.setColour(juce::ComboBox::arrowColourId, accentColor);
    inputDeviceCombo.onChange = [this] {
        audioEngine.setInputDevice(inputDeviceCombo.getText());
        saveSettings();
    };
    addAndMakeVisible(inputDeviceCombo);
    
    // Output Device
    outputLabel.setText("OUTPUT DEVICE", juce::dontSendNotification);
    outputLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    outputLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(outputLabel);
    
    outputDeviceCombo.setColour(juce::ComboBox::backgroundColourId, surfaceColor);
    outputDeviceCombo.setColour(juce::ComboBox::outlineColourId, cardBorderColor);
    outputDeviceCombo.setColour(juce::ComboBox::textColourId, textPrimary);
    outputDeviceCombo.setColour(juce::ComboBox::arrowColourId, accentColor);
    outputDeviceCombo.onChange = [this] {
        audioEngine.setOutputDevice(outputDeviceCombo.getText());
        saveSettings();
    };
    addAndMakeVisible(outputDeviceCombo);
    
    // Boost
    boostLabel.setText("BOOST GAIN", juce::dontSendNotification);
    boostLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    boostLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(boostLabel);
    
    boostSlider.setRange(-20.0, 40.0, 0.1);
    boostSlider.setValue(0.0);
    boostSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    boostSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    boostSlider.setColour(juce::Slider::trackColourId, accentColor);
    boostSlider.setColour(juce::Slider::backgroundColourId, surfaceColor);
    boostSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    boostSlider.onValueChange = [this] {
        auto val = boostSlider.getValue();
        audioEngine.setBoostGain(static_cast<float>(val));
        juce::String sign = val >= 0.0 ? "+" : "";
        boostValueLabel.setText(sign + juce::String(val, 1) + " dB", juce::dontSendNotification);
        
        if (val > 30.0)
            boostValueLabel.setColour(juce::Label::textColourId, errorColor);
        else if (val > 20.0)
            boostValueLabel.setColour(juce::Label::textColourId, warningColor);
        else
            boostValueLabel.setColour(juce::Label::textColourId, accentColor);
        
        saveSettings();
    };
    addAndMakeVisible(boostSlider);
    
    boostValueLabel.setText("+0.0 dB", juce::dontSendNotification);
    boostValueLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    boostValueLabel.setJustificationType(juce::Justification::centred);
    boostValueLabel.setColour(juce::Label::textColourId, accentColor);
    addAndMakeVisible(boostValueLabel);
    
    // EQ Section
    eqLabel.setText("TONE ADJUSTMENTS", juce::dontSendNotification);
    eqLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    eqLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(eqLabel);
    
    // Bass
    bassLabel.setText("Bass", juce::dontSendNotification);
    bassLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    bassLabel.setColour(juce::Label::textColourId, bassColor);
    addAndMakeVisible(bassLabel);
    
    bassSlider.setRange(-12.0, 12.0, 0.1);
    bassSlider.setValue(0.0);
    bassSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bassSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bassSlider.setColour(juce::Slider::trackColourId, bassColor);
    bassSlider.setColour(juce::Slider::backgroundColourId, surfaceColor);
    bassSlider.setColour(juce::Slider::thumbColourId, bassColor);
    bassSlider.onValueChange = [this] {
        auto val = bassSlider.getValue();
        audioEngine.setBassGain(static_cast<float>(val));
        juce::String sign = val >= 0.0 ? "+" : "";
        bassValueLabel.setText(sign + juce::String(val, 1) + " dB", juce::dontSendNotification);
        saveSettings();
    };
    addAndMakeVisible(bassSlider);
    
    bassValueLabel.setText("+0.0 dB", juce::dontSendNotification);
    bassValueLabel.setFont(juce::Font(10.0f));
    bassValueLabel.setJustificationType(juce::Justification::centredRight);
    bassValueLabel.setColour(juce::Label::textColourId, bassColor.withAlpha(0.8f));
    addAndMakeVisible(bassValueLabel);
    
    // Mid
    midLabel.setText("Mid", juce::dontSendNotification);
    midLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    midLabel.setColour(juce::Label::textColourId, midColor);
    addAndMakeVisible(midLabel);
    
    midSlider.setRange(-12.0, 12.0, 0.1);
    midSlider.setValue(0.0);
    midSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    midSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    midSlider.setColour(juce::Slider::trackColourId, midColor);
    midSlider.setColour(juce::Slider::backgroundColourId, surfaceColor);
    midSlider.setColour(juce::Slider::thumbColourId, midColor);
    midSlider.onValueChange = [this] {
        auto val = midSlider.getValue();
        audioEngine.setMidGain(static_cast<float>(val));
        juce::String sign = val >= 0.0 ? "+" : "";
        midValueLabel.setText(sign + juce::String(val, 1) + " dB", juce::dontSendNotification);
        saveSettings();
    };
    addAndMakeVisible(midSlider);
    
    midValueLabel.setText("+0.0 dB", juce::dontSendNotification);
    midValueLabel.setFont(juce::Font(10.0f));
    midValueLabel.setJustificationType(juce::Justification::centredRight);
    midValueLabel.setColour(juce::Label::textColourId, midColor.withAlpha(0.8f));
    addAndMakeVisible(midValueLabel);
    
    // Treble
    trebleLabel.setText("Treble", juce::dontSendNotification);
    trebleLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    trebleLabel.setColour(juce::Label::textColourId, trebleColor);
    addAndMakeVisible(trebleLabel);
    
    trebleSlider.setRange(-12.0, 12.0, 0.1);
    trebleSlider.setValue(0.0);
    trebleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    trebleSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    trebleSlider.setColour(juce::Slider::trackColourId, trebleColor);
    trebleSlider.setColour(juce::Slider::backgroundColourId, surfaceColor);
    trebleSlider.setColour(juce::Slider::thumbColourId, trebleColor);
    trebleSlider.onValueChange = [this] {
        auto val = trebleSlider.getValue();
        audioEngine.setTrebleGain(static_cast<float>(val));
        juce::String sign = val >= 0.0 ? "+" : "";
        trebleValueLabel.setText(sign + juce::String(val, 1) + " dB", juce::dontSendNotification);
        saveSettings();
    };
    addAndMakeVisible(trebleSlider);
    
    trebleValueLabel.setText("+0.0 dB", juce::dontSendNotification);
    trebleValueLabel.setFont(juce::Font(10.0f));
    trebleValueLabel.setJustificationType(juce::Justification::centredRight);
    trebleValueLabel.setColour(juce::Label::textColourId, trebleColor.withAlpha(0.8f));
    addAndMakeVisible(trebleValueLabel);
    
    // Plugin
    pluginLabel.setText("VST3 PLUGIN", juce::dontSendNotification);
    pluginLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    pluginLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(pluginLabel);
    
    loadPluginButton.setButtonText("Load Plugin");
    loadPluginButton.setColour(juce::TextButton::buttonColourId, accentColor.withAlpha(0.15f));
    loadPluginButton.setColour(juce::TextButton::buttonOnColourId, accentColor.withAlpha(0.3f));
    loadPluginButton.setColour(juce::TextButton::textColourOffId, accentColor);
    loadPluginButton.onClick = [this] { loadPluginClicked(); };
    addAndMakeVisible(loadPluginButton);
    
    removePluginButton.setButtonText("Remove");
    removePluginButton.setColour(juce::TextButton::buttonColourId, surfaceColor);
    removePluginButton.setColour(juce::TextButton::textColourOffId, textSecondary);
    removePluginButton.onClick = [this] {
        audioEngine.removePlugin();
        pluginStatusLabel.setText("No plugin loaded", juce::dontSendNotification);
        pluginStatusLabel.setColour(juce::Label::textColourId, textSecondary);
    };
    addAndMakeVisible(removePluginButton);
    
    pluginStatusLabel.setText("No plugin loaded", juce::dontSendNotification);
    pluginStatusLabel.setFont(juce::Font(11.0f));
    pluginStatusLabel.setColour(juce::Label::textColourId, textSecondary);
    addAndMakeVisible(pluginStatusLabel);
    
    // Startup Toggle
    startupToggle.setButtonText("Launch on system startup");
    startupToggle.setColour(juce::ToggleButton::textColourId, textSecondary);
    startupToggle.setColour(juce::ToggleButton::tickColourId, accentColor);
    startupToggle.setToggleState(isStartupEnabled(), juce::dontSendNotification);
    startupToggle.onClick = [this] {
        setStartupEnabled(startupToggle.getToggleState());
    };
    addAndMakeVisible(startupToggle);
    
    // Initialize
    audioEngine.initialize();
    
    auto inputs = audioEngine.getAvailableInputDevices();
    for (int i = 0; i < inputs.size(); ++i)
        inputDeviceCombo.addItem(inputs[i], i + 1);
    
    auto outputs = audioEngine.getAvailableOutputDevices();
    for (int i = 0; i < outputs.size(); ++i)
        outputDeviceCombo.addItem(outputs[i], i + 1);
    
    loadSettings();
    
    // Check for updates
    updateChecker.onUpdateFound = [this](const UpdateChecker::UpdateInfo& info) {
        updateAvailable = true;
        updateVersion = info.version;
        updateDownloadUrl = info.downloadUrl;
        updateLabel.setText("Update v" + info.version + " available!", juce::dontSendNotification);
        updateLabel.setVisible(true);
        updateButton.setVisible(info.downloadUrl.isNotEmpty());
        resized();
    };
    updateChecker.checkForUpdates();
    
    startTimer(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    saveSettings();
}

std::unique_ptr<juce::PropertiesFile> MainComponent::getPropertiesFile()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "MicBooster";
    options.folderName = "MicBooster";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    return std::make_unique<juce::PropertiesFile>(options);
}

void MainComponent::saveSettings()
{
    auto props = getPropertiesFile();
    if (props == nullptr) return;
    
    props->setValue("inputDevice", inputDeviceCombo.getText());
    props->setValue("outputDevice", outputDeviceCombo.getText());
    props->setValue("boostGain", boostSlider.getValue());
    props->setValue("bassGain", bassSlider.getValue());
    props->setValue("midGain", midSlider.getValue());
    props->setValue("trebleGain", trebleSlider.getValue());
    props->saveIfNeeded();
}

void MainComponent::loadSettings()
{
    auto props = getPropertiesFile();
    bool hasSettings = (props != nullptr && props->containsKey("inputDevice"));
    
    if (hasSettings)
    {
        auto savedInput = props->getValue("inputDevice");
        for (int i = 0; i < inputDeviceCombo.getNumItems(); ++i)
        {
            if (inputDeviceCombo.getItemText(i) == savedInput)
            {
                inputDeviceCombo.setSelectedItemIndex(i, juce::sendNotification);
                break;
            }
        }
        
        auto savedOutput = props->getValue("outputDevice");
        for (int i = 0; i < outputDeviceCombo.getNumItems(); ++i)
        {
            if (outputDeviceCombo.getItemText(i) == savedOutput)
            {
                outputDeviceCombo.setSelectedItemIndex(i, juce::sendNotification);
                break;
            }
        }
        
        boostSlider.setValue(props->getDoubleValue("boostGain", 0.0), juce::sendNotification);
        bassSlider.setValue(props->getDoubleValue("bassGain", 0.0), juce::sendNotification);
        midSlider.setValue(props->getDoubleValue("midGain", 0.0), juce::sendNotification);
        trebleSlider.setValue(props->getDoubleValue("trebleGain", 0.0), juce::sendNotification);
    }
    else
    {
        if (inputDeviceCombo.getNumItems() > 0)
            inputDeviceCombo.setSelectedId(1, juce::sendNotification);
        if (outputDeviceCombo.getNumItems() > 0)
            outputDeviceCombo.setSelectedId(1, juce::sendNotification);
    }
}

bool MainComponent::isStartupEnabled()
{
#ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS)
        return false;
    
    DWORD type;
    result = RegQueryValueExW(hKey, L"MicBooster", nullptr, &type, nullptr, nullptr);
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
#else
    return false;
#endif
}

void MainComponent::setStartupEnabled(bool enabled)
{
#ifdef _WIN32
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);
    
    if (result != ERROR_SUCCESS)
        return;
    
    if (enabled)
    {
        auto exePath = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName();
        auto exePathW = exePath.toWideCharPointer();
        RegSetValueExW(hKey, L"MicBooster", 0, REG_SZ,
            (const BYTE*)exePathW,
            (DWORD)((wcslen(exePathW) + 1) * sizeof(wchar_t)));
    }
    else
    {
        RegDeleteValueW(hKey, L"MicBooster");
    }
    
    RegCloseKey(hKey);
#else
    juce::ignoreUnused(enabled);
#endif
}

void MainComponent::drawCard(juce::Graphics& g, juce::Rectangle<int> bounds, float cornerRadius)
{
    auto fb = bounds.toFloat();
    g.setColour(cardColor);
    g.fillRoundedRectangle(fb, cornerRadius);
    g.setColour(cardBorderColor.withAlpha(0.4f));
    g.drawRoundedRectangle(fb, cornerRadius, 1.0f);
}

void MainComponent::drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level, juce::Colour color)
{
    auto fb = bounds.toFloat();
    g.setColour(surfaceColor);
    g.fillRoundedRectangle(fb, 4.0f);
    
    float clampedLevel = juce::jlimit(0.0f, 1.0f, level);
    auto fillWidth = fb.getWidth() * clampedLevel;
    if (fillWidth > 2.0f)
    {
        auto fillBounds = fb.withWidth(fillWidth);
        juce::ColourGradient gradient(color.withAlpha(0.6f), fb.getX(), fb.getCentreY(),
                                       color, fb.getX() + fillWidth, fb.getCentreY(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(fillBounds, 4.0f);
    }
}

void MainComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    juce::ColourGradient bgGradient(bgGradientTop, 0, 0, bgGradientBottom, 0, (float)bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(bounds);
    
    auto glowArea = bounds.toFloat().removeFromRight(250).removeFromTop(200);
    juce::ColourGradient glow(accentColor.withAlpha(0.04f), glowArea.getCentreX(), glowArea.getCentreY(),
                               accentColor.withAlpha(0.0f), glowArea.getX(), glowArea.getBottom(), true);
    g.setGradientFill(glow);
    g.fillEllipse(glowArea);
    
    auto glow2Area = bounds.toFloat().removeFromLeft(200).removeFromBottom(200);
    juce::ColourGradient glow2(accentAlt.withAlpha(0.03f), glow2Area.getCentreX(), glow2Area.getCentreY(),
                                accentAlt.withAlpha(0.0f), glow2Area.getRight(), glow2Area.getY(), true);
    g.setGradientFill(glow2);
    g.fillEllipse(glow2Area);
    
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(60);
    
    // Update banner
    if (updateAvailable)
    {
        auto updateBounds = area.removeFromTop(36);
        g.setColour(successColor.withAlpha(0.08f));
        g.fillRoundedRectangle(updateBounds.toFloat(), 8.0f);
        g.setColour(successColor.withAlpha(0.3f));
        g.drawRoundedRectangle(updateBounds.toFloat(), 8.0f, 1.0f);
        area.removeFromTop(8);
    }
    
    drawCard(g, area.removeFromTop(110));
    area.removeFromTop(8);
    
    auto boostCardBounds = area.removeFromTop(110);
    drawCard(g, boostCardBounds);
    
    auto meterArea = boostCardBounds.reduced(14, 0);
    meterArea.removeFromTop(86);
    auto meterRow = meterArea.removeFromTop(12);
    auto inMeter = meterRow.removeFromLeft(meterRow.getWidth() / 2 - 4);
    meterRow.removeFromLeft(8);
    auto outMeter = meterRow;
    
    g.setColour(textSecondary.withAlpha(0.6f));
    g.setFont(juce::Font(8.0f));
    g.drawText("IN", inMeter.removeFromLeft(16), juce::Justification::centredLeft);
    drawMeter(g, inMeter, smoothedInputLevel, accentColor);
    g.drawText("OUT", outMeter.removeFromLeft(20), juce::Justification::centredLeft);
    drawMeter(g, outMeter, smoothedOutputLevel, successColor);
    
    area.removeFromTop(8);
    drawCard(g, area.removeFromTop(120));
    area.removeFromTop(8);
    drawCard(g, area.removeFromTop(98));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    titleLabel.setBounds(area.removeFromTop(38));
    auto subRow = area.removeFromTop(18);
    subtitleLabel.setBounds(subRow);
    versionLabel.setBounds(subRow);
    area.removeFromTop(4);
    
    // Update banner
    if (updateAvailable)
    {
        auto updateBounds = area.removeFromTop(36).reduced(10, 6);
        updateLabel.setBounds(updateBounds.removeFromLeft(updateBounds.getWidth() - 110));
        updateBounds.removeFromLeft(6);
        updateButton.setBounds(updateBounds);
        area.removeFromTop(8);
    }
    
    // Devices card
    auto devCard = area.removeFromTop(110);
    auto devInner = devCard.reduced(14, 10);
    auto inputRow = devInner.removeFromTop(44);
    inputLabel.setBounds(inputRow.removeFromTop(16));
    inputRow.removeFromTop(2);
    inputDeviceCombo.setBounds(inputRow.removeFromTop(26));
    devInner.removeFromTop(4);
    auto outputRow = devInner.removeFromTop(44);
    outputLabel.setBounds(outputRow.removeFromTop(16));
    outputRow.removeFromTop(2);
    outputDeviceCombo.setBounds(outputRow.removeFromTop(26));
    area.removeFromTop(8);
    
    // Boost card
    auto boostCard = area.removeFromTop(110);
    auto boostInner = boostCard.reduced(14, 10);
    boostLabel.setBounds(boostInner.removeFromTop(16));
    boostInner.removeFromTop(2);
    boostValueLabel.setBounds(boostInner.removeFromTop(30));
    boostInner.removeFromTop(2);
    boostSlider.setBounds(boostInner.removeFromTop(24));
    area.removeFromTop(8);
    
    // EQ card
    auto eqCard = area.removeFromTop(120);
    auto eqInner = eqCard.reduced(14, 8);
    eqLabel.setBounds(eqInner.removeFromTop(16));
    eqInner.removeFromTop(6);
    
    auto eqRow = [&](juce::Label& label, juce::Slider& slider, juce::Label& valueLabel) {
        auto row = eqInner.removeFromTop(26);
        label.setBounds(row.removeFromLeft(50));
        valueLabel.setBounds(row.removeFromRight(55));
        row.removeFromLeft(4);
        row.removeFromRight(4);
        slider.setBounds(row);
        eqInner.removeFromTop(2);
    };
    
    eqRow(bassLabel, bassSlider, bassValueLabel);
    eqRow(midLabel, midSlider, midValueLabel);
    eqRow(trebleLabel, trebleSlider, trebleValueLabel);
    area.removeFromTop(8);
    
    // Plugin card
    auto plugCard = area.removeFromTop(98);
    auto plugInner = plugCard.reduced(14, 10);
    pluginLabel.setBounds(plugInner.removeFromTop(16));
    plugInner.removeFromTop(6);
    auto buttonRow = plugInner.removeFromTop(28);
    loadPluginButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() / 2 - 4));
    buttonRow.removeFromLeft(8);
    removePluginButton.setBounds(buttonRow);
    plugInner.removeFromTop(4);
    pluginStatusLabel.setBounds(plugInner.removeFromTop(20));
    area.removeFromTop(8);
    
    startupToggle.setBounds(area.removeFromTop(24));
}

void MainComponent::timerCallback()
{
    float targetIn = audioEngine.getCurrentInputLevel();
    float targetOut = audioEngine.getCurrentOutputLevel();
    
    smoothedInputLevel = smoothedInputLevel * 0.8f + targetIn * 0.2f;
    smoothedOutputLevel = smoothedOutputLevel * 0.8f + targetOut * 0.2f;
    
    repaint();
}

void MainComponent::loadPluginClicked()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select a VST3 plugin",
        juce::File::getSpecialLocation(juce::File::globalApplicationsDirectory),
        "*.vst3;*.dll");
    
    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync(flags, [this, chooser](const juce::FileChooser&)
    {
        auto file = chooser->getResult();
        if (file.existsAsFile())
        {
            audioEngine.loadPlugin(file);
            
            if (audioEngine.hasPluginLoaded())
            {
                pluginStatusLabel.setText("Loaded: " + audioEngine.getPluginName(), juce::dontSendNotification);
                pluginStatusLabel.setColour(juce::Label::textColourId, successColor);
            }
            else
            {
                pluginStatusLabel.setText("Failed to load plugin", juce::dontSendNotification);
                pluginStatusLabel.setColour(juce::Label::textColourId, errorColor);
            }
        }
    });
}
