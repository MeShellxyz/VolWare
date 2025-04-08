#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

class Config {
private:
    std::string mConfigFilePath;

    std::string mComPort;
    int mBaudRate;
    bool mInvertSlider;
    std::unordered_map<int, std::vector<std::string>> mChannelApps;

    void loadConfig();

public:
    Config();
    explicit Config(const std::string &configFilePath);

    const std::string &getComPort() const { return mComPort; };
    int getBaudRate() const { return mBaudRate; };
    bool isInvertSlider() const { return mInvertSlider; };
    const std::unordered_map<int, std::vector<std::string>> &
    getChannelApps() const {
        return mChannelApps;
    };

    int getChannelCount() const { return mChannelApps.size(); };
};
