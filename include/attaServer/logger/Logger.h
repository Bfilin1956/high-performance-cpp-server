//
// Created by r13x on 5/6/26.
//

#ifndef ATTA1_LOGGER_H
#define ATTA1_LOGGER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <fmt/core.h>

#include "LogLevel.h"
#include "BumpAllocator.h"
class Logger {
public:
    ~Logger() {
        delete bufferMsg_[0];
        delete bufferMsg_[1];
    }
    static void start() {
        instance().start_();
    }

    static void stop() {
        instance().stop_();
    }

    template<typename... Args>
    static void log(const LogLevel level, fmt::format_string<Args...> fmt, Args&&... args) {
        auto& logger = instance();
        const auto msgSize = fmt::formatted_size(fmt, std::forward<Args>(args)...);
        const auto time = logger.getCachedTime();
        const auto lvl  = levelToString(level);
        const auto prefixSize = fmt::formatted_size("[{}] [{}] ", time, lvl);
		const auto totalSize = msgSize + prefixSize + 1;
        while (true) {
            const int idx = logger.currentMsg_.load(std::memory_order_acquire);
            logger.currentWrite_[idx].fetch_add(1, std::memory_order_acquire);
						if (idx != logger.currentMsg_.load(std::memory_order_acquire)) {
				logger.currentWrite_[idx].fetch_sub(1, std::memory_order_release);
                continue;
            }
            auto addr = static_cast<char*>(logger.bufferMsg_[idx]->alloc(totalSize));
            if (!addr) {
				logger.currentWrite_[idx].fetch_sub(1, std::memory_order_release);
				return;
			}
			
            auto out = fmt::format_to(addr,"[{}] [{}] ", time, lvl);
            auto out1 = fmt::format_to(out, fmt, std::forward<Args>(args)...);
            addr[totalSize-1] = '\n';

			if (logger.bufferMsg_[idx]->used() > logger.threshold_) logger.cv_.notify_one();
			logger.currentWrite_[idx].fetch_sub(1, std::memory_order_release);
			return;
        }
    }
    template<typename... Args>
    static void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::TRACE, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::WARN, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void fatal(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::FATAL, fmt, std::forward<Args>(args)...);
    }

    static void setBufferSize(const size_t size) {
        auto& logger = instance();
        logger.bufferMsg_[0] = new PollAllocator(size);
        logger.bufferMsg_[1] = new PollAllocator(size);

        if (!logger.bufferMsg_[0]->isValid() || !logger.bufferMsg_[1]->isValid()) {
            perror("setBufferSizeFail");
            exit(-1);
        }
        instance().bufferSize_ = size;
        instance().threshold_ = size * 0.9;
    }
    static void setOutputFilename(const std::string_view filename) {
        auto& logger = instance();
        memcpy(logger.fileName_, filename.data(), filename.size());
        logger.fileName_[filename.size()] = '\0';
        logger.file_ = std::ofstream(logger.fileName_, std::ios::app);
    }
    static void setLevel(const LogLevel level){instance().level_ = level;}
private:
    void timeWatcher() {
        int next{}, countFlush{};
        while (!exit_) {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            std::tm tm{};

            next = 1 - currentTime_.load(std::memory_order_relaxed);
            localtime_r(&time_t_now, &tm);
            fmt::format_to_n(bufferTime_[next], sizeof(bufferTime_[next]), "{:02}.{:02}.{} {:02}:{:02}:{:02}.{:03}\0",
                                              tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
                                              tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count());
            currentTime_.store(next, std::memory_order_release);

            ++countFlush;
            if(countFlush == 5) {
                file_.flush();
                countFlush = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    void start_() {
        thread_ = std::thread(&Logger::loop, &instance());
        timeThread_ = std::thread(&Logger::timeWatcher, &instance());
    }
    void stop_() {
        exit_ = true;
        cv_.notify_one();
        if (thread_.joinable()) thread_.join();
        if (timeThread_.joinable()) timeThread_.join();
    }
    static Logger &instance() {
        static Logger inst;
        return inst;
    }
    void loop() {
        int old{};
        while (!exit_) {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this] {
                    return !bufferMsg_[0]->empty() ||!bufferMsg_[1]->empty()|| exit_;
                });
                old = currentMsg_.load(std::memory_order_acquire);
                currentMsg_.store(1-old, std::memory_order_release);
            }
            while (true) {
                if (currentWrite_[old].load(std::memory_order_acquire) == 0) break;
                for (int i = 0; i < 100; ++i) __asm__ ( "pause;" );
                std::this_thread::yield();
            }
            if (!bufferMsg_[old]->empty())
                file_.write(bufferMsg_[old]->getAddrChar(), static_cast<ssize_t>(bufferMsg_[old]->size()));

            bufferMsg_[old]->reset();
        }
    }
    static constexpr std::string_view levelToString(const LogLevel level) {
        switch(level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
    std::string_view getCachedTime() const {
        const int idx = currentTime_.load(std::memory_order_acquire);
        return bufferTime_[idx];
    }


    //нужно отсортировать
    char bufferTime_[2][64]{};
    std::atomic<int> currentTime_{};

    PollAllocator* bufferMsg_[2]{};
    std::atomic<int> currentMsg_{};
    std::atomic<int> currentWrite_[2]{};

    std::mutex mtx_;
    std::ofstream file_;
    char fileName_[1024]{};
    std::thread thread_;
    std::thread timeThread_;
    size_t threshold_{};
    size_t bufferSize_{};
    size_t charCount{};

    std::condition_variable cv_;
    bool exit_{false};
    char intBuffer_[32]{};
    LogLevel level_{LogLevel::INFO};
};


#endif //ATTA1_LOGGER_H