#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
    juce::addDefaultFormatsToManager(pluginFormatManager);
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

void AudioEngine::initialize()
{
    deviceManager.initialiseWithDefaultDevices(1, 1);
    deviceManager.addAudioCallback(this);
}

void AudioEngine::shutdown()
{
    deviceManager.removeAudioCallback(this);
    deviceManager.closeAudioDevice();
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    currentSampleRate = device->getCurrentSampleRate();
    auto bufferSize = device->getCurrentBufferSizeSamples();
    
    pluginBuffer.setSize(2, bufferSize);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = (juce::uint32)bufferSize;
    spec.numChannels = 1;
    
    bassFilterL.prepare(spec);
    bassFilterR.prepare(spec);
    midFilterL.prepare(spec);
    midFilterR.prepare(spec);
    trebleFilterL.prepare(spec);
    trebleFilterR.prepare(spec);
    
    updateEQFilters();
    
    if (pluginInstance != nullptr)
        pluginInstance->prepareToPlay(currentSampleRate, bufferSize);
}

void AudioEngine::audioDeviceStopped()
{
    bassFilterL.reset();
    bassFilterR.reset();
    midFilterL.reset();
    midFilterR.reset();
    trebleFilterL.reset();
    trebleFilterR.reset();
    
    if (pluginInstance != nullptr)
        pluginInstance->releaseResources();
}

void AudioEngine::audioDeviceIOCallbackWithContext(
    const float* const* inputChannelData,
    int numInputChannels,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& context)
{
    for (int i = 0; i < numOutputChannels; ++i)
    {
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }
    
    if (numInputChannels == 0 || inputChannelData[0] == nullptr)
        return;
    
    for (int ch = 0; ch < juce::jmin(numInputChannels, pluginBuffer.getNumChannels()); ++ch)
    {
        juce::FloatVectorOperations::copy(pluginBuffer.getWritePointer(ch), 
                                         inputChannelData[ch], 
                                         numSamples);
    }
    
    if (numInputChannels == 1 && pluginBuffer.getNumChannels() > 1)
    {
        juce::FloatVectorOperations::copy(pluginBuffer.getWritePointer(1),
                                         pluginBuffer.getReadPointer(0),
                                         numSamples);
    }
    
    float inLevel = 0.0f;
    for (int ch = 0; ch < pluginBuffer.getNumChannels(); ++ch)
        inLevel = juce::jmax(inLevel, pluginBuffer.getMagnitude(ch, 0, numSamples));
    inputLevel.store(inLevel);
    
    for (int ch = 0; ch < pluginBuffer.getNumChannels(); ++ch)
    {
        juce::FloatVectorOperations::multiply(pluginBuffer.getWritePointer(ch), 
                                             currentGain, 
                                             numSamples);
    }
    
    if (pluginBuffer.getNumChannels() >= 1)
    {
        auto* dataL = pluginBuffer.getWritePointer(0);
        for (int i = 0; i < numSamples; ++i)
        {
            dataL[i] = bassFilterL.processSample(dataL[i]);
            dataL[i] = midFilterL.processSample(dataL[i]);
            dataL[i] = trebleFilterL.processSample(dataL[i]);
        }
    }
    if (pluginBuffer.getNumChannels() >= 2)
    {
        auto* dataR = pluginBuffer.getWritePointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            dataR[i] = bassFilterR.processSample(dataR[i]);
            dataR[i] = midFilterR.processSample(dataR[i]);
            dataR[i] = trebleFilterR.processSample(dataR[i]);
        }
    }
    
    if (pluginInstance != nullptr)
    {
        midiBuffer.clear();
        pluginInstance->processBlock(pluginBuffer, midiBuffer);
    }
    
    float outLevel = 0.0f;
    for (int ch = 0; ch < pluginBuffer.getNumChannels(); ++ch)
        outLevel = juce::jmax(outLevel, pluginBuffer.getMagnitude(ch, 0, numSamples));
    outputLevel.store(outLevel);
    
    for (int ch = 0; ch < juce::jmin(numOutputChannels, pluginBuffer.getNumChannels()); ++ch)
    {
        if (outputChannelData[ch] != nullptr)
        {
            juce::FloatVectorOperations::copy(outputChannelData[ch], 
                                             pluginBuffer.getReadPointer(ch), 
                                             numSamples);
        }
    }
}

void AudioEngine::setBoostGain(float gainDb)
{
    boostGainDb = gainDb;
    currentGain = juce::Decibels::decibelsToGain(gainDb);
}

void AudioEngine::setInputDevice(const juce::String& deviceName)
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.inputDeviceName = deviceName;
    deviceManager.setAudioDeviceSetup(setup, true);
}

void AudioEngine::setOutputDevice(const juce::String& deviceName)
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.outputDeviceName = deviceName;
    deviceManager.setAudioDeviceSetup(setup, true);
}

juce::StringArray AudioEngine::getAvailableInputDevices()
{
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    if (deviceType != nullptr)
    {
        deviceType->scanForDevices();
        return deviceType->getDeviceNames(true);
    }
    return juce::StringArray();
}

juce::StringArray AudioEngine::getAvailableOutputDevices()
{
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    if (deviceType != nullptr)
    {
        deviceType->scanForDevices();
        return deviceType->getDeviceNames(false);
    }
    return juce::StringArray();
}

void AudioEngine::setBassGain(float gainDb)
{
    bassGainDb = gainDb;
    updateEQFilters();
}

void AudioEngine::setMidGain(float gainDb)
{
    midGainDb = gainDb;
    updateEQFilters();
}

void AudioEngine::setTrebleGain(float gainDb)
{
    trebleGainDb = gainDb;
    updateEQFilters();
}

void AudioEngine::updateEQFilters()
{
    auto bassCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        currentSampleRate, 200.0f, 0.707f, juce::Decibels::decibelsToGain(bassGainDb));
    *bassFilterL.coefficients = *bassCoeffs;
    *bassFilterR.coefficients = *bassCoeffs;
    
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, 1000.0f, 1.0f, juce::Decibels::decibelsToGain(midGainDb));
    *midFilterL.coefficients = *midCoeffs;
    *midFilterR.coefficients = *midCoeffs;
    
    auto trebleCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        currentSampleRate, 4000.0f, 0.707f, juce::Decibels::decibelsToGain(trebleGainDb));
    *trebleFilterL.coefficients = *trebleCoeffs;
    *trebleFilterR.coefficients = *trebleCoeffs;
}

void AudioEngine::loadPlugin(const juce::File& pluginFile)
{
    juce::OwnedArray<juce::PluginDescription> descriptions;
    
    for (int i = 0; i < pluginFormatManager.getNumFormats(); ++i)
    {
        auto* format = pluginFormatManager.getFormat(i);
        juce::KnownPluginList pluginList;
        pluginList.scanAndAddFile(pluginFile.getFullPathName(), true, descriptions, *format);
    }
    
    if (descriptions.size() > 0)
    {
        juce::String errorMessage;
        pluginInstance = pluginFormatManager.createPluginInstance(
            *descriptions[0], currentSampleRate, 
            (int)pluginBuffer.getNumSamples(), errorMessage);
        
        if (pluginInstance != nullptr)
            pluginInstance->prepareToPlay(currentSampleRate, pluginBuffer.getNumSamples());
    }
}

void AudioEngine::removePlugin()
{
    if (pluginInstance != nullptr)
    {
        pluginInstance->releaseResources();
        pluginInstance.reset();
    }
}

juce::String AudioEngine::getPluginName() const
{
    if (pluginInstance != nullptr)
        return pluginInstance->getName();
    return "No plugin loaded";
}
