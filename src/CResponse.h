/**
 * @file CResponse.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CResponse
 *
 */

#pragma once

#include "CURLHandler.h"

#include <string>

using std::string;

/**
 * @brief Http Response class containing header info, content and status
 *
 */
class CResponse
{
public:
    /**
     * @brief Status of CResponse (Not HTTP status) used for internal logic
     *
     */
    enum class EStatus
    {
        FINISHED,
        IN_PROGRESS,
        MOVED,
        TIMED_OUT,
        CONN_ERROR,
        SERVER_ERROR
    };

    /**
     * @brief Construct a new CResponse object
     *
     */
    CResponse() = default;

    /**
     * @brief Construct a new CResponse object and set initial status
     *
     * @param status
     */
    explicit CResponse(EStatus status);

    /**
     * @brief Set next moved URL
     *
     * @param location New URL from header Location
     * @param currentUrl Current URL
     */
    void setMovedUrl(const string &location, CURLHandler currentUrl);

    CURLHandler m_MovedUrl;
    EStatus m_Status = EStatus::IN_PROGRESS;
    int m_ContentLength = -1;
    int m_StatusCode;
    string m_ContentType;
    string m_ContentDisposition;
    string m_Body;
};