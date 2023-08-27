#pragma once

#include <JuceHeader.h>

#include "Router.h"

class MainComponent : public juce::Component,
                      public juce::Timer,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public juce::Button::Listener
{
public:
    MainComponent(Router* router);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void textEditorReturnKeyPressed(juce::TextEditor& textEditor) override;
    void textEditorFocusLost(juce::TextEditor& textEditor) override;
    void buttonClicked(juce::Button* button) override;

private:
    Router* router;
    juce::Rectangle<int> statusRect;

    juce::Label statusLabel;

    juce::Label inputDeviceLabel;
    juce::ComboBox inputDeviceChooser;

    juce::Label outputDeviceLabel;
    juce::ComboBox outputDeviceChooser;

    juce::Label channelCountLabel;
    juce::TextEditor channelCountField;

    juce::ToggleButton autostartToggle;

    juce::TextButton startButton;
    juce::TextButton stopButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
