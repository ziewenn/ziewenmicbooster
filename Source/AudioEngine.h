#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    AudioEngine();
    ~AudioEngine();
    
    void initialize();
    void shutdown();
    
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                         int numInputChannels,
                                         float* const* outputChannelData,
                                         int numOutputChannels,
                                         int numSamples,
                                         const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    
    void setBoostGain(float gainDb);
    void setInputDevice(const juce::String& deviceName);
    void setOutputDevice(const juce::String& deviceName);
    
    void setBassGain(float gainDb);
    void setMidGain(float gainDb);
    void setTrebleGain(float gainDb);
    
    void loadPlugin(const juce::File& pluginFile);
    void removePlugin();
    
    juce::StringArray getAvailableInputDevices();
    juce::StringArray getAvailableOutputDevices();
    float getCurrentBoostGain() const { return boostGainDb; }
    float getBassGain() const { return bassGainDb; }
    float getMidGain() const { return midGainDb; }
    float getTrebleGain() const { return trebleGainDb; }
    bool hasPluginLoaded() const { return pluginInstance != nullptr; }
    juce::String getPluginName() const;
    float getCurrentInputLevel() const { return inputLevel.load(); }
    float getCurrentOutputLevel() const { return outputLevel.load(); }
    
private:
    void updateEQFilters();
    
    juce::AudioDeviceManager deviceManager;
    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    juce::AudioPluginFormatManager pluginFormatManager;
    
    float boostGainDb = 0.0f;
    float currentGain = 1.0f;
    float bassGainDb = 0.0f;
    float midGainDb = 0.0f;
    float trebleGainDb = 0.0f;
    
    juce::dsp::IIR::Filter<float> bassFilterL, bassFilterR;
    juce::dsp::IIR::Filter<float> midFilterL, midFilterR;
    juce::dsp::IIR::Filter<float> trebleFilterL, trebleFilterR;
    
    double currentSampleRate = 44100.0;
    juce::AudioBuffer<float> pluginBuffer;
    juce::MidiBuffer midiBuffer;
    
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
