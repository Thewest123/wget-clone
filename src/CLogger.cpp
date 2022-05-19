/**
 * @file CLogger.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <iostream>

#include "CLogger.h"

using namespace std;


CLogger &CLogger::getInstance()
{
    return getInstanceImpl();
}

void CLogger::init(LogLevel level)
{
    getInstanceImpl(&level);
}

CLogger& CLogger::getInstanceImpl(const CLogger::LogLevel* level)
{
    static CLogger instance(*level);
    return instance;
}

CLogger::CLogger(LogLevel logLevel)
    : m_Level(logLevel),
      m_Type(CLogger::LogType::Terminal){};

CLogger::CLogger(LogLevel logLevel, const string &filePath)
    : m_Level(logLevel),
      m_Type(CLogger::LogType::File),
      m_FilePath(filePath){};

/**
 * @brief Log message in VERBOSE level
 *
 * @param msg
 */
void CLogger::log(const CLogger::LogLevel level, const string &msg) const
{
    if (level < m_Level)
        return;

    string levelName;

    if (level == LogLevel::Error)        levelName = "ERROR";
    else if (level == LogLevel::Info)    levelName = "INFO";
    else if (level == LogLevel::Verbose) levelName = "VERBOSE";

    logToOutput("[" + levelName + "] (" + getDateTimeNow() + "): " + msg);
}

string m_FilePath;

/**
 * @brief Print the message to COUT or FILE
 *
 * @param msg
 */
void CLogger::logToOutput(const string &msg) const
{
    if (m_Type == CLogger::LogType::Terminal)
    {
        cout << msg << endl;
        return;
    }

    if (m_Type == CLogger::LogType::File)
    {
        // todo
        return;
    }
}

string CLogger::getDateTimeNow() const
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}