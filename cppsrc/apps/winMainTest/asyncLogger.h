#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical
};

struct LogMessage {
    LogLevel level;
    std::string message;
};

class SimpleAsyncSingletonLogger {
public:
    static SimpleAsyncSingletonLogger& getInstance() {
        static SimpleAsyncSingletonLogger instance;
        return instance;
    }

    ~SimpleAsyncSingletonLogger() {
        {
            //std::lock_guard<std::mutex> lock(mutexQueue);
            stop = true;
        }
        cv.notify_one();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void setLogLevel(LogLevel level) {
        currentLogLevel = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level >= currentLogLevel) {
            std::lock_guard<std::mutex> lock(mutexQueue);
            logQueue.push({level, message});
            cv.notify_one();
        }
    }

    void trace(const std::string& message) { log(LogLevel::Trace, message); }
    void debug(const std::string& message) { log(LogLevel::Debug, message); }
    void info(const std::string& message) { log(LogLevel::Info, message); }
    void warn(const std::string& message) { log(LogLevel::Warn, message); }
    void error(const std::string& message) { log(LogLevel::Error, message); }
    void critical(const std::string& message) { log(LogLevel::Critical, message); }

private:
    SimpleAsyncSingletonLogger() : stop(false) {
        workerThread = std::thread(&SimpleAsyncSingletonLogger::workerFunction, this);
    }

    std::queue<LogMessage> logQueue;
    std::mutex mutexQueue;
    std::condition_variable cv;
    std::thread workerThread;
    std::atomic<bool> stop;
    LogLevel currentLogLevel = LogLevel::Info;

    SimpleAsyncSingletonLogger(const SimpleAsyncSingletonLogger&) = delete;
    SimpleAsyncSingletonLogger& operator=(const SimpleAsyncSingletonLogger&) = delete;

    void workerFunction() {
        while (true) {
            LogMessage msg;
            {
                std::unique_lock<std::mutex> lock(mutexQueue);
                cv.wait(lock, [this] { return stop ||!logQueue.empty(); });
                if (stop && logQueue.empty()) {
                    return;
                }
                msg = logQueue.front();
                logQueue.pop();
            }

            std::string levelStr = getLevelString(msg.level);
            std::string timestamp = getCurrentTimestamp();
            std::string formattedMessage = formatMessage(timestamp, levelStr, msg.message);
            std::cout << formattedMessage << std::endl;
        }
    }

    std::string getLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warn: return "WARN";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    std::string getCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm* timeInfo = std::localtime(&now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
        return std::string(buffer);
    }

    std::string formatMessage(const std::string& timestamp, const std::string& level, const std::string& message) {
        std::ostringstream oss;
        oss << "[" << timestamp << "] [" << level << "] " << message;
        return oss.str();
    }
};