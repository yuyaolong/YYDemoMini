//
// Created by yuyao on 2024/3/12.
//

#pragma once
#include <string>
#include <fstream>

class YYLog {
public:
    enum LogLevel
    {
        Verbose = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4
    };

    static void SetLevel(LogLevel levelIn);
    static void SetAppName(const std::string& appNameIn);

    // dump a string to file
    static void DumpToFile(const char *fmt, ...);
    static void SetLogFile(const std::string& filename);

    // verbose
    static void V(const char *fmt, ...);

    // debug
    static void D(const char *fmt, ...);

    // info
    static void I(const char *fmt, ...);

    // warning
    static void W(const char *fmt, ...);

    // error
    static void E(const char *fmt, ...);

    YYLog() = delete;
    YYLog(const YYLog&) = delete;
    YYLog& operator=(const YYLog&) = delete;

private:
    static int printf(const char *fmt, ...);

};

