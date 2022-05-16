/**
 * @file CLogger.cpp
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

    CLogger() = default;

    CLogger(LogLevel logLevel)
        : m_Level(logLevel){};

    CLogger(LogLevel logLevel, const string &filePath)
        : m_Level(logLevel),
          m_Type(LogType::File),
          m_FilePath(filePath){};

    /**
     * @brief Log message in VERBOSE level
     *
     * @param msg
     */
    void logVerbose(const string &msg) const
    {
        if (m_Level > LogLevel::Verbose)
            return;

        logToOutput("[VERBOSE] " + msg);
    }

    /**
     * @brief Log message in INFO level
     *
     * @param msg
     */
    void logInfo(const string &msg) const
    {
        if (m_Level > LogLevel::Info)
            return;

        logToOutput("[INFO] " + msg);
    }

    /**
     * @brief Log message in ERROR level
     *
     * @param msg
     */
    void logError(const string &msg) const
    {
        if (m_Level > LogLevel::Error)
            return;

        logToOutput("[ERROR] " + msg);
    }

private:
    enum class LogType
    {
        Terminal,
        File
    };

    LogLevel m_Level = LogLevel::Info;
    LogType m_Type = LogType::Terminal;
    string m_FilePath;

    /**
     * @brief Print the message to COUT or FILE
     *
     * @param msg
     */
    void logToOutput(const string &msg) const
    {
        if (m_Type == LogType::Terminal)
        {
            cout << msg;
            return;
        }

        if (m_Type == LogType::File)
        {
            // todo
            return;
        }
    }
};