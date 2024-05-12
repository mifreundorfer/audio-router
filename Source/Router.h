#pragma once

#include <JuceHeader.h>

class Router;

class RouterAudioCallback : public juce::AudioIODeviceCallback
{
public:
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}

    void audioDeviceStopped() override {}
};

class Router : public juce::Timer
{
public:
    Router();
    ~Router();

    void start();
    void stop();
    bool isRunning() const { return audioDevice != nullptr && audioDevice->isPlaying(); }

    void saveSettingsToFile(juce::File file);
    void loadSettingsFromFile(juce::File file);

    juce::StringArray getInputDeviceNames() { return deviceType->getDeviceNames(true); }
    juce::StringArray getOutputDevicNames() { return deviceType->getDeviceNames(false); }
    juce::StringArray getAvailableBufferSizes();

    void timerCallback() override;

    int channelCount = 2;
    int bufferSize = 0;
    juce::String inputDeviceName = "";
    juce::String outputDeviceName = "";

private:
    void tryStartingAudioDevice();

    bool isStarted = false;
    RouterAudioCallback audioCallback;
	std::unique_ptr<juce::AudioIODevice> audioDevice;
    std::unique_ptr<juce::AudioIODeviceType> deviceType;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Router);
};
