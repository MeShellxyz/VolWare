#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <string>
#include <thread>
#include <vector>

class SerialReader {
private:
    boost::asio::io_service mIoService;
    boost::asio::serial_port mSerialPort;
    std::thread mIoThread;
    bool mRunning;

    void readStart();
    void readComplete(const boost::system::error_code &error,
                      size_t bytes_transferred);

    boost::asio::streambuf mReadBuffer;
    std::function<void(const std::string &)> mCallback;

public:
    SerialReader(const std::string &port, unsigned int baudRate);
    ~SerialReader();

    bool start();
    void stop();

    void setCallback(std::function<void(const std::vector<int> &)> callback);
}