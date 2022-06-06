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

private:
    EStatus m_Status;
    int m_StatusCode;
    CURLHandler m_MovedUrl;
    string m_ContentType;
    string m_ContentDisposition;
    std::time_t m_LastModified;
    string m_Body;
};