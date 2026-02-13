//
//  Logger.cpp
//  SaplingEngine
//

#include "Core/Logger.hpp"

Logger* Logger::Instance = nullptr;

void Logger::initialize()
{
    if (!Instance)
    {
        Instance = new Logger();
    }
}

void Logger::cleanUp()
{
    if (Instance)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void Logger::setLevel(LogLevel level)
{
    getInstance()->m_minLevel = level;
}

void Logger::debug(const std::string& message)
{
#ifndef DEBUG
    return;
#endif
    if (getInstance()->m_minLevel > LogLevel::Debug) return;
    std::cout << "[DEBUG] " << message << std::endl;
}

void Logger::info(const std::string& message)
{
    if (getInstance()->m_minLevel > LogLevel::Info) return;
    std::cout << "[INFO] " << message << std::endl;
}

void Logger::warn(const std::string& message)
{
    if (getInstance()->m_minLevel > LogLevel::Warn) return;
    std::cout << "[WARN] " << message << std::endl;
}

void Logger::error(const std::string& message)
{
    if (getInstance()->m_minLevel > LogLevel::Error) return;
    std::cerr << "[ERROR] " << message << std::endl;
}
