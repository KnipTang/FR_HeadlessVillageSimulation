#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <filesystem>


#ifdef _WIN32
#define SAFE_LOCALTIME(time, tm) localtime_s(tm, time)
#else
#define SAFE_LOCALTIME(time, tm) localtime_r(time, tm)
#endif

#define FILE_LOG(msg) FileLogger::getInstance().log(msg)

class FileLogger {
public:
    static FileLogger& getInstance() {
        static FileLogger instance;
        return instance;
    }

    // Initialize logger with file path and optional configuration
    void init(const std::string& filename,
        size_t bufferSize = 1024 * 1024,  // 1MB buffer
        bool async = true) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_filename = m_foldername + filename;
        m_bufferSize = bufferSize;
        m_async = async;

        try {
            std::filesystem::path filePath(m_filename);
            std::filesystem::path parentPath = filePath.parent_path();

            if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
                std::filesystem::create_directories(parentPath);
            }
        }
        catch (const std::exception&) {
            // If directory creation fails, we'll still try to open the file
            // The file open will fail if the directory doesn't exist
        }

        if (m_fileStream.is_open()) {
            m_fileStream.close();
        }

        m_fileStream.open(m_filename, std::ios::out | std::ios::app);
        if (!m_fileStream.is_open()) {
            throw std::runtime_error("Failed to open log file: " + m_filename);
        }

        if (m_async && !m_stopThread) {
            startAsyncThread();
        }
    }

    void log(const std::string& message) {
        std::string formattedMessage = formatMessage(message);

        if (m_async) {
            // Async logging
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queue.push(formattedMessage);
            lock.unlock();
            m_cv.notify_one();
        }
        else {
            // Sync logging
            std::lock_guard<std::mutex> lock(m_mutex);
            writeToFile(formattedMessage);
            flushIfNeeded();
        }
    }

void flush() {
        if (m_async) {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            while (!m_queue.empty()) {
                writeToFile(m_queue.front());
                m_queue.pop();
            }
            if (m_fileStream.is_open()) {
                m_fileStream.flush();
            }
        } else {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_fileStream.is_open()) {
                m_fileStream.flush();
            }
        }
    }


    ~FileLogger() {
        if (m_async) {
            m_stopThread = true;
            m_cv.notify_all();
            if (m_logThread && m_logThread->joinable()) {
                m_logThread->join();
            }
        }

        if (m_fileStream.is_open()) {
            flush();
            m_fileStream.close();
        }
    }

    // Disable copy and move
    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;
    FileLogger(FileLogger&&) = delete;
    FileLogger& operator=(FileLogger&&) = delete;

private:
    FileLogger() : m_stopThread(false) { deleteAllFilesInDirectory(); }

    std::string formatMessage(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        struct tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &now_time_t);
#else
        localtime_r(&now_time_t, &tm_buf);
#endif

        localtime_s(&tm_buf, &now_time_t);

        std::stringstream ss;
        ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << "\t";
        ss << message;

        return ss.str();
    }

    void writeToFile(const std::string& message) {
        if (m_fileStream.is_open()) {
            m_fileStream << message << std::endl;
            m_currentBufferSize += message.size() + 1;
        }
        else {
            // Try to reopen if file was closed
            try {
                m_fileStream.open(m_filename, std::ios::out | std::ios::app);
                if (m_fileStream.is_open()) {
                    m_fileStream << message << std::endl;
                    m_currentBufferSize += message.size() + 1;
                }
            }
            catch (...) {
                // Silently fail - we don't want logging to crash the application
            }
        }
    }

    void flushIfNeeded() {
        if (m_currentBufferSize >= m_bufferSize) {
            if (m_fileStream.is_open()) {
                m_fileStream.flush();
            }
            m_currentBufferSize = 0;
        }
    }

    void startAsyncThread() {
        m_logThread = std::make_unique<std::thread>([this]() {
            while (!m_stopThread) {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_cv.wait_for(lock, std::chrono::milliseconds(100),
                    [this]() { return !m_queue.empty() || m_stopThread; });

                std::queue<std::string> localQueue;
                std::swap(localQueue, m_queue);
                lock.unlock();

                if (!localQueue.empty()) {
                    std::lock_guard<std::mutex> fileLock(m_mutex);
                    while (!localQueue.empty()) {
                        writeToFile(localQueue.front());
                        localQueue.pop();
                    }
                    flushIfNeeded();
                }
            }

            // Final flush on shutdown
            std::lock_guard<std::mutex> lock(m_mutex);
            flush();
            });
    }


    void deleteAllFilesInDirectory() {
        try {
            std::filesystem::path dirPath(m_foldername);

            if (!std::filesystem::exists(dirPath)) {
                return;
            }

            if (!std::filesystem::is_directory(dirPath)) {
                return;
            }

            // Delete all files in the directory
            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                if (std::filesystem::is_regular_file(entry.path())) {
                    std::filesystem::remove(entry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& ) {
        }
        catch (const std::exception& ) {
        }
    }


    // Members
    std::string m_foldername = "Log/";
    std::string m_filename;
    std::ofstream m_fileStream;
    std::mutex m_mutex;
    size_t m_bufferSize = 1024 * 1024;
    size_t m_currentBufferSize = 0;
    bool m_async = true;

    // Async logging members
    std::unique_ptr<std::thread> m_logThread;
    std::queue<std::string> m_queue;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stopThread;
};