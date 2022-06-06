#pragma once

#include "CURLHandler.h"

#include <string>
#include <ctime>   //tm
#include <iomanip> //get_time

using std::string;

class CResponse
{
public:
    enum class EStatus
    {
        FINISHED,
        IN_PROGRESS,
        MOVED,
        TIMED_OUT,
        CONN_ERROR,
        SERVER_ERROR
    };

    CResponse() = default;

    explicit CResponse(EStatus status);

    ~CResponse() = default;

    void setMovedUrl(const string &location, CURLHandler currentUrl);

    void setContentType(const string &contentType);

    void setContentDisposition(const string &contentDisposition);

    void setLastModified(const string &lastModified);

    void setBody(const string &body);

    EStatus getStatus();

    string getBody();

    void setStatus(EStatus status);

    void setStatusCode(int statusCode);

    size_t m_ContentLength = 0;
    CURLHandler m_MovedUrl;

private:
    EStatus m_Status = EStatus::IN_PROGRESS;
    int m_StatusCode;
    string m_ContentType;
    string m_ContentDisposition;
    std::time_t m_LastModified = 0;
    string m_Body;
};