/**
 * @file CConfig.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Config singleton class to parse, store, and provide config values to other parts of the program
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
    (*this)["cookies"] = string("");
    (*this)["user_agent"] = string("WGET-Project/0.1 (FIT CVUT, Jan Cerny <cernyj87@fit.cvut.cz>)");
    (*this)["advertisement"] = true;
    (*this)["limit"] = string("");
    (*this)["cert_store"] = string("");
};

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
                         "-c, --cookie <cookie>",
                         "Set Cookie header to this value");

    cout << formatOption(paramSize,
                         "-u, --user-agent <useragent>",
                         "Set User-Agent header to this value");

    cout << formatOption(paramSize,
                         "--cert-store <path>",
                         "Path to user defined certificate keys store (default = \"\"; uses system store)");

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
        logger.log(CLogger::LogLevel::Error, "Too few arguments!");
        printHelp(argv[0]);
        return false;
    }

    // Loop through all remaining arguments
    for (int i = 1; i < argc; i++)
    {
        string value = argv[i];

        if (value == "-h" || value == "--help")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: Read Help argument");
            printHelp(argv[0]);
            return false;
        }

        else if (value == "-d" || value == "--depth")
        {
            value = argv[++i];

            if (value.find_first_not_of("0123456789") != string::npos)
            {
                logger.log(CLogger::LogLevel::Error, "Depth value is not a valid number!");
                return false;
            }

            logger.log(CLogger::LogLevel::Verbose, "Config: depth = " + value);
            (*this)["depth"] = value;
        }

        else if (value == "-l" || value == "--limit")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: limit = " + value);
            (*this)["limit"] = value;
        }

        else if (value == "-r" || value == "--remote")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: remote = true");
            (*this)["remote"] = true;
        }

        else if (value == "-R" || value == "--remote-images")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: remote_images = true");
            (*this)["remote_images"] = true;
        }

        else if (value == "-e" || value == "--error-page")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: error_page = true");
            (*this)["error_page"] = true;
        }

        else if (value == "-o" || value == "--output")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: output = " + value);
            (*this)["output"] = value;
        }

        else if (value == "-v" || value == "--verbose")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: log_level = verbose");
            (*this)["log_level"] = 0;
            logger.setLevel(CLogger::LogLevel::Verbose);
        }

        else if (value == "-q" || value == "--quiet")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: log_level = error");
            (*this)["log_level"] = 2;
            logger.setLevel(CLogger::LogLevel::Error);
        }

        else if (value == "-c" || value == "--cookie")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: cookies = " + value);
            (*this)["cookies"] = value;
        }

        else if (value == "-u" || value == "--user-agent")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: user_agent = " + value);
            (*this)["user_agent"] = value;
        }

        else if (value == "--cert-store")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: cert_store = " + value);
            (*this)["cert_store"] = value;
        }

        else if (value == "--disable-annoying-advertisement-that-nobody-wants-to-see")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: advertisement = false");
            (*this)["advertisement"] = false;
        }

        else if (value != "" && !Utils::startsWith(value, "-"))
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: url = " + value);
            (*this)["url"] = value;
        }

        else
        {
            logger.log(CLogger::LogLevel::Error, "Unknown argument: " + value);
            return false;
        }
    }

    if ((string)((*this)["url"]) == "")
    {
        logger.log(CLogger::LogLevel::Error, "No URL provided!");
        return false;
    }

    cout << "URL: " << ((string)((*this)["url"])) << endl;
    return true;
}

CConfig &CConfig::getInstance()
{
    static CConfig instance;
    return instance;
}

CConfig::Setting &CConfig::operator[](const string &key)
{
    // If key already exists, return setting
    if (m_Settings.find(key) != m_Settings.end())
        return m_Settings.find(key)->second;

    // Otherwise insert new empty setting
    m_Settings.insert(pair<string, Setting>(key, Setting("")));
    return m_Settings.find(key)->second;
}

CConfig::Setting::Setting() = default;

CConfig::Setting::Setting(const string &value)
    : m_Value(value) {}

// Get values
CConfig::Setting::operator bool() const
{
    return (m_Value == "true") ? true : false;
}

CConfig::Setting::operator int() const
{
    return stoi(m_Value);
}

CConfig::Setting::operator string() const
{
    return m_Value;
}

// Set values
CConfig::Setting &CConfig::Setting::operator=(bool value)
{
    m_Value = value ? "true" : "false";
    return *this;
}

CConfig::Setting &CConfig::Setting::operator=(int value)
{
    m_Value = std::to_string(value);
    return *this;
}

CConfig::Setting &CConfig::Setting::operator=(const string &value)
{
    m_Value = value;
    return *this;
}
