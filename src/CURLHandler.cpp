#include "CURLHandler.h"

/**
 * @brief Construct a new CURLHandler object
 *
 * @param url Base URL of the CURLHandler, can be just domain or full URL
 */
CURLHandler::CURLHandler(const string &url)
{
    regex re("((?:http://|https://)?[^/]*)/?(.*)");
    smatch result;

    if (!regex_match(url, result, re))
    {
        cout << "CURLHandler ERROR: Too much pop backs!" << endl;
        //! throw
        return;
    }

    setDomain(result[1].str());
    addPath(result[2].str());
}

CURLHandler::~CURLHandler()
{
}

/**
 * @brief Sets the domain URL and deducts protocol from 'urlDomain'
 *
 * @param urlDomain Domain of the URL, eg. https://google.com/
 */
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

/**
 * @brief Add additional relative part of path to the existing URL
 *
 * @param path Additional relative path (eg. "next/directory/../index.html")
 */
void CURLHandler::addPath(const string &path)
{
    string delimiter = "/";

    // If the path doesn't end with trailing slash, don't add it later with normalization
    if (Utils::endsWith(path, delimiter))
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

/**
 * @brief Returns only the normalized path without domain, with fixed path changes (eg. '../')
 *
 * @return string Normalized path only
 */
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
            else
                cout << "CURLHandler ERROR: Too much pop backs!" << endl;

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

/**
 * @brief Returns the full normalized URL with fixed path changes (eg. '../')
 *
 * @return string Normalized URL
 */
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

    if (!m_HasTrailingSlash)
        path.pop_back();

    return path;
}

string CURLHandler::getDomain() const
{
    return m_Domain;
}

bool CURLHandler::isHttps() const
{
    return m_IsHttps;
}