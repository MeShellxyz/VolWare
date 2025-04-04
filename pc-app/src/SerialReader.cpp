#include "SerialReader.h"

#include <chrono>
#include <iostream>
#include <sstream>

SerialReader::SerialReader(const std::string &port, unsigned int baudRate)
    : mPortName(port), mBaudRate(baudRate), mSerialPort(mIoService),
      mRunning(false), mConnected(false) {
    mSyncTimer = std::make_unique<boost::asio::steady_timer>(mIoService);
}

SerialReader::~SerialReader() { stop(); }

bool SerialReader::start() {
    if (mRunning) {
        return true;
    }

    mRunning = true;
    mWorkerThread = std::jthread(&SerialReader::workerThread, this);
    return true;
}

void SerialReader::stop() {
    if (!mRunning) {
        return;
    }

    mRunning = false;
    if (mConnected) {
        closePort();
    }
    mIoService.stop();
}

bool SerialReader::openPort() {
    try {
        mSerialPort.open(mPortName);

        mSerialPort.set_option(
            boost::asio::serial_port_base::baud_rate(mBaudRate));
        mSerialPort.set_option(
            boost::asio::serial_port_base::character_size(8));
        mSerialPort.set_option(boost::asio::serial_port_base::parity(
            boost::asio::serial_port_base::parity::none));
        mSerialPort.set_option(boost::asio::serial_port_base::stop_bits(
            boost::asio::serial_port_base::stop_bits::one));

        std::cout << "Serial port opened: " << mPortName << std::endl;
        mConnected = true;

        scheduleSyncTimer();

    } catch (const std::exception &e) {
        std::cerr << "Error opening port: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool SerialReader::closePort() {
    if (mSerialPort.is_open()) {
        try {
            mSerialPort.cancel();
            mSerialPort.close();
        } catch (const std::exception &e) {
            std::cerr << "Error closing port: " << e.what() << std::endl;
            return false;
        }
    }
    mConnected = false;
    return true;
}

bool SerialReader::sendMessage(const std::string &message) {
    if (!mConnected || !mSerialPort.is_open()) {
        return false;
    }

    try {
        boost::asio::write(mSerialPort, boost::asio::buffer(message));
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
        return false;
    }
}

void SerialReader::sendSyncMessage() {
    if (mConnected && mRunning) {
        if (!sendMessage(mSyncMessage)) {
            std::cerr << "Failed to send sync message." << std::endl;
            mConnected = false;
            closePort();
            mIoService.reset();
            return;
        }
        scheduleSyncTimer();
    }
}

void SerialReader::scheduleSyncTimer() {
    if (!mRunning || !mConnected) {
        return;
    }

    mSyncTimer->expires_after(std::chrono::milliseconds(SYNC_INTERVAL_MS));
    mSyncTimer->async_wait([this](const boost::system::error_code &error) {
        if (!error && mRunning && mConnected) {
            sendSyncMessage();
        }
    });
}

void SerialReader::readStart() {
    if (!mConnected || !mRunning) {
        return;
    }
    boost::asio::async_read_until(mSerialPort, mReadBuffer, '\n',
                                  std::bind(&SerialReader::readComplete, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));
}

void SerialReader::readComplete(const boost::system::error_code &error,
                                size_t bytes_transferred) {
    if (!mRunning) {
        return;
    }

    if (!error) {
        std::istream is(&mReadBuffer);
        std::string line;
        std::getline(is, line);

        std::vector<int> values;
        std::string item;
        std::istringstream iss(line);

        // Parse comma-separated values instead of space-separated
        while (std::getline(iss, item, ',')) {
            try {
                int value = std::stoi(item);
                values.push_back(value);
            } catch (const std::exception &) {
                // Skip invalid values
            }
        }

        if (mCallback) {
            mCallback(values);
        }
        readStart();
    } else {
        std::cerr << "Error reading from serial port: " << error.message()
                  << std::endl;
        closePort();

        mIoService.reset();
    }
}
void SerialReader::workerThread() {
    while (mRunning) {
        if (!mConnected) {
            if (openPort()) {
                readStart();

                mIoService.run();

                mIoService.reset();
            } else {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(RECONNECT_INTERVAL_MS));
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}