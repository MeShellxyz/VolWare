
#include "VolumeControllerImpl_Windows.h"

#include <TlHelp32.h>
#include <codecvt>

#include <algorithm>
#include <stdexcept>

VolumeController::Impl::Impl() {
    if (!initializeCOM()) {
        throw std::runtime_error("Failed to initialize COM.");
    }
}

VolumeController::Impl::~Impl() { CoUninitialize(); }

bool VolumeController::Impl::initializeCOM() {
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

std::unordered_map<DWORD, VolumeController::Impl::CacheProcessEntry>
    VolumeController::Impl::processNameCache;

std::string VolumeController::Impl::WideToUtf8(const wchar_t *wstr) {
    if (!wstr)
        return "";

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    try {
        return converter.to_bytes(wstr);
    } catch (const std::exception &) {
        return "<conversion_error>";
    }
}

std::string
VolumeController::Impl::getProcessNameFromId(const DWORD &processId) {
    auto now = std::chrono::system_clock::now();
    auto it = processNameCache.find(processId);
    if (it != processNameCache.end() &&
        (now - it->second.timestamp) < std::chrono::seconds(30)) {
        return it->second.name;
    }

    std::string processName = "<unknown>";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE, processId);
    if (hProcess) {
        WCHAR szProcessPath[MAX_PATH] = {0};
        DWORD dwSize = MAX_PATH;

        // Get the full path
        if (QueryFullProcessImageNameW(hProcess, 0, szProcessPath, &dwSize)) {
            WCHAR *fileName = wcsrchr(szProcessPath, L'\\');
            if (fileName) {
                // Convert wide string to UTF-8 using WideCharToMultiByte
                fileName++; // Skip the backslash
                processName = WideToUtf8(fileName);
                processNameCache[processId] = {processName, now};
            }
        }
    }
    CloseHandle(hProcess);
    return processName;
}

std::vector<CComPtr<ISimpleAudioVolume>>
VolumeController::Impl::getAudioSessionsForProcess(
    const std::string &processName) {
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
    std::string processNameLower = processName;
    std::transform(processNameLower.begin(), processNameLower.end(),
                   processNameLower.begin(), ::towlower);

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
        std::string currentProcessName = getProcessNameFromId(processId);
        std::transform(currentProcessName.begin(), currentProcessName.end(),
                       currentProcessName.begin(), ::towlower);
        if (currentProcessName == processNameLower) {
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
bool VolumeController::Impl::setMasterVolume(float volumeLevel) {

    // Clip volume level to [0.0, 1.0]
    volumeLevel = std::clamp(volumeLevel, 0.0f, 1.0f);

    HRESULT hr =
        pEndpointVolume->SetMasterVolumeLevelScalar(volumeLevel, nullptr);

    return SUCCEEDED(hr);
}

bool VolumeController::Impl::setVolumeInternal(const std::string &processName,
                                               float volumeLevel) {
    // Clip volume level to [0.0, 1.0]
    volumeLevel = std::clamp(volumeLevel, 0.0f, 1.0f);
    std::string processNameLower = processName;
    std::transform(processNameLower.begin(), processNameLower.end(),
                   processNameLower.begin(), ::towlower);

    if (processNameLower == "master") {
        return setMasterVolume(volumeLevel);
    }

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

bool VolumeController::Impl::setVolume(const std::string &processName,
                                       float volumeLevel) {
    std::lock_guard<std::mutex> lock(mtx);
    return setVolumeInternal(processName, volumeLevel);
}

bool VolumeController::Impl::setVolume(
    const std::vector<std::string> &processNames, float volumeLevel) {
    std::lock_guard<std::mutex> lock(mtx);
    // Clip volume level to [0.0, 1.0]
    volumeLevel = std::clamp(volumeLevel, 0.0f, 1.0f);
    for (const auto &processName : processNames) {
        if (!setVolumeInternal(processName, volumeLevel)) {
            return false;
        }
    }
    return true;
}

bool VolumeController::Impl::setMasterMute(int mute) {
    // toggle master mute
    HRESULT hr = pEndpointVolume->SetMute(mute, nullptr);
    return SUCCEEDED(hr);
}

bool VolumeController::Impl::setMuteInternal(const std::string &processName,
                                             int mute) {
    std::string processNameLower = processName;
    std::transform(processNameLower.begin(), processNameLower.end(),
                   processNameLower.begin(), ::towlower);

    if (processNameLower == "master") {
        return setMasterMute(mute);
    }

    for (auto &session : getAudioSessionsForProcess(processName)) {
        if (session) {
            HRESULT hr = session->SetMute(mute, nullptr);
            if (FAILED(hr)) {
                return false;
            }
        }
    }
    return true;
}

bool VolumeController::Impl::setMute(const std::string &processName, int mute) {
    std::lock_guard<std::mutex> lock(mtx);
    return setMuteInternal(processName, mute);
}

bool VolumeController::Impl::setMute(
    const std::vector<std::string> &processNames, int mute) {
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &processName : processNames) {
        if (!setMuteInternal(processName, mute)) {
            return false;
        }
    }
    return true;
}