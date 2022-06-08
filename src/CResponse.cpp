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

void CResponse::setMovedUrl(const string &location, CURLHandler currentUrl)
{
    string newLocation = location;

    if (m_StatusCode == 301)
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "301 Moved Permanently - New location: " + newLocation);

    else if (m_StatusCode == 302)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "302 Found - New location: " + newLocation);

        // If the URL is not full, but only relative to domain, prepend it with domain
        if (Utils::startsWith(newLocation, "/"))
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
            newLocation = location;
    }

    CURLHandler newUrl(newLocation);

    if (CURLHandler(static_cast<string>(CConfig::getInstance()["url"])).getDomainNorm() != newUrl.getDomainNorm())
        newUrl.setExternal(true);
    else
        newUrl.setExternal(currentUrl.isExternal());

    m_MovedUrl = newUrl;
    m_Status = EStatus::MOVED;
}