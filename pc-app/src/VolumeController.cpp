#include "VolumeController.h"

#include <TlHelp32.h>

#include <algorithm>
#include <stdexcept>

VolumeController::VolumeController() {
    if (!initializeCOM()) {
        throw std::runtime_error("Failed to initialize COM.");
    }
}

VolumeController::~VolumeController() { CoUninitialize(); }

bool VolumeController::initializeCOM() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        return false;
    }

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator),
                          reinterpret_cast<void **>(&pEnumerator));
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                           reinterpret_cast<void **>(&pEndpointVolume));
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
                           reinterpret_cast<void **>(&pSessionManager));
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    return true;
}

std::unordered_map<DWORD, VolumeController::CacheProcessEntry>
    VolumeController::processNameCache;

std::wstring VolumeController::getProcessNameFromId(const DWORD &processId) {
    auto now = std::chrono::system_clock::now();
    auto it = processNameCache.find(processId);
    if (it != processNameCache.end() &&
        (now - it->second.timestamp) < std::chrono::seconds(30)) {
        return it->second.name;
    }

    std::wstring processName = L"<unknown>";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE, processId);
    if (hProcess) {
        WCHAR szProcessPath[MAX_PATH] = {0};
        DWORD dwSize = MAX_PATH;

        // Get the full path
        if (QueryFullProcessImageNameW(hProcess, 0, szProcessPath, &dwSize)) {

            WCHAR *fileName = wcsrchr(szProcessPath, L'\\');
            if (fileName) {
                processName = fileName + 1; // Skip the backslash
                processNameCache[processId] = {processName, now};
            }
        }
        CloseHandle(hProcess);
    }
    return processName;
}

std::vector<CComPtr<ISimpleAudioVolume>>
VolumeController::getAudioSessionsForProcess(const std::wstring &processName) {
    std::vector<CComPtr<ISimpleAudioVolume>> sessions;
    CComPtr<IAudioSessionEnumerator> pSessionEnumerator = nullptr;
    HRESULT hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    if (FAILED(hr)) {
        return sessions;
    }
    int sessionCount = 0;
    hr = pSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr)) {
        return sessions;
    }

    for (int i = 0; i < sessionCount; i++) {
        CComPtr<IAudioSessionControl> pSessionControl = nullptr;
        hr = pSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(hr)) {
            continue;
        }

        CComPtr<IAudioSessionControl2> pSessionControl2 = nullptr;
        hr = pSessionControl->QueryInterface(
            __uuidof(IAudioSessionControl2),
            reinterpret_cast<void **>(&pSessionControl2));
        if (FAILED(hr)) {
            continue;
        }

        DWORD processId = 0;
        hr = pSessionControl2->GetProcessId(&processId);
        if (FAILED(hr)) {
            continue;
        }

        std::wstring currentProcessName = getProcessNameFromId(processId);
        std::transform(currentProcessName.begin(), currentProcessName.end(),
                       currentProcessName.begin(), ::towlower);
        if (currentProcessName == processName) {
            CComPtr<ISimpleAudioVolume> pSimpleVolume = nullptr;
            hr = pSessionControl->QueryInterface(
                __uuidof(ISimpleAudioVolume),
                reinterpret_cast<void **>(&pSimpleVolume));
            if (SUCCEEDED(hr)) {
                sessions.push_back(pSimpleVolume);
            }
        }
    }
    return sessions;
}
bool VolumeController::setVolume(float volumeLevel) {
    std::lock_guard<std::mutex> lock(mtx);

    // Clip volume level to [0.0, 1.0]
    volumeLevel = std::clamp(volumeLevel, 0.0f, 1.0f);

    HRESULT hr =
        pEndpointVolume->SetMasterVolumeLevelScalar(volumeLevel, nullptr);

    return SUCCEEDED(hr);
}

bool VolumeController::setVolumeInternal(const std::wstring &processName,
                                         float volumeLevel) {
    // Clip volume level to [0.0, 1.0]
    volumeLevel = std::clamp(volumeLevel, 0.0f, 1.0f);

    for (auto &session : getAudioSessionsForProcess(processName)) {
        if (session) {
            HRESULT hr = session->SetMasterVolume(volumeLevel, nullptr);
            if (FAILED(hr)) {
                return false;
            }
        }
    }
    return true;
}

bool VolumeController::setVolume(const std::wstring &processName,
                                 float volumeLevel) {
    std::lock_guard<std::mutex> lock(mtx);
    return setVolumeInternal(processName, volumeLevel);
}

bool VolumeController::setVolume(const std::wstring *processNames, size_t count,
                                 float volumeLevel) {
    std::lock_guard<std::mutex> lock(mtx);
    // Clip volume level to [0.0, 1.0]
    volumeLevel = std::clamp(volumeLevel, 0.0f, 1.0f);
    for (size_t i = 0; i < count; ++i) {
        if (!setVolumeInternal(processNames[i], volumeLevel)) {
            return false;
        }
    }
    return true;
}

bool VolumeController::toggleMute() {
    // toggle master mute
    std::lock_guard<std::mutex> lock(mtx);

    BOOL isMuted = FALSE;
    HRESULT hr = pEndpointVolume->GetMute(&isMuted);
    if (SUCCEEDED(hr)) {
        hr = pEndpointVolume->SetMute(!isMuted, nullptr);
    }
    return SUCCEEDED(hr);
}

bool VolumeController::toggleMuteInternal(const std::wstring &processName) {
    for (auto &session : getAudioSessionsForProcess(processName)) {
        if (session) {
            BOOL isMuted = FALSE;
            HRESULT hr = session->GetMute(&isMuted);
            if (SUCCEEDED(hr)) {
                hr = session->SetMute(!isMuted, nullptr);
            }
            if (FAILED(hr)) {
                return false;
            }
        }
    }
    return true;
}

bool VolumeController::toggleMute(const std::wstring &processName) {
    std::lock_guard<std::mutex> lock(mtx);
    return toggleMuteInternal(processName);
}

bool VolumeController::toggleMute(const std::wstring *processNames,
                                  size_t count) {
    std::lock_guard<std::mutex> lock(mtx);
    for (size_t i = 0; i < count; ++i) {
        if (!toggleMuteInternal(processNames[i])) {
            return false;
        }
    }
    return true;
}