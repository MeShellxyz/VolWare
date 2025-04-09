#include "Config.h"

#include <filesystem>

Config::Config() : mConfigFilePath("config.yaml") { loadConfig(); }

Config::Config(const std::string &configFilePath)
    : mConfigFilePath(configFilePath) {
    loadConfig();
}

void Config::loadConfig() {
    try {
        auto yamlPath = std::filesystem::current_path() / mConfigFilePath;

        YAML::Node config = YAML::LoadFile(yamlPath.string());

        if (config["com_port"]) {
            mComPort = config["com_port"].as<std::string>();
        } else {
            throw std::runtime_error("Missing 'com_port' in config file.");
        }
        if (config["baud_rate"]) {
            mBaudRate = config["baud_rate"].as<int>();
        } else {
            throw std::runtime_error("Missing 'baud_rate' in config file.");
        }
        if (config["invert_slider"]) {
            mInvertSlider = config["invert_slider"].as<bool>();
        } else {
            throw std::runtime_error("Missing 'invert_slider' in config file.");
        }
        if (config["auto_start"]) {
            mAutoStart = config["auto_start"].as<bool>();
        } else {
            throw std::runtime_error("Missing 'auto_start' in config file.");
        }
        if (config["channel_apps"]) {
            int channelCount = config["channel_apps"].size();
            for (const auto &channel : config["channel_apps"]) {
                int channelNumber = channel.first.as<int>();
                std::vector<std::string> apps =
                    channel.second.as<std::vector<std::string>>();
                mChannelApps[channelNumber] = apps;
            }
        } else {
            throw std::runtime_error("Missing 'channel_apps' in config file.");
        }
    } catch (const YAML::Exception &e) {
        throw std::runtime_error("YAML parsing error: " +
                                 std::string(e.what()));
    } catch (const std::exception &e) {
        throw std::runtime_error("Error loading config: " +
                                 std::string(e.what()));
    }
}
