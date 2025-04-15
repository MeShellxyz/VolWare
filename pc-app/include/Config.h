#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

class Config {
public:
    Config();
    explicit Config(const std::string &configFilePath);

    // Accessors
    const std::string &getComPort() const { return m_comPort; }
    int getBaudRate() const { return m_baudRate; }
    bool isMuteButtons() const { return m_muteButtons; }
    bool isInvertSlider() const { return m_invertSlider; }
    bool isAutoStart() const { return m_autoStart; }

    const std::unordered_map<int, std::vector<std::string>> &
    getChannelApps() const {
        return m_channelApps;
    }

    int getChannelCount() const { return m_channelApps.size(); }

private:
    void loadConfig();

    // Configuration file path
    std::string m_configFilePath;

    // Configuration parameters
    std::string m_comPort;
    int m_baudRate;
    bool m_muteButtons;
    bool m_invertSlider;
    bool m_autoStart;
    std::unordered_map<int, std::vector<std::string>> m_channelApps;
};
