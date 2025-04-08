#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

typedef std::function<void(const std::vector<int> &)> SerialInputCallback;

class SerialReader {
private:
    const unsigned int RECONNECT_INTERVAL_MS = 1000;
    const unsigned int SYNC_INTERVAL_MS = 1000;

    bool openPort();
    bool closePort();

    void sendMessage(const std::string &message,
                     std::function<void(bool success)> callback = nullptr);
    void sendSyncMessage();
    void scheduleSyncTimer();

    void readStart();
    void readComplete(const boost::system::error_code &error,
                      size_t bytes_transferred);

    void workerThread();

    std::string mPortName;
    unsigned int mBaudRate;
    SerialInputCallback mCallback;
    std::string mSyncMessage;

    boost::asio::io_service mIoService;
    boost::asio::serial_port mSerialPort;
    boost::asio::streambuf mReadBuffer;
    std::unique_ptr<boost::asio::steady_timer> mSyncTimer;

    std::jthread mWorkerThread;

    std::atomic<bool> mRunning;
    std::atomic<bool> mConnected;

public:
    SerialReader(const std::string &port, unsigned int baudRate);
    ~SerialReader();

    bool start();
    void stop();

    bool isConnected() const { return mConnected; }

    void setCallback(std::function<void(const std::vector<int> &)> callback) {
        mCallback = callback;
    };
    void setSyncMessage(const std::string &syncMsg) { mSyncMessage = syncMsg; }
};