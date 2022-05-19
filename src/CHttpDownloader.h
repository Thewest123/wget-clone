#pragma once

/**
 * @file CHttpDownloader.h
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

using namespace std;

class CHttpDownloader
{
public:
    CHttpDownloader() = default;
    CHttpDownloader(const string &outputPath);
    void setHeader(const string &header);

    /**
     * @brief Make GET request to the URL and save it to file
     *
     * @param url
     */
    string get(const string &url);

private:
    string m_OutputPath = "./files";
    size_t getHeaderLength(string &content) const;
    bool parseUrl(const string &url, string &host, string &resource, string &query) const;

};