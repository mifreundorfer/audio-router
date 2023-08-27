#include "MainComponent.h"

static const char* autostartRegKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\AudioRouter";

static juce::String getAutostartCommand()
{
    return "\"" + juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName() + "\" --background";
}

MainComponent::MainComponent(Router* router) :
    router(router)
{
    setSize(350, 180);

    addAndMakeVisible(statusLabel);

    inputDeviceLabel.setText("Input Device:", juce::dontSendNotification);
    inputDeviceChooser.addItemList(router->getInputDeviceNames(), 1);
    inputDeviceChooser.setText(router->inputDeviceName, juce::dontSendNotification);
    inputDeviceChooser.addListener(this);
    addAndMakeVisible(inputDeviceLabel);
    addAndMakeVisible(inputDeviceChooser);

    outputDeviceLabel.setText("Output Device:", juce::dontSendNotification);
    outputDeviceChooser.addItemList(router->getOutputDevicNames(), 1);
    outputDeviceChooser.setText(router->outputDeviceName, juce::dontSendNotification);
    outputDeviceChooser.addListener(this);
    addAndMakeVisible(outputDeviceLabel);
    addAndMakeVisible(outputDeviceChooser);

    channelCountLabel.setText("Channel Count:", juce::dontSendNotification);
    channelCountField.setText(juce::String(router->channelCount));
    channelCountField.addListener(this);
    addAndMakeVisible(channelCountLabel);
    addAndMakeVisible(channelCountField);

    auto path = juce::WindowsRegistry::getValue(autostartRegKey);
    bool autostartEnabled = path == getAutostartCommand();

    autostartToggle.setButtonText("Start automatically when Windows starts");
    autostartToggle.setToggleState(autostartEnabled, juce::dontSendNotification);
    autostartToggle.addListener(this);
    addAndMakeVisible(autostartToggle);

    startButton.setButtonText("Start");
    startButton.addListener(this);
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    addAndMakeVisible(startButton);
    addAndMakeVisible(stopButton);

    startTimer(200);

    // Update the status label
    timerCallback();
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto statusColour = router->isRunning() ? juce::Colours::green : juce::Colours::red;
    g.setColour(statusColour);
    auto circleRect = statusRect.reduced(4);
    circleRect += juce::Point(2, 0);
    g.fillEllipse(circleRect.toFloat());
}

void MainComponent::resized()
{
    auto rect = getLocalBounds();
    rect.reduce(8, 8);

    juce::Rectangle<int> row;

    row = rect.removeFromTop(20);
    statusRect = row.removeFromLeft(20);
    statusLabel.setBounds(row);

    rect.removeFromTop(8);

    row = rect.removeFromTop(20);
    inputDeviceLabel.setBounds(row.removeFromLeft(120));
    inputDeviceChooser.setBounds(row);

    rect.removeFromTop(8);

    row = rect.removeFromTop(20);
    outputDeviceLabel.setBounds(row.removeFromLeft(120));
    outputDeviceChooser.setBounds(row);

    rect.removeFromTop(8);

    row = rect.removeFromTop(20);
    channelCountLabel.setBounds(row.removeFromLeft(120));
    channelCountField.setBounds(row);

    rect.removeFromTop(8);

    row = rect.removeFromTop(20);
    autostartToggle.setBounds(row);

    row = rect.removeFromBottom(20);
    stopButton.setBounds(row.removeFromRight(80));
    row.removeFromRight(8);
    startButton.setBounds(row.removeFromRight(80));
}

void MainComponent::timerCallback()
{
    statusLabel.setText(router->isRunning() ? "Running" : "Stopped", juce::dontSendNotification);
    repaint(statusRect);
}

void MainComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &inputDeviceChooser)
    {
        int index = inputDeviceChooser.getSelectedItemIndex();
        if (index != -1)
        {
            router->inputDeviceName = inputDeviceChooser.getItemText(index);
        }
    }

    if (comboBox == &outputDeviceChooser)
    {
        int index = outputDeviceChooser.getSelectedItemIndex();
        if (index != -1)
        {
            router->outputDeviceName = outputDeviceChooser.getItemText(index);
        }
    }
}

void MainComponent::textEditorReturnKeyPressed(juce::TextEditor& textEditor)
{
    juce::Component::unfocusAllComponents();
}

void MainComponent::textEditorFocusLost(juce::TextEditor& textEditor)
{
    if (&textEditor == &channelCountField)
    {
        int channelCount = channelCountField.getText().getIntValue();
        if (channelCount > 0 && channelCount < 10)
        {
            router->channelCount = channelCount;
        }
        else
        {
            channelCountField.setText(juce::String(router->channelCount));
        }
    }
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &startButton)
    {
        router->start();
    }

    if (button == &stopButton)
    {
        router->stop();
    }

    if (button == &autostartToggle)
    {
        bool autostartEnabled = autostartToggle.getToggleState();

        if (autostartEnabled)
        {
            juce::WindowsRegistry::setValue(autostartRegKey, getAutostartCommand());
        }
        else
        {
            juce::WindowsRegistry::deleteValue(autostartRegKey);
        }
    }
}
