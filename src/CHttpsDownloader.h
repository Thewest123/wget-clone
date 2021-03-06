/**
 * @file CHttpsDownloader.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CHttpsDownloader, also contains TDeleter
 *
 */

#pragma once

#include "CURLHandler.h"
#include "CResponse.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/crypto.h>

#include <unistd.h> // Kvuli close() na sockfd
#include <string.h>

#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem> // Kvuli tvorbe slozek
#include <memory>     // unique_ptr<>
#include <string>
#include <vector>

using std::string, std::vector, std::unique_ptr;

// OpenSSL handling inspired and studied from 5 part blog post
// available on https://quuxplusone.github.io/blog/2020/01/24/openssl-part-1/
// but thoroughly modified to use STL functions and types instead of C functions, to fix memory leaks, and rewritten to use non-blocking sockets

template <typename T>
/**
 * @brief Templated deleter struct for unique_ptr
 *
 */
struct TDeleter;

template <>
struct TDeleter<BIO>
{
    void operator()(BIO *p) const { BIO_free_all(p); }
};

template <>
struct TDeleter<SSL_CTX>
{
    void operator()(SSL_CTX *p) const { SSL_CTX_free(p); }
};

template <>
struct TDeleter<X509>
{
    void operator()(X509 *p) const { X509_free(p); }
};

/**
 * @brief Class that interacts through sockets with web server, makes SSL handshake and validates certificates, downloads content and parses headers
 *
 */
class CHttpsDownloader
{
public:
    /**
     * @brief Construct a new CHttpsDownloader object, init OpenSSL, load SSL trust store
     *
     */
    CHttpsDownloader();

    /**
     * @brief Makes GET request to the URL and returns content
     *
     * @param url CURLHandler url of the remote file
     * @return CResponse Content of the downloaded file
     */
    CResponse get(CURLHandler &url);

private:
    /**
     * @brief Receives data through socket using BIO, retries the connection if appropriate
     *
     * @param bio
     * @return string
     */
    string receiveData(BIO *bio);

    /**
     * @brief Gets data from BIO, validates response, parses headers
     *
     * @param bio
     * @param currentUrl
     * @return CResponse
     */
    CResponse receiveHttpMessage(BIO *bio, CURLHandler &currentUrl);

    /**
     * @brief Sends the HTTP/HTTPS request using provided BIO
     *
     * @param bio Pointer to BIO object
     * @param resource Required remote resource (eg. '/file/index.html')
     * @param host Host of the resource (eg. 'google.com')
     */
    void sendHttpRequest(BIO *bio, const string &resource, const string &host);

    /**
     * @brief Extract SSL certificate from SSL BIO
     *
     * @param bio Pointer to BIO object
     * @return SSL*
     */
    SSL *getSSL(BIO *bio);

    /**
     * @brief Verify validity of the SSL certificate for provided hostname
     *
     * @param ssl Pointer to the SSL certificate
     * @param expectedHostname Hostname for verification
     * @return true If valid
     * @return false If invalid or error
     */
    bool verifyCertificate(SSL *ssl, const string &expectedHostname);

    /**
     * @brief Pointer to the SSL context
     *
     */
    unique_ptr<SSL_CTX, TDeleter<SSL_CTX>> m_Ctx;

    const string HTTP_PORT = "80";
    const string HTTPS_PORT = "443";
};
