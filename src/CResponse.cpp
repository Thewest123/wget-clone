#include <sstream>

#include "CResponse.h"
#include "Utils.h"
#include "CLogger.h"
#include "CConfig.h"

#include <iostream>
#include <iomanip>

using std::stringstream;

CResponse::CResponse(EStatus status)
    : m_Status(status) {}

void CResponse::setContentType(const string &contentType)
{
    m_ContentType = contentType;
}

void CResponse::setContentDisposition(const string &contentDisposition)
{
    m_ContentDisposition = contentDisposition;
}

void CResponse::setBody(const string &body)
{
    m_Body = body;
}

CResponse::EStatus CResponse::getStatus()
{
    return m_Status;
}

string CResponse::getBody()
{
    return m_Body;
}

void CResponse::setStatus(EStatus status)
{
    m_Status = status;
}

void CResponse::setStatusCode(int statusCode)
{
    m_StatusCode = statusCode;
}

void CResponse::setLastModified(const string &lastModified)
{
    std::tm t;
    std::istringstream ss(lastModified);

    ss >> std::get_time(&t, "%a, %d %b %Y %H:%M:%S");
    if (ss.fail())
    {
        throw std::runtime_error{"Failed to parse LastModified date!"};
    }

    m_LastModified = mktime(&t);
}

void CResponse::setMovedUrl(const string &location, CURLHandler currentUrl)
{
    if (m_StatusCode == 301)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Verbose, "301 Moved Permanently - New location: " + location);

        CURLHandler newUrl(location);

        if (CURLHandler(static_cast<string>(CConfig::getInstance()["url"])).getDomainNorm() != newUrl.getDomainNorm())
            m_MovedUrl = CURLHandler(location, true);
        else
            m_MovedUrl = CURLHandler(location, currentUrl.isExternal());
    }
    else if (m_StatusCode == 302)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Verbose, "302 Found - New location: " + location);

        // If the URL is not full, but only relative to domain, prepend it with domain
        string newLocation;
        if (Utils::startsWith(location, "/"))
        {
            stringstream ss;
            if (currentUrl.isHttps())
                ss << "https://";
            else
                ss << "http://";

            ss << currentUrl.getDomain();
            ss << location;

            newLocation = ss.str();
        }
        else
        {
            newLocation = location;
        }

        CURLHandler newUrl(newLocation);

        if (CURLHandler(static_cast<string>(CConfig::getInstance()["url"])).getDomainNorm() != newUrl.getDomainNorm())
            m_MovedUrl = CURLHandler(newLocation, true);
        else
            m_MovedUrl = CURLHandler(newLocation, currentUrl.isExternal());
    }

    m_Status = EStatus::MOVED;
}