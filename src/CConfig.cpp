/**
 * @file CConfig.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CConfig
 *
 */

#include "CConfig.h"
#include "Utils.h"
#include "CLogger.h"

#include <utility> // pair<>

// using namespace std;
using std::string, std::stringstream, std::cout, std::endl, std::pair;

CConfig::CConfig()
{
    (*this)["url"] = string("");
    (*this)["depth"] = 1;
    (*this)["remote_images"] = false;
    (*this)["remote"] = false;
    (*this)["error_page"] = false;
    (*this)["output"] = string("./output");
    (*this)["log_level"] = 1;
    (*this)["log_file"] = string("");
    (*this)["cookies"] = string("");
    (*this)["user_agent"] = string("WGET-Project/0.1 (FIT CVUT, Jan Cerny <cernyj87@fit.cvut.cz>)");
    (*this)["advertisement"] = true;
    (*this)["limit"] = string("");
    (*this)["cert_store"] = string("");
}

string CConfig::formatOption(size_t paramSize, const string &args, const string &helpText) const
{
    stringstream ss;

    ss << "\t" << std::setw(paramSize) << std::left
       << args
       << "\t"
       << helpText
       << "\n";

    return ss.str();
}

void CConfig::printHelp(const string &programName) const
{
    const size_t paramSize = 35;

    stringstream ss;
    cout << "\nUSAGE:\n";

    cout << "\t" << programName << " <URL Address> [OPTIONS]\n\n";
    cout << "\t"
         << "Wget Clone to download mirror of the specified page, replacing links to local relative links.\n\n";

    cout << "\nOPTIONS:\n";

    cout << formatOption(paramSize,
                         "-h, --help",
                         "Print this help");

    cout << formatOption(paramSize,
                         "-o, --output <path>",
                         "Set output path (default = ./output)");

    cout << formatOption(paramSize,
                         "-d, --depth <int>",
                         "Set max recursive depth (default = 1)");

    cout << formatOption(paramSize,
                         "-l, --limit <domain,domain,...>",
                         "Limit download only to selected domains (comma separated list) (default = \"\"; downloads anything)");

    cout << formatOption(paramSize,
                         "-r, --remote",
                         "Don't download any external links, keep the original remote link");

    cout << formatOption(paramSize,
                         "-R, --remote-images",
                         "Don't download external images, keep the original remote link");

    cout << formatOption(paramSize,
                         "-e, --error-page",
                         "Replace links to not-downloaded pages with a 404 error page");

    cout << formatOption(paramSize,
                         "-v, --verbose",
                         "Print all possible logs");

    cout << formatOption(paramSize,
                         "-q, --quiet",
                         "Print only errors");

    cout << formatOption(paramSize,
                         "-L, --log-file",
                         "Send log output to file instead of terminal");

    cout << formatOption(paramSize,
                         "-c, --cookie <cookie>",
                         "Set Cookie header to this value");

    cout << formatOption(paramSize,
                         "-u, --user-agent <useragent>",
                         "Set User-Agent header to this value");

    cout << formatOption(paramSize,
                         "--cert-store <path>",
                         "Path to user defined keys bundle .crt file (eg. \"./assets/certs/ca-certificates.crt\") (default = \"\"; uses system store)");

    cout << formatOption(paramSize,
                         "--disable-annoying-advertisement-that-nobody-wants-to-see",
                         "Self explanatory :)");

    cout << endl;
}

bool CConfig::parseArgs(int argc, char const *argv[])
{
    auto &logger = CLogger::getInstance();

    // Check args count
    if (argc < 2)
    {
        logger.log(CLogger::ELogLevel::Error, "Too few arguments!");
        printHelp(argv[0]);
        return false;
    }

    // Loop through all remaining arguments
    for (int i = 1; i < argc; i++)
    {
        string value = argv[i];

        if (value == "-h" || value == "--help")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: Read Help argument");
            printHelp(argv[0]);
            return false;
        }

        else if (value == "-d" || value == "--depth")
        {
            if (++i >= argc)
                return false;

            value = argv[i];

            if (value.find_first_not_of("0123456789") != string::npos)
            {
                logger.log(CLogger::ELogLevel::Error, "Depth value is not a valid number!");
                return false;
            }

            logger.log(CLogger::ELogLevel::Verbose, "Config: depth = " + value);
            (*this)["depth"] = value;
        }

        else if (value == "-l" || value == "--limit")
        {
            if (!setWithNext("limit", i, argc, argv))
                return false;
        }

        else if (value == "-r" || value == "--remote")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: remote = true");
            (*this)["remote"] = true;
        }

        else if (value == "-R" || value == "--remote-images")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: remote_images = true");
            (*this)["remote_images"] = true;
        }

        else if (value == "-e" || value == "--error-page")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: error_page = true");
            (*this)["error_page"] = true;
        }

        else if (value == "-o" || value == "--output")
        {
            if (!setWithNext("output", i, argc, argv))
                return false;
        }

        else if (value == "-v" || value == "--verbose")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: log_level = verbose");
            (*this)["log_level"] = 0;

            logger.setLevel(CLogger::ELogLevel::Verbose);
        }

        else if (value == "-q" || value == "--quiet")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: log_level = error");
            (*this)["log_level"] = 2;

            logger.setLevel(CLogger::ELogLevel::Error);
        }

        else if (value == "-L" || value == "--log-file")
        {
            if (!setWithNext("log_file", i, argc, argv))
                return false;

            logger.setToFile((*this)["log_file"]);
        }

        else if (value == "-c" || value == "--cookie")
        {
            if (!setWithNext("cookies", i, argc, argv))
                return false;
        }

        else if (value == "-u" || value == "--user-agent")
        {
            if (!setWithNext("user_agent", i, argc, argv))
                return false;
        }

        else if (value == "--cert-store")
        {
            if (!setWithNext("cert_store", i, argc, argv))
                return false;
        }

        else if (value == "--disable-annoying-advertisement-that-nobody-wants-to-see")
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: advertisement = false");
            (*this)["advertisement"] = false;
        }

        else if (value != "" && !Utils::startsWith(value, "-"))
        {
            logger.log(CLogger::ELogLevel::Verbose, "Config: url = " + value);
            (*this)["url"] = value;
        }

        else
        {
            logger.log(CLogger::ELogLevel::Error, "Unknown argument: " + value);
            return false;
        }
    }

    if ((string)((*this)["url"]) == "")
    {
        logger.log(CLogger::ELogLevel::Error, "No URL provided!");
        return false;
    }

    return true;
}

bool CConfig::setWithNext(const string &configName, int &currentArg, int argc, const char *argv[])
{
    if (++currentArg >= argc)
        return false;

    string value = argv[currentArg];

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Config: " + configName + " = " + value);
    (*this)[configName] = value;

    return true;
}

CConfig &CConfig::getInstance()
{
    static CConfig instance;
    return instance;
}

CConfig::TSetting &CConfig::operator[](const string &key)
{
    // If key already exists, return setting
    if (m_Settings.find(key) != m_Settings.end())
        return m_Settings.find(key)->second;

    // Otherwise insert new empty setting
    m_Settings.insert(pair<string, TSetting>(key, TSetting("")));
    return m_Settings.find(key)->second;
}

CConfig::TSetting::TSetting() = default;

CConfig::TSetting::TSetting(const string &value)
    : m_Value(value) {}

// Get values
CConfig::TSetting::operator bool() const
{
    return (m_Value == "true") ? true : false;
}

CConfig::TSetting::operator int() const
{
    return stoi(m_Value);
}

CConfig::TSetting::operator string() const
{
    return m_Value;
}

// Set values
CConfig::TSetting &CConfig::TSetting::operator=(bool value)
{
    m_Value = value ? "true" : "false";
    return *this;
}

CConfig::TSetting &CConfig::TSetting::operator=(int value)
{
    m_Value = std::to_string(value);
    return *this;
}

CConfig::TSetting &CConfig::TSetting::operator=(const string &value)
{
    m_Value = value;
    return *this;
}
