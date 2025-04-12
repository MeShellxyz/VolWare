#pragma once
#include "VolumeController.h"

#include <atlbase.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <windows.h>

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Windows-specific implementation of VolumeController
 */
class VolumeController::Impl {
public:
    Impl();
    ~Impl();

    // Volume control methods
    bool setMasterVolume(float volumeLevel);
    bool setVolume(const std::string &processName, float volumeLevel);
    bool setVolume(const std::vector<std::string> &processNames,
                   float volumeLevel);

    // Mute control methods
    bool setMasterMute(int mute);
    bool setMute(const std::string &processName, int mute);
    bool setMute(const std::vector<std::string> &processNames, int mute);

private:
    // Windows COM initialization
    bool initializeCOM();

    // Internal implementation methods (thread-unsafe)
    bool setVolumeInternal(const std::string &processName, float volumeLevel);
    bool setMuteInternal(const std::string &processName, int mute);

    // Process utilities
    struct CacheProcessEntry {
        std::string name;
        std::chrono::system_clock::time_point timestamp;
    };

    static std::unordered_map<DWORD, CacheProcessEntry> processNameCache;
    static std::string WideToUtf8(const wchar_t *wstr);
    static std::string getProcessNameFromId(const DWORD &processId);

    // Audio session management
    std::vector<CComPtr<ISimpleAudioVolume>>
    getAudioSessionsForProcess(const std::string &processName);

    // Windows COM interfaces
    CComPtr<IMMDeviceEnumerator> pEnumerator;
    CComPtr<IMMDevice> pDevice;
    CComPtr<IAudioEndpointVolume> pEndpointVolume;
    CComPtr<IAudioSessionManager2> pSessionManager;

    // Thread safety
    std::mutex mtx;
};
