/**
 * @file CLogger.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CLogger
 *
 */

#pragma once

#include <iostream>
#include <string>
#include <fstream>

using std::string, std::ofstream;

/**
 * @brief Logger singleton class that provides various logging levels and basic interface to log messages to output or log file
 *
 */
class CLogger
{
public:
    /**
     * @brief Log levels
     *
     */
    enum class ELogLevel
    {
        Verbose,
        Info,
        Error
    };

    /**
     * @brief Disabled constructor because Logger without set level is not allowed
     *
     */
    CLogger() = delete;

    /**
     * @brief Destroy the CLogger object, close ofstream if needed
     *
     */
    ~CLogger();

    /**
     * @brief Construct a new CLogger object, with logging to 'cout'
     *
     * @param logLevel Minimal level to log
     */
    explicit CLogger(ELogLevel logLevel);

    /**
     * @brief Set the logger to send next logs to file instead of terminal
     *
     * @param filePath Path to the log file
     */
    void setToFile(const string &filePath);

    /**
     * @brief Log a message
     *
     * @param level ELogLevel level of this message
     * @param msg The message
     */
    void log(const ELogLevel level, const string &msg);

    /**
     * @brief Set current log level
     *
     * @param level
     */
    void setLevel(const ELogLevel level);

    // Singleton stuff
    /**
     * @brief Init a new singleton instance of CLogger with desired level
     *
     * @param level
     */
    static void init(const ELogLevel level);

    /**
     * @brief Get the the singleton instance
     *
     * @return CLogger&
     */
    static CLogger &getInstance();

    /**
     * @brief Disabled copy constructor because of CLogger being singleton
     *
     */
    CLogger(const CLogger &) = delete;

    /**
     * @brief Disabled operator= because of CLogger being singleton
     *
     */
    void operator=(const CLogger &) = delete;

private:
    static CLogger &getInstanceImpl(const ELogLevel *level = nullptr);

    /**
     * @brief Type of the current Logger
     *
     */
    enum class ELogType
    {
        Terminal,
        File
    };

    ELogLevel m_Level;
    ELogType m_Type;
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
