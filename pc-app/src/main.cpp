#include "Config.h"
#include "SerialReader.h"
#include "VolumeController.h"
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

#include "WindowsAutostart.h"
#include "WindowsTray.h"

#include <atomic>

std::atomic<bool> g_running = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    try {
        Config config;

        AutoStart::SetAutoStart(config.isAutoStart());

        VolumeController volumeController;

        SerialReader serialReader(config.getComPort(), config.getBaudRate());

        if (config.isMuteButtons()) {
            serialReader.setCallback([&volumeController,
                                      &config](const std::vector<int> &data) {
                for (int i = 0; i < data.size() && i < config.getChannelCount();
                     ++i) {
                    float volumeLevel = static_cast<float>(data[i]) / 1024.0f;
                    if (config.isInvertSlider()) {
                        volumeLevel = 1.0f - volumeLevel;
                    }
                    int mute = data[i + config.getChannelCount()];

                    const std::vector<std::string> &apps =
                        config.getChannelApps().at(i);
                    volumeController.setVolume(apps, volumeLevel);
                    volumeController.setMute(apps, mute);
                }
            });
        } else {
            serialReader.setCallback([&volumeController,
                                      &config](const std::vector<int> &data) {
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
        }

        serialReader.setSyncMessage("s");

        if (!serialReader.start()) {
            std::cerr << "Failed to start serial reader." << std::endl;
            return 1;
        }

        WindowsTray tray(hInstance, "VolWare Volume Controller");

        tray.setOnExitCallback([&]() {
            g_running = false;
            PostQuitMessage(0);
        });

        MSG msg;
        while (g_running) {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) {
                    g_running = false;
                }
            } else {
                Sleep(100); // Sleep to prevent high CPU usage
            }
        }

        serialReader.stop();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

#endif // _WIN32 || _WIN64