//
// Created by yuyao on 2024/3/12.
//

#include "YYLog.h"
#include <cstdio>
#include <cstdarg>

#ifdef __ANDROID__
#include <android/log.h>
#endif

static YYLog::LogLevel level = YYLog::Verbose;
static std::string appName = "YYDemo";
static std::ofstream logFile;

#ifdef __ANDROID__
static int log_vprintf(int prio, const char *fmt, va_list args)
{
    char buffer[4096];
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    __android_log_write(prio, appName.c_str(), buffer);
    return rc;
}
#else
static int log_vprintf(const char *fmt, va_list args)
{
    char buffer[4096];
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    fwrite(buffer, rc, 1, stdout);
    fflush(stdout);
    return rc;
}
#endif

int YYLog::printf(const char *fmt, ...)
{
#ifdef __ANDROID__
    return 0;
#else
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    fwrite(buffer, rc, 1, stdout);
    fflush(stdout);

    return rc;
#endif
}

void YYLog::SetLevel(LogLevel levelIn)
{
    level = levelIn;
}

void YYLog::SetAppName(const std::string& appNameIn)
{
    appName = appNameIn;
}

void YYLog::V(const char *fmt, ...)
{
    if (level <= YYLog::Verbose)
    {
        YYLog::printf("[VERBOSE] ");
        va_list args;
        va_start(args, fmt);
#ifdef __ANDROID__
        log_vprintf(ANDROID_LOG_VERBOSE, fmt, args);
#else
        log_vprintf(fmt, args);
#endif
        va_end(args);
    }
}

void YYLog::D(const char *fmt, ...)
{
    if (level <= YYLog::Debug)
    {
        YYLog::printf("[DEBUG] ");
        va_list args;
        va_start(args, fmt);
#ifdef __ANDROID__
        log_vprintf(ANDROID_LOG_DEBUG, fmt, args);
#else
        log_vprintf(fmt, args);
        YYLog::printf("\n");
#endif
        va_end(args);
    }
}

void YYLog::I(const char *fmt, ...)
{
    if (level <= YYLog::Info)
    {
        YYLog::printf("[INFO] ");
        va_list args;
        va_start(args, fmt);
#ifdef __ANDROID__
        log_vprintf(ANDROID_LOG_INFO, fmt, args);
#else
        log_vprintf(fmt, args);
#endif
        va_end(args);
    }
}

void YYLog::W(const char *fmt, ...)
{
    if (level <= YYLog::Warning)
    {
        YYLog::printf("[WARNING] ");
        va_list args;
        va_start(args, fmt);
#ifdef __ANDROID__
        log_vprintf(ANDROID_LOG_WARN, fmt, args);
#else
        log_vprintf(fmt, args);
#endif
        va_end(args);
    }
}

void YYLog::E(const char *fmt, ...)
{
    if (level <= YYLog::Error)
    {
        YYLog::printf("[ERROR] ");
        va_list args;
        va_start(args, fmt);
#ifdef __ANDROID__
        log_vprintf(ANDROID_LOG_ERROR, fmt, args);
#else
        log_vprintf(fmt, args);
#endif
        va_end(args);
    }
}

void YYLog::SetLogFile(const std::string& filename) {
    logFile.open(filename, std::ios::out | std::ios::app); // Open file in append mode
}

void YYLog::DumpToFile(const char *fmt, ...) {
    if (logFile.is_open()) {
        va_list args;
        va_start(args, fmt);
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        
        logFile << buffer << std::endl; // Write to file
    }
}