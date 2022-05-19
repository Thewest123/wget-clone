#pragma once

/**
 * @file CLogger.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <iostream>

using namespace std;

class CLogger
{
public:
    enum class LogLevel
    {
        Verbose,
        Info,
        Error
    };

    CLogger() = delete;

    CLogger(LogLevel logLevel);

    CLogger(LogLevel logLevel, const string &filePath);

    /**
     * @brief Log message in VERBOSE level
     *
     * @param msg
     */
    void log(const LogLevel level, const string &msg) const;

    // Singleton stuff
    static void init(const LogLevel level);
    static CLogger &getInstance();

    CLogger(const CLogger &)  = delete;
    void operator=(const CLogger &) = delete;

private:

    static CLogger &getInstanceImpl(const LogLevel* level = nullptr);

    enum class LogType
    {
        Terminal,
        File
    };

    LogLevel m_Level;
    LogType m_Type;
    string m_FilePath;

    /**
     * @brief Print the message to COUT or FILE
     *
     * @param msg
     */
    void logToOutput(const string &msg) const;

    string getDateTimeNow() const;
};