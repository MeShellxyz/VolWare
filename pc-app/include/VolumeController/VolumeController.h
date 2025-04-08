#pragma once

#include <memory>
#include <string>
#include <vector>

class VolumeController {
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

public:
    VolumeController();
    ~VolumeController();

    VolumeController(const VolumeController &) = delete;
    VolumeController &operator=(const VolumeController &) = delete;
    VolumeController(VolumeController &&) noexcept;
    VolumeController &operator=(VolumeController &&) noexcept;

    bool setMasterVolume(float volumeLevel);
    bool setVolume(const std::string &processName, float volumeLevel);
    bool setVolume(const std::vector<std::string> &processNames,
                   float volumeLevel);

    bool setMasterMute(int mute);
    bool setMute(const std::string &processName, int mute);
    bool setMute(const std::vector<std::string> &processNames, int mute);
};
