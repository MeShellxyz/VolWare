#include "SerialReader.h"

#include <iostream>
#include <sstream>

SerialReader::SerialReader(const std::string &port, unsigned int baudRate)
    : mSerialPort(mIoService, port), running(false) {
    mSerialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    mSerialPort.set_option(boost::asio::serial_port_base::character_size(8));
    mSerialPort.set_option(boost::asio::serial_port_base::parity(
        boost::asio::serial_port_base::parity::none));
    mSerialPort.set_option(boost::asio::serial_port_base::stop_bits(
        boost::asio::serial_port_base::stop_bits::one));
    mSerialPort.set_option(boost::asio::serial_port_base::flow_control(
        boost::asio::serial_port_base::flow_control::none));
}

SerialReader::~SerialReader() { stop(); }

bool SerialReader::start() {
    if (running) {
        return false;
    }
    try {
        running = true;
        readStart();
        mIoThread = std::thread([this]() { mIoService.run(); });
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error starting SerialReader: " << e.what() << std::endl;
        return false;
    }
}

void SerialReader::stop() {
    if (!running) {
        return;
    }

    running = false;
    mIoService.stop();
    if (mIoThread.joinable()) {
        mIoThread.join();
    }
    if (mSerialPort.is_open()) {
        mSerialPort.close();
    }
}

void SerialReader::setCallback(
    std::function<void(const std::vector<int> &)> callback) {
    mCallback = callback;
}

void SerialReader::readStart() {
    if (!running) {
        return;
    }
    boost::asio::async_read_until(
        mSerialPort, mReadBuffer, '\n',
        std::bind(&SerialReader::readComplete, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void SerialReader::readComplete(const boost::system::error_code &error,
                                size_t bytes_transferred) {
    if (!running) {
        return;
    }
    if (!error) {
        std::istream is(&mReadBuffer);
        std::string line;
        std::getline(is, line);

        std::vector<int> values;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, ',')) {
            try {
                values.push_back(std::stoi(value));
            } catch (const std::invalid_argument &e) {
                std::cerr << "Invalid value: " << value << std::endl;
            }
        }

        if (!values.empty() && mCallback) {
            mCallback(values);
        }

        readStart();
    } else {
        std::cerr << "Error reading from serial port: " << error.message()
                  << std::endl;
        if (running) {
            readStart();
        }
    }
}