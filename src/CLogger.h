/**
 * @file CLogger.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Logger singleton class that provides various logging levels and basic interface to log messages to output or log file
 *
 */

#pragma once

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

    /**
     * @brief Construct a new CLogger object, with logging to 'cout'
     *
     * @param logLevel Minimal level to log
     */
    CLogger(LogLevel logLevel);

    /**
     * @brief Construct a new CLogger object, with logging to a file
     *
     * @param logLevel Minimal level to log
     * @param filePath Path to the log file
     */
    CLogger(LogLevel logLevel, const string &filePath);

    /**
     * @brief Log a message
     *
     * @param level LogLevel level of this message
     * @param msg The message
     */
    void log(const LogLevel level, const string &msg) const;

    /**
     * @brief Set current log level
     *
     * @param level
     */
    void setLevel(const LogLevel level);

    // Singleton stuff
    /**
     * @brief Init a new singleton instance of CLogger with desired level
     *
     * @param level
     */
    static void init(const LogLevel level);

    /**
     * @brief Get the the singleton instance
     *
     * @return CLogger&
     */
    static CLogger &getInstance();

    CLogger(const CLogger &) = delete;
    void operator=(const CLogger &) = delete;

private:
    static CLogger &getInstanceImpl(const LogLevel *level = nullptr);

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

    /**
     * @brief Return formatted string with current date and time
     *
     * @return string
     */
    string getDateTimeNow() const;
};