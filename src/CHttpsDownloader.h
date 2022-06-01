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

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include "CLogger.h"
#include "CURLHandler.h"

using namespace std;

// OpenSSL handling inspired and studied from 5 part blog post
// available on https://quuxplusone.github.io/blog/2020/01/24/openssl-part-1/
// but thoroughly modified to use STL functions and types instead of C functions, and to fix memory leaks

template <class T>
struct DeleterOf;
template <>
struct DeleterOf<BIO>
{
    void operator()(BIO *p) const { BIO_free_all(p); }
};
template <>
struct DeleterOf<SSL_CTX>
{
    void operator()(SSL_CTX *p) const { SSL_CTX_free(p); }
};

template <typename T>
struct DeleterOf;

class CHttpsDownloader
{
public:
    CHttpsDownloader();
    ~CHttpsDownloader();
    void setHeader(const string &header);

    /**
     * @brief Make GET request to the URL and save it to file
     *
     * @param url
     */
    string get(CURLHandler &url);

private:
    bool parseUrl(const string &url, bool &isHttps, string &host, string &resource) const;

    string receiveData(BIO *bio);
    vector<string> splitHeaders(const string &header);
    string receiveHttpMessage(BIO *bio, CURLHandler &currentUrl);
    void sendHttpRequest(BIO *bio, const string &resource, const string &host);
    SSL *getSSL(BIO *bio);
    void verifyCertificate(SSL *ssl, const string &expectedHostname);

    unique_ptr<SSL_CTX, DeleterOf<SSL_CTX>> m_Ctx;

    const string HTTP_PORT = "80";
    const string HTTPS_PORT = "443";
};