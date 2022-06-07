/**
 * @file CLogger.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Logger singleton class that provides various logging levels and basic interface to log messages to output or log file
 *
 */

#pragma once

#include <iostream>
#include <string>
#include <fstream>

using std::string, std::ofstream;

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

    ~CLogger();

    /**
     * @brief Construct a new CLogger object, with logging to 'cout'
     *
     * @param logLevel Minimal level to log
     */
    explicit CLogger(LogLevel logLevel);

    /**
     * @brief Set the logger to send next logs to file instead of terminal
     *
     * @param filePath Path to the log file
     */
    void setToFile(const string &filePath);

    /**
     * @brief Log a message
     *
     * @param level LogLevel level of this message
     * @param msg The message
     */
    void log(const LogLevel level, const string &msg);

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
    ofstream m_Ofs;

    /**
     * @brief Print the message to COUT or FILE
     *
     * @param msg
     */
    void logToOutput(const string &msg);

    /**
     * @brief Return formatted string with current date and time
     *
     * @return string
     */
    string getDateTimeNow() const;
};
