/**
 * @file CLogger.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CLogger
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
    if (m_Type == ELogType::File)
        m_Ofs.close();
}

CLogger &CLogger::getInstance()
{
    return getInstanceImpl();
}

void CLogger::setLevel(const ELogLevel level)
{
    m_Level = level;
}

void CLogger::init(ELogLevel level)
{
    getInstanceImpl(&level);
}

CLogger &CLogger::getInstanceImpl(const CLogger::ELogLevel *level)
{
    static CLogger instance(*level);
    return instance;
}

CLogger::CLogger(ELogLevel logLevel)
    : m_Level(logLevel),
      m_Type(CLogger::ELogType::Terminal) {}

void CLogger::setToFile(const string &filePath)
{
    m_Type = ELogType::File;
    m_FilePath = filePath;
    m_Ofs = ofstream(m_FilePath, std::ios_base::app);
}

void CLogger::log(const CLogger::ELogLevel level, const string &msg)
{
    if (level < m_Level)
        return;

    stringstream ss;

    ss << "[";

    if (level == ELogLevel::Error)
        ss << "ERROR";
    else if (level == ELogLevel::Info)
        ss << "INFO";
    else if (level == ELogLevel::Verbose)
        ss << "VERBOSE";

    ss << "] ("
       << getDateTimeNow()
       << "): "
       << msg;

    logToOutput(ss.str());
}

void CLogger::logToOutput(const string &msg)
{
    if (m_Type == CLogger::ELogType::Terminal)
        cout << msg << endl;

    else if (m_Type == CLogger::ELogType::File)
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
