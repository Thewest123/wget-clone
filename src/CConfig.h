/**
 * @file CConfig.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdio.h>

#include "CLogger.h"

using namespace std;

class CConfig
{
public:
    enum class ConfigName
    {
        Depth,
        ReplaceImageURL,
        ReplaceDepthURL,
        HttpHeader,
        OutputPath,
        LogLevel
    };

    /**
     * @brief Get the CConfig value
     *
     * @tparam T
     * @param name
     * @return T
     */
    template <class T>
    T get(ConfigName name)
    {
    }

    /**
     * @brief Parse CLI arguments to CConfig values
     *
     * @param argc
     * @param argv
     */
    void parseArgs(int argc, char const *argv[])
    {
    }

private:
    size_t m_Depth = 1;
    bool m_ReplaceImageURL = true;
    bool m_ReplaceDepthURL = true;
    string m_HttpHeader = "";
    string m_OutputPath = "";
    CLogger::LogLevel m_LogLevel = CLogger::LogLevel::Info;
};