#include <JuceHeader.h>
#include "MainComponent.h"
#include "Router.h"

#include <windows.h>

class AudioRouterApplication;

class TrayIcon : public juce::SystemTrayIconComponent
{
public:
    TrayIcon(AudioRouterApplication* app) :
        app(app)
    {
    }

    void mouseDown(const juce::MouseEvent& event) override;

private:
    AudioRouterApplication* app;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrayIcon)
};

class AudioRouterApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLine) override
    {
        juce::Image icon = juce::ImageFileFormat::loadFrom(BinaryData::icon_white_png, BinaryData::icon_white_pngSize);

        trayIcon.reset(new TrayIcon(this));
        trayIcon->setIconImage(icon, icon);

        auto appFolder = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();
        configFile = appFolder.getChildFile("config.xml");
        if (configFile.existsAsFile())
        {
            router.loadSettingsFromFile(configFile);
        }

        auto args = getCommandLineParameterArray();
        if (args.contains("--background"))
        {
            router.start();
        }
        else
        {
            showMainWindow(false);
        }
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        router.saveSettingsToFile(configFile);

        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        showMainWindow(false);
    }

    void showMainWindow(bool bringToFront)
    {
        if (mainWindow == nullptr)
        {
            mainWindow.reset(new MainWindow(this));

            if (bringToFront)
            {
                // XXX HACK: Component::toFront does not actually work.
                // Manually call the windows API with the correct function.
                SetForegroundWindow((HWND)mainWindow->getWindowHandle());
            }
        }
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(AudioRouterApplication* app)
            : DocumentWindow(app->getApplicationName(),
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons),
            app(app)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(&app->router), true);

            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            app->mainWindow.reset();
        }

    private:
        AudioRouterApplication* app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    Router router;
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<TrayIcon> trayIcon;
    juce::File configFile;
};

void TrayIcon::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        app->showMainWindow(true);
    }

    if (event.mods.isPopupMenu())
    {
        juce::PopupMenu menu;

        menu.addItem(1, "Quit");

        menu.showMenuAsync({}, [](int id)
        {
            if (id == 1)
            {
                juce::JUCEApplication::getInstance()->systemRequestedQuit();
            }
        });
    }
}


START_JUCE_APPLICATION(AudioRouterApplication)
