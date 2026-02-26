#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

class UpdateChecker : private juce::Thread
{
public:
    static constexpr const char* CURRENT_VERSION = "1.0.0";
    static constexpr const char* GITHUB_API_URL = "https://api.github.com/repos/ziewenn/ziewenmicbooster/releases/latest";
    
    struct UpdateInfo
    {
        bool available = false;
        juce::String version;
        juce::String downloadUrl;
        juce::String releaseNotes;
    };
    
    UpdateChecker() : Thread("UpdateChecker") {}
    
    ~UpdateChecker()
    {
        stopThread(5000);
    }
    
    void checkForUpdates()
    {
        startThread(juce::Thread::Priority::low);
    }
    
    UpdateInfo getUpdateInfo() const
    {
        juce::ScopedLock lock(infoLock);
        return latestInfo;
    }
    
    bool isChecking() const { return isThreadRunning(); }
    
    std::function<void(const UpdateInfo&)> onUpdateFound;
    
    void downloadUpdate(const juce::String& url)
    {
        auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
        auto tempExe = tempDir.getChildFile("MicBooster_update.exe");
        auto currentExe = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        
        juce::URL downloadUrl(url);
        auto stream = downloadUrl.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(30000));
        
        if (stream == nullptr)
            return;
        
        tempExe.deleteFile();
        juce::FileOutputStream fos(tempExe);
        if (!fos.openedOk())
            return;
        
        char buffer[8192];
        while (!stream->isExhausted())
        {
            auto bytesRead = stream->read(buffer, sizeof(buffer));
            if (bytesRead <= 0) break;
            fos.write(buffer, (size_t)bytesRead);
        }
        fos.flush();
        
        auto batFile = tempDir.getChildFile("micbooster_update.bat");
        batFile.replaceWithText(
            "@echo off\r\n"
            "timeout /t 2 /nobreak >nul\r\n"
            "copy /y \"" + tempExe.getFullPathName() + "\" \"" + currentExe.getFullPathName() + "\"\r\n"
            "start \"\" \"" + currentExe.getFullPathName() + "\"\r\n"
            "del \"" + tempExe.getFullPathName() + "\"\r\n"
            "del \"%~f0\"\r\n"
        );
        
        batFile.startAsProcess();
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
private:
    void run() override
    {
        juce::URL apiUrl(GITHUB_API_URL);
        auto stream = apiUrl.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(10000)
            .withExtraHeaders("Accept: application/vnd.github.v3+json\r\nUser-Agent: MicBooster"));
        
        if (stream == nullptr)
            return;
        
        auto response = stream->readEntireStreamAsString();
        auto json = juce::JSON::parse(response);
        
        if (!json.isObject())
            return;
        
        auto tagName = json.getProperty("tag_name", "").toString().trimCharactersAtStart("vV");
        auto body = json.getProperty("body", "").toString();
        
        auto assets = json.getProperty("assets", juce::var());
        juce::String downloadUrl;
        
        if (assets.isArray())
        {
            for (int i = 0; i < assets.size(); ++i)
            {
                auto asset = assets[i];
                auto name = asset.getProperty("name", "").toString().toLowerCase();
                if (name.endsWith(".exe"))
                {
                    downloadUrl = asset.getProperty("browser_download_url", "").toString();
                    break;
                }
            }
        }
        
        if (tagName.isNotEmpty() && isNewerVersion(tagName))
        {
            UpdateInfo info;
            info.available = true;
            info.version = tagName;
            info.downloadUrl = downloadUrl;
            info.releaseNotes = body;
            
            {
                juce::ScopedLock lock(infoLock);
                latestInfo = info;
            }
            
            if (onUpdateFound)
            {
                juce::MessageManager::callAsync([this, info]()
                {
                    if (onUpdateFound)
                        onUpdateFound(info);
                });
            }
        }
    }
    
    bool isNewerVersion(const juce::String& remoteVersion)
    {
        auto current = juce::StringArray::fromTokens(CURRENT_VERSION, ".", "");
        auto remote = juce::StringArray::fromTokens(remoteVersion, ".", "");
        
        for (int i = 0; i < juce::jmax(current.size(), remote.size()); ++i)
        {
            int c = (i < current.size()) ? current[i].getIntValue() : 0;
            int r = (i < remote.size()) ? remote[i].getIntValue() : 0;
            if (r > c) return true;
            if (r < c) return false;
        }
        return false;
    }
    
    mutable juce::CriticalSection infoLock;
    UpdateInfo latestInfo;
};
