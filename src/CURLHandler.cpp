#include "CURLHandler.h"

CURLHandler::CURLHandler(const string &url, bool isExternal)
    : m_IsExternal(isExternal)
{
    regex re("((?:http://|https://)?[^/]*)/?(.*)");
    smatch result;

    if (!regex_match(url, result, re))
    {
        cout << "CURLHandler ERROR: Failed regex!" << endl;
        //! throw
        return;
    }

    setDomain(result[1].str());
    addPath(result[2].str());
}

CURLHandler::~CURLHandler()
{
}

void CURLHandler::setDomain(const string &urlDomain)
{
    // Transform the input to all lower case
    string tmpUrl = Utils::toLowerCase(urlDomain);

    // Check domain starts with https:// and remove it
    if (tmpUrl.rfind("https://", 0) == 0)
    {
        m_IsHttps = true;
        tmpUrl = tmpUrl.substr(string("https://").size(), tmpUrl.length());
    }

    // Check domain starts with http:// and remove it
    else if (tmpUrl.rfind("http://", 0) == 0)
    {
        m_IsHttps = false;
        tmpUrl = tmpUrl.substr(string("http://").size(), tmpUrl.length());
    }

    // Remove trailing slash if present
    if (tmpUrl.rfind("/") == tmpUrl.size() - string("/").size())
    {
        tmpUrl = tmpUrl.substr(0, tmpUrl.size() - 1);
    }

    m_Domain = tmpUrl;
}

void CURLHandler::addPath(const string &path)
{
    // cout << "Adding path: " << path << endl;
    string delimiter = "/";

    // If path starts with /, it's relative to the base domain
    if (Utils::startsWith(path, delimiter))
        m_PathLevels.clear();

    // If the path doesn't end with trailing slash, don't add it later with normalization
    if (!Utils::endsWith(path, delimiter))
        m_HasTrailingSlash = false;

    auto start = 0U;
    auto end = path.find(delimiter);

    while (end != string::npos)
    {
        // Get current level
        string level = path.substr(start, end - start);

        // Add level to vector
        m_PathLevels.push_back(level);

        // Get new delimiter position
        start = end + delimiter.length();
        end = path.find(delimiter, start);
    }

    // Add also the last level to vector
    string level = path.substr(start, end - start);
    m_PathLevels.push_back(level);
}

string CURLHandler::getNormFilePath() const
{
    vector<string> tempPath;

    // Normalize path
    for (auto &level : m_PathLevels)
    {
        // Move back one level
        if (level == "..")
        {
            if (tempPath.size() > 0)
                tempPath.pop_back();
            // else
            //     cout << "CURLHandler ERROR: Too much pop backs!" << endl;

            continue;
        }

        // Skip redundant dots or empty levels
        else if (level == "." || level.empty())
        {
            continue;
        }

        // Add level to path
        tempPath.push_back(level);
    }

    // Build the final normalized path
    stringstream path;

    // Add every level separate by slash
    size_t tempPathSize = tempPath.size();
    for (size_t i = 0; i < tempPathSize; i++)
    {
        path << tempPath[i];

        // Don't include trailing slash after the last element if its a file (contains dot)
        if (i != tempPathSize - 1 || tempPath[i].find('.') == string::npos)
            path << "/";
    }

    return path.str();
}

vector<string> CURLHandler::getNormalizedLevels() const
{
    vector<string> tempPath;

    // Normalize path
    for (auto &level : m_PathLevels)
    {
        // Move back one level
        if (level == "..")
        {
            if (tempPath.size() > 0)
                tempPath.pop_back();
            // else
            //     cout << "CURLHandler ERROR: Too much pop backs!" << endl;

            continue;
        }

        // Skip redundant dots or empty levels
        else if (level == "." || level.empty())
        {
            continue;
        }

        // Add level to path
        tempPath.push_back(level);
    }

    return tempPath;
}

string CURLHandler::getNormURL() const
{
    // Build the final normalized URL
    stringstream url;

    // Add protocol
    if (m_IsHttps)
        url << "https://";
    else
        url << "http://";

    // Add domain string with slash
    url << m_Domain << "/";

    // Add normalized path
    url << getNormFilePath();

    return url.str();
}

string CURLHandler::getNormURLPath() const
{
    string path = getNormFilePath();

    if (!m_HasTrailingSlash && !path.empty() && Utils::endsWith(path, "/"))
        path.pop_back();

    return path;
}

string CURLHandler::getDomain() const
{
    return m_Domain;
}

string CURLHandler::getDomainNorm() const
{
    if (Utils::startsWith(m_Domain, "www."))
        return m_Domain.substr(4);

    return m_Domain;
}

bool CURLHandler::isHttps() const
{
    return m_IsHttps;
}

bool CURLHandler::isExternal() const
{
    return m_IsExternal;
}

size_t CURLHandler::getPathDepth() const
{
    auto levels = getNormalizedLevels();

    // If there is no path after domain name
    if ((levels.size() == 1 && levels.back() == "") || levels.size() <= 0)
    {
        return 0;
    }

    // If last element is a file (contains a dot)
    if (levels.back().find('.') != string::npos)
    {
        return levels.size() - 1;
    }

    return levels.size();
}