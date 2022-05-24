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

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/crypto.h>

#include <unistd.h> // Kvuli close() na sockfd
#include <string.h>

#include "CHttpsDownloader.h"
#include "CLogger.h"

using namespace std;

CHttpsDownloader::CHttpsDownloader()
{

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    m_Ctx = unique_ptr<SSL_CTX, DeleterOf<SSL_CTX>>(SSL_CTX_new(SSLv23_client_method());
#else
    m_Ctx = unique_ptr<SSL_CTX, DeleterOf<SSL_CTX>>(SSL_CTX_new(TLS_client_method()));
#endif

    //Add user defined certificates
    SSL_CTX_load_verify_locations(m_Ctx.get(), NULL, "keys");

    // Add system preinstalled certificates
    if (SSL_CTX_set_default_verify_paths(m_Ctx.get()) != 1)
    {
        cout << "Error setting up trust store" << endl;
        //! throw
    }
}

CHttpsDownloader::~CHttpsDownloader()
{
    // SSL_CTX_free(ctx);
}

/**
 * @brief Make GET request to the URL and save it to file
 *
 * @param url
 */
string CHttpsDownloader::get(const CURLHandler url)
{
    string response = "";

    bool isHttps = url.isHttps();
    string host = url.getDomain();
    string resource = "/" + url.getNormURLPath();

    // if (!parseUrl(url, isHttps, host, resource))
    //     cout << "ERROR: Failed parsing URL" << endl;

    cout << "isHttps: " << boolalpha << isHttps << endl;
    cout << "host: " << host << endl;
    cout << "resource: " << resource << endl;

    if (isHttps)
    {
        auto bio = unique_ptr<BIO, DeleterOf<BIO>>(BIO_new_connect((host + ":" + HTTPS_PORT).c_str()));

        if (bio == nullptr)
        {
            cout << "Error in BIO_new_connect" << endl;
            //! throw
        }

        if (BIO_do_connect(bio.get()) <= 0)
        {
            cout << "Error in BIO_do_connect" << endl;
            //! throw
        }

        auto ssl_bio = unique_ptr<BIO, DeleterOf<BIO>>(BIO_new_ssl(m_Ctx.get(), 1));
        BIO_push(ssl_bio.get(), bio.release());

        SSL_set_tlsext_host_name(getSSL(ssl_bio.get()), host.c_str());

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        SSL_set1_host(getSSL(ssl_bio.get()), host.c_str());
#endif

        if (BIO_do_handshake(ssl_bio.get()) <= 0)
        {
            cout << "Error in BIO_do_handshake" << endl;
            //! throw
        }

        SSL *sslpointer = getSSL(ssl_bio.get());
        verifyCertificate(sslpointer, host.c_str());

        sendHttpRequest(ssl_bio.get(), resource, host);
        response = receiveHttpMessage(ssl_bio.get());
    }
    else
    {
        auto bio = unique_ptr<BIO, DeleterOf<BIO>>(BIO_new_connect((host + ":" + HTTP_PORT).c_str()));

        if (bio == nullptr)
        {
            cout << "Error in BIO_new_connect" << endl;
            //! throw
        }

        if (BIO_do_connect(bio.get()) <= 0)
        {
            cout << "Error in BIO_do_connect" << endl;
            //! throw
        }

        sendHttpRequest(bio.get(), resource, host);
        response = receiveHttpMessage(bio.get());
    }

    return response;
}

// private

bool CHttpsDownloader::parseUrl(const string &url, bool &isHttps, string &host, string &resource) const
{

    const regex re("(?:http://|https://)?([^/]{3,})(.*)", regex_constants::icase);
    smatch result;

    if (!regex_match(url, result, re))
    {
        return false;
    }

    // If url starts with https://
    if (url.rfind("https://", 0) == 0)
        isHttps = true;
    else
        isHttps = false;

    host = result[1].str();
    resource = result[2].str();

    if (resource.empty())
        resource = "/";

    return true;
}

string CHttpsDownloader::receiveData(BIO *bio)
{
    char buffer[1024];

    int dataLength = BIO_read(bio, buffer, sizeof(buffer));

    if (dataLength < 0)
    {
        cout << "ERROR Receiving data!";
        //! throw
        return "";
    }

    else if (BIO_should_retry(bio))
        return receiveData(bio);

    else if (dataLength >= 0)
        return string(buffer, dataLength);

    return "";
}

vector<string> CHttpsDownloader::splitHeaders(const string &header)
{
    vector<string> lines;
    string delimiter = "\r\n";

    auto start = 0U;
    auto end = header.find(delimiter);

    while (end != string::npos)
    {
        lines.push_back(header.substr(start, end - start));
        start = end + delimiter.length();
        end = header.find(delimiter, start);
    }

    lines.push_back(header.substr(start, end));

    return lines;
}

string CHttpsDownloader::receiveHttpMessage(BIO *bio)
{
    string header = receiveData(bio);
    string delimiter = "\r\n\r\n";

    size_t headerEnd = header.find(delimiter);

    // Read data until delimiter
    while (headerEnd == string::npos)
    {
        header += receiveData(bio);
        headerEnd = header.find(delimiter);
    }

    // Copy everything after delimiter to body
    string body = header.substr(headerEnd + delimiter.length());

    // Remove part after delimiter from header
    header = header.substr(0, headerEnd + 2);

    // Split and process headers
    vector<string> headers = splitHeaders(header);

    regex re_httpStatus("HTTP/\\d\\.\\d\\s+(\\d+)\\s+(.*)", regex_constants::icase);
    smatch result;

    if (regex_match(headers[0], result, re_httpStatus) == false)
    {
        cout << "ERROR: Regex match HTTP status" << endl;
        //! throw
    }

    int statusCode = stoi(result[1].str());
    string hdr_location;

    for (const string &line : headers)
    {
        size_t colon = line.find(':');

        if (colon == string::npos)
            continue;

        string key = line.substr(0, colon);
        string value = line.substr(colon + 2); // +2 to skip colon and whitespace

        if (key == "Location")
            hdr_location = value;
    }

    if (statusCode == 301)
    {
        cout << "MOVED 301 to " << hdr_location << endl;
        CURLHandler newUrl(hdr_location);
        return get(newUrl);
    }
    else if (statusCode == 302)
    {
        cout << "MOVED 302 to " << hdr_location << endl;
        return "xxx";
    }

    // Read data if possible
    while (true)
    {
        string newData = receiveData(bio);

        if (newData.length() <= 0)
            break;

        body += newData;
    }

    return body;
}

void CHttpsDownloader::sendHttpRequest(BIO *bio, const string &resource, const string &host)
{
    string request = "";

    request += "GET " + resource + " HTTP/1.0\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "User-Agent: WGET-Project/0.1 (FIT CVUT, Jan Cerny <cernyj87@fit.cvut.cz>)\r\n";
    request += "\r\n";

    BIO_write(bio, request.data(), request.size());
    BIO_flush(bio);
}

SSL *CHttpsDownloader::getSSL(BIO *bio)
{
    SSL *ssl = nullptr;
    BIO_get_ssl(bio, &ssl);

    if (ssl == nullptr)
        cout << "ERROR in getSSL()" << endl;

    return ssl;
}

void CHttpsDownloader::verifyCertificate(SSL *ssl, const std::string &expectedHostname)
{
    int error = SSL_get_verify_result(ssl);

    if (error != X509_V_OK)
    {
        const char *message = X509_verify_cert_error_string(error);
        cout << "ERROR: Certificate verification error: " << message << " - " << error << endl;
        //! throw
    }

    X509 *cert = SSL_get_peer_certificate(ssl);

    if (cert == nullptr)
    {
        cout << "ERROR: No certificate was presented by the server" << endl;
        //! throw
    }

    X509_free(cert);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (X509_check_host(cert, expectedHostname.data(), expectedHostname.size(), 0, nullptr) != 1)
    {
        cout << "ERROR: Certificate verification error: X509_check_host" << endl;
        //! throw
    }
#else
    // X509_check_host is called automatically during verification,
    // because we set it up in main().
    (void)expectedHostname;
#endif
}
