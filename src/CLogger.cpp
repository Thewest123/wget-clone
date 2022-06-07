/**
 * @file CLogger.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Logger singleton class that provides various logging levels and basic interface to log messages to output or log file
 *
 */

#include "CLogger.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using std::string, std::cout, std::endl, std::ofstream, std::stringstream;

CLogger::~CLogger()
{
    if (m_Type == LogType::File)
        m_Ofs.close();
}

CLogger &CLogger::getInstance()
{
    return getInstanceImpl();
}

void CLogger::setLevel(const LogLevel level)
{
    m_Level = level;
}

void CLogger::init(LogLevel level)
{
    getInstanceImpl(&level);
}

CLogger &CLogger::getInstanceImpl(const CLogger::LogLevel *level)
{
    static CLogger instance(*level);
    return instance;
}

CLogger::CLogger(LogLevel logLevel)
    : m_Level(logLevel),
      m_Type(CLogger::LogType::Terminal) {}

void CLogger::setToFile(const string &filePath)
{
    m_Type = LogType::File;
    m_FilePath = filePath;
    m_Ofs = ofstream(m_FilePath, std::ios_base::app);
}

/**
 * @brief Log message in depending on level
 *
 * @param msg
 */
void CLogger::log(const CLogger::LogLevel level, const string &msg)
{
    if (level < m_Level)
        return;

    stringstream ss;

    ss << "[";

    if (level == LogLevel::Error)
        ss << "ERROR";
    else if (level == LogLevel::Info)
        ss << "INFO";
    else if (level == LogLevel::Verbose)
        ss << "VERBOSE";

    ss << "] ("
       << getDateTimeNow()
       << "): "
       << msg;

    logToOutput(ss.str());
}

/**
 * @brief Print the message to COUT or FILE
 *
 * @param msg
 */
void CLogger::logToOutput(const string &msg)
{
    if (m_Type == CLogger::LogType::Terminal)
        cout << msg << endl;

    else if (m_Type == CLogger::LogType::File)
        m_Ofs << msg << endl;

    return;
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
