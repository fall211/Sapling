//
//  Logger.hpp
//  SaplingEngine
//

#pragma once

#include <string>
#include <iostream>

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
    static Logger* Instance;
    LogLevel m_minLevel = LogLevel::Debug;

    Logger() = default;
    ~Logger() = default;

public:
    static void initialize();
    static void cleanUp();

    static Logger* getInstance()
    {
        if (Instance == nullptr) {
            Instance = new Logger();
        }
        return Instance;
    }

    static void setLevel(LogLevel level);

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
};
