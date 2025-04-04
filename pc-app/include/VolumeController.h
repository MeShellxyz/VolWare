#pragma once

#include <windows.h>

#include <atlbase.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <mutex>

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

class VolumeController {
private:
    std::mutex mtx;

    bool initializeCOM();

    CComPtr<IMMDeviceEnumerator> pEnumerator;
    CComPtr<IMMDevice> pDevice;
    CComPtr<IAudioEndpointVolume> pEndpointVolume;
    CComPtr<IAudioSessionManager2> pSessionManager;

    // CacheProcessEntry
    struct CacheProcessEntry {
        std::string name;
        std::chrono::system_clock::time_point timestamp;
    };

    static std::unordered_map<DWORD, CacheProcessEntry> processNameCache;
    static std::string WideToUtf8(const wchar_t *wstr);
    static std::string getProcessNameFromId(const DWORD &processId);

    std::vector<CComPtr<ISimpleAudioVolume>>
    getAudioSessionsForProcess(const std::string &processName);

    bool setMasterVolume(float volumeLevel);
    bool toggleMasterMute();

    bool setVolumeInternal(const std::string &processName, float volumeLevel);
    bool toggleMuteInternal(const std::string &processName);

public:
    VolumeController();
    ~VolumeController();

    bool setVolume(const std::string &processName, float volumeLevel);
    bool setVolume(const std::vector<std::string> &processNames,
                   float volumeLevel);

    bool toggleMute(const std::string &processName);
    bool toggleMute(const std::vector<std::string> &processNames);
};
