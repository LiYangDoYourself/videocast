#ifndef LOGGER_H
#define LOGGER_H
#pragma once
#include <stdint.h>
#include <string>
#include <mutex>

class Logger
{
public:
    static void init();
    static void setDebugSW(int sw);
    static int record(const char *fmt,...);
    static void removeOverstock();

private:
    static int mDebugLevel;
    static std::string mLogDir;
    static std::mutex mMutex;


private:
    Logger()=default;
    ~Logger()=default;

    static void scanDirRemoveExceedFile(const char *dirPath);
};

#endif // LOGGER_H
