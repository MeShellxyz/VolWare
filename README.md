# VolWare

<div align="center">
  <img src="pc-app/icons/icon.svg" alt="VolWare Logo" width="200" height="200">
  <br>
  <br>
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
  [![OS: Windows](https://img.shields.io/badge/OS-Windows-blue.svg)](https://www.microsoft.com/windows)
  [![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-orange.svg)](https://isocpp.org/)
  [![Arduino](https://img.shields.io/badge/Arduino-Compatible-teal.svg)](https://www.arduino.cc/)
  
</div>

**VolWare** is a hardware and software solution that allows you to control Windows audio volumes using physical controls. It connects an Arduino with potentiometers to a Windows application that adjusts volume levels for both the master channel and specific applications.

## 📋 Table of Contents

- [Features](#-features)
- [How It Works](#-how-it-works)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Quick Start Guide](#-quick-start-guide)
- [Configuration](#-configuration)
- [Building from Source](#-building-from-source)
- [Acknowledgments](#-acknowledgments)
- [License](#-license)
- [Contributing](#-contributing)

## What is VolWare?

VolWare transforms physical potentiometers into precise volume controls for your Windows system. Adjust your master volume or fine-tune individual applications with a simple twist of a knob – no more hunting through windows and menus to manage your audio environment.

## Features

- Physical volume control via potentiometers
- Per-app audio level adjustment 
- System-wide master volume control
- Mute functionality (optional)
- Low resource footprint
- System tray integration
- Simple YAML configuration
- Noise filtering for stable control

## 🔍 How It Works

VolWare consists of two main components:

1. **Arduino Firmware**: Reads potentiometer values and sends them to the PC via serial connection
2. **Windows Application**: Processes the incoming data and adjusts audio volumes accordingly

The system uses a simple serial protocol to communicate between the Arduino and the PC application, providing low-latency volume control for your master audio and specific applications.

## 🔧 Hardware Requirements

- Arduino (Uno, Nano, or similar)
- Potentiometers (one per audio channel you want to control)
- Optional pushbuttons for mute functionality
- USB cable to connect Arduino to PC
- Breadboard and jumper wires for connections

<details>
<summary>Recommended Hardware Setup</summary>

```
Arduino Pin Connections:
- A0, A1, etc.: Connect to potentiometer middle pins
- 5V: Connect to potentiometer outer pin 1
- GND: Connect to potentiometer outer pin 2
- Digital pins (optional): Connect to momentary buttons for mute functionality
```
</details>

## 💻 Software Requirements

- Windows 10 or later
- CMake (for building from source)
- C++ Compiler (MinGW or MSVC)
- Arduino IDE (for uploading firmware)

## 🚀 Quick Start Guide

### Hardware Setup

1. Connect your potentiometers to analog pins on your Arduino (A0, A1, etc.)
2. Optional: Connect buttons to digital pins for mute functionality
3. Connect your Arduino to your PC via USB

### Software Setup

1. Upload the `mcu/volware/volware.ino` sketch to your Arduino
2. Download the [latest release](https://github.com/yourusername/VolWare/releases) or build from source
3. Configure `config.yaml` to map each channel to your desired applications
4. Adjust volumes using the physical controls!

## ⚙️ Configuration

VolWare uses a YAML configuration file (`config.yaml`) to specify the COM port, baud rate, and application mappings:

```yaml
com_port: "COM3"               # Your Arduino's COM port
baud_rate: 115200              # Communication speed
invert_slider: false           # Set to true if your sliders work in reverse
auto_start: true               # Launch on Windows startup
mute_buttons: false            # Set to true if using mute buttons

# Map each channel to applications (by executable name)
channel_apps:
  0: ["master"]                # First potentiometer controls master volume
  1: ["spotify.exe", "chrome.exe"]  # Second potentiometer controls these apps
  2: ["discord.exe", "teams.exe"]   # Third potentiometer controls these apps
  3: ["game.exe"]              # Fourth potentiometer controls game volume
```

## 🛠️ Building from Source

### Windows Application

```bash
git clone https://github.com/yourusername/VolWare.git
cd VolWare/pc-app
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Arduino Firmware

Open `mcu/volware/volware.ino` in the Arduino IDE and upload it to your device.

## 🙏 Acknowledgments

This project was inspired by [deej](https://github.com/omriharel/deej), a similar project implemented in Go. VolWare is a C++ implementation with some additional features and optimizations.

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

While I'm not actively maintaining contribution guidelines at the moment, pull requests are welcome. Feel free to fork the project and submit improvements.

<details>
<summary>How to contribute</summary>

1. Fork the project
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request
</details>

---

<div align="center">
  <sub>Built with ❤️ by <a href="https://github.com/yourusername">Your Name</a></sub>
</div>