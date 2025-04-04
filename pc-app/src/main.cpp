#include "Config.h"
#include "SerialReader.h"
#include "VolumeController.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    try {
        VolumeController volumeController;
        Config config;

        SerialReader serialReader(config.getComPort(), config.getBaudRate());

        serialReader.setCallback(
            [&volumeController, &config](const std::vector<int> &data) {
                for (int i = 0; i < data.size() && i < config.getChannelCount();
                     ++i) {
                    float volumeLevel = static_cast<float>(data[i]) / 1024.0f;
                    if (config.isInvertSlider()) {
                        volumeLevel = 1.0f - volumeLevel;
                    }
                    const std::vector<std::string> &apps =
                        config.getChannelApps().at(i);
                    volumeController.setVolume(apps, volumeLevel);
                }
            });

        serialReader.setSyncMessage("SYNC\n");

        if (!serialReader.start()) {
            std::cerr << "Failed to start serial reader." << std::endl;
            return 1;
        }

        std::cout << "Serial reader started. Listening for data..."
                  << std::endl;

        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        serialReader.stop();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
