/**
 * @file CHttpDownloader.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem> // Kvuli tvorbe slozek

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include <unistd.h> // Kvuli close() na sockfd
#include <string.h>

#include "CHttpDownloader.h"
#include "CLogger.h"

using namespace std;

/**
 * @brief Make GET request to the URL and save it to file
 *
 * @param url
 */
string CHttpDownloader::get(const string &url)
{
    int sockfd;
    sockaddr_in serv_address;
    hostent *server;
    char buffer[4096];

    string host = "";
    string port = "80";
    string resource = "/";
    string query = "";

    auto &logger = CLogger::getInstance();

    if (!parseUrl(url, host, resource, query))
        logger.log(CLogger::LogLevel::Error, "Failed to parse URL");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        logger.log(CLogger::LogLevel::Error, "Failed to create socket");
        return "";
    }

    server = gethostbyname(host.c_str());

    if (server == NULL)
    {
        logger.log(CLogger::LogLevel::Error, "Failed to get host by name");
        close(sockfd);
        return "";
    }

    memset((char *)&serv_address, 0, sizeof(serv_address));

    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(stoi(port));

    memmove((char *)&serv_address.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);

    if (connect(sockfd, (sockaddr *)&serv_address, sizeof(serv_address)) < 0)
    {
        logger.log(CLogger::LogLevel::Error, "Connection failed");
    
        close(sockfd);
        return "";
    }

    string request = "GET " + resource + query + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    logger.log(CLogger::LogLevel::Verbose, request);

    if (send(sockfd, request.c_str(), request.size(), 0) < 0)
    {
        logger.log(CLogger::LogLevel::Error, "Request failed");
        close(sockfd);
        return "";
    }

    size_t n;
    string response;
    while ((n = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        response.append(buffer, n);
    }

    close(sockfd);

    size_t headerLen = getHeaderLength(response);

    string responseHeader = response.substr(0, headerLen);

    response.erase(0, headerLen);

    return response;
}

size_t CHttpDownloader::getHeaderLength(string &content) const
{
    const string endString1 = "\r\n\r\n";
    const string endString2 = "\n\r\n\r";

    // Try to find \r\n\r\n
    size_t endPosition = content.find(endString1);

    if (endPosition != string::npos)
    {
        endPosition += endString1.length();
        return endPosition;
    }

    // Try to find \n\r\n\r
    endPosition = content.find(endString2);
    endPosition += endString2.length();

    return endPosition;
}

bool CHttpDownloader::parseUrl(const string &url, string &host, string &resource, string &query) const
{
    const regex re("(?:http:\\/\\/|https:\\/\\/)?((?:www\\.)?[^/]*)([^\\?]*)(.*)", regex_constants::icase);
    smatch result;

    if (regex_match(url, result, re) != 1 || result.size() < 2)
        return false;

    host = result[1].str();
    resource = result[2].str();
    query = result[3].str();

    if (resource.empty())
        resource = "/";

    return true;
}