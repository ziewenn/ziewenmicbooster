#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

#ifdef _WIN32
#include <windows.h>
#endif

class TrayIcon : public juce::SystemTrayIconComponent
{
public:
    TrayIcon(std::function<void()> onShow, std::function<void()> onQuit)
        : showCallback(std::move(onShow)), quitCallback(std::move(onQuit))
    {
        juce::Image icon(juce::Image::ARGB, 16, 16, true);
        juce::Graphics g(icon);
        g.setColour(juce::Colour(0xff00d2ff));
        g.fillEllipse(1.0f, 1.0f, 14.0f, 14.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("M", 0, 0, 16, 16, juce::Justification::centred);
        setIconImage(icon, icon);
        setIconTooltip("Mic Booster");
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Show Mic Booster");
            menu.addSeparator();
            menu.addItem(2, "Quit");
            
            menu.showMenuAsync(juce::PopupMenu::Options(),
                [this](int result)
                {
                    if (result == 1 && showCallback)
                        showCallback();
                    else if (result == 2 && quitCallback)
                        quitCallback();
                });
        }
        else if (e.mods.isLeftButtonDown())
        {
            if (showCallback)
                showCallback();
        }
    }
    
private:
    std::function<void()> showCallback;
    std::function<void()> quitCallback;
};

class MicBoosterApplication : public juce::JUCEApplication
{
public:
    MicBoosterApplication() {}

    const juce::String getApplicationName() override { return "Mic Booster"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName(), this));
        trayIcon = std::make_unique<TrayIcon>(
            [this]() { showWindow(); },
            [this]() { reallyQuit(); }
        );
    }

    void shutdown() override
    {
        trayIcon = nullptr;
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
            mainWindow->setVisible(false);
    }
    
    void showWindow()
    {
        if (mainWindow != nullptr)
        {
            mainWindow->setVisible(true);
            mainWindow->toFront(true);
        }
    }
    
    void reallyQuit()
    {
        trayIcon = nullptr;
        quit();
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name, MicBoosterApplication* app)
            : DocumentWindow(name, juce::Colours::darkgrey, DocumentWindow::allButtons),
              ownerApp(app)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(false, false);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            ownerApp->systemRequestedQuit();
        }

    private:
        MicBoosterApplication* ownerApp;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<TrayIcon> trayIcon;
};

START_JUCE_APPLICATION(MicBoosterApplication)
