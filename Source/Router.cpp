#include "Router.h"

void RouterAudioCallback::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
    int numInputChannels,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& context)
{
    for (int i = 0; i < numOutputChannels; i++)
    {
        if (i < numInputChannels)
        {
            juce::FloatVectorOperations::copy(outputChannelData[i], inputChannelData[i], numSamples);
        }
        else
        {
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
        }
    }
}

Router::Router()
{
    deviceType.reset(juce::AudioIODeviceType::createAudioIODeviceType_WASAPI(juce::WASAPIDeviceMode::exclusive));
    deviceType->scanForDevices();

    // Try restarting the device every 2 seconds if the start has failed
    startTimer(2000);
}

Router::~Router()
{
    stop();
}

void Router::start()
{
    stop();

    isStarted = true;
    tryStartingAudioDevice();
}

void Router::stop()
{
    isStarted = false;

    if (audioDevice != nullptr)
    {
        audioDevice->stop();
        audioDevice.reset();
    }
}

void Router::saveSettingsToFile(juce::File file)
{
    juce::XmlElement rootElement("AudioRouter");

    rootElement.setAttribute("channelCount", channelCount);

    auto inputDevice = new juce::XmlElement("InputDevice");
    inputDevice->setAttribute("name", inputDeviceName);
    rootElement.addChildElement(inputDevice);

    auto outputDevice = new juce::XmlElement("OutputDevice");
    outputDevice->setAttribute("name", outputDeviceName);
    rootElement.addChildElement(outputDevice);

    if (!rootElement.writeTo(file))
    {
        juce::Logger::getCurrentLogger()->writeToLog("Failed to save settings file");
    }
}

void Router::loadSettingsFromFile(juce::File file)
{
    auto doc = juce::XmlDocument::parse(file);
    channelCount = 0;
    inputDeviceName = "";
    outputDeviceName = "";
    if (doc == nullptr)
    {
        juce::Logger::getCurrentLogger()->writeToLog("Failed to parse settings file");
        return;
    }

    channelCount = doc->getIntAttribute("channelCount");

    auto elem = doc->getChildByName("InputDevice");
    if (elem == nullptr)
    {
        juce::Logger::getCurrentLogger()->writeToLog("InputDevice entry not found");
    }
    else
    {
        inputDeviceName = elem->getStringAttribute("name");
    }

    elem = doc->getChildByName("OutputDevice");
    if (elem == nullptr)
    {
        juce::Logger::getCurrentLogger()->writeToLog("OutputDevice entry not found");
    }
    else
    {
        outputDeviceName = elem->getStringAttribute("name");
    }
}

void Router::timerCallback()
{
    if (isStarted && !isRunning())
    {
        tryStartingAudioDevice();
    }
}

void Router::tryStartingAudioDevice()
{
    if (inputDeviceName.isEmpty() || outputDeviceName.isEmpty())
        return;

    audioDevice.reset(deviceType->createDevice(outputDeviceName, inputDeviceName));

    if (audioDevice != nullptr)
    {
        juce::BigInteger channelIndices = 0;
        for (int i = 0; i < channelCount; i++)
        {
            channelIndices |= 1 << i;
        }

        int smallestBufferSize = 0;
        for (int bufferSize : audioDevice->getAvailableBufferSizes())
        {
            if (smallestBufferSize == 0)
            {
                smallestBufferSize = bufferSize;
            }
            else if (bufferSize < smallestBufferSize)
            {
                smallestBufferSize = bufferSize;
            }
        }

        juce::String error = audioDevice->open(channelIndices, channelIndices, 48000, smallestBufferSize);
        if (error.isNotEmpty())
        {
            juce::Logger::getCurrentLogger()->writeToLog("Failed to open audio device");
            juce::Logger::getCurrentLogger()->writeToLog(error);
        }
        else
        {
            audioDevice->start(&audioCallback);
        }
    }
}
