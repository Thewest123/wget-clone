/**
 * @file CHttpsDownloader.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Class that interacts through sockets with web server, makes SSL handshake and validates certificates, downloads content and parses headers
 *
 */

#include "CHttpsDownloader.h"
#include "CLogger.h"
#include "CConfig.h"
#include "Utils.h"
#include "CURLHandler.h"

// using namespace std;
using std::unique_ptr, std::regex, std::smatch, std::regex_match, std::stringstream;

CHttpsDownloader::CHttpsDownloader()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    m_Ctx = unique_ptr<SSL_CTX, DeleterOf<SSL_CTX>>(SSL_CTX_new(SSLv23_client_method()));
#else
    m_Ctx = unique_ptr<SSL_CTX, DeleterOf<SSL_CTX>>(SSL_CTX_new(TLS_client_method()));
#endif

    // Add user defined certificates
    string certStore = static_cast<string>(CConfig::getInstance()["cert_store"]);

    if (!certStore.empty())
    {
        int result = SSL_CTX_load_verify_locations(m_Ctx.get(), NULL, certStore.c_str());
        if (result == 1)
            CLogger::getInstance().log(CLogger::LogLevel::Info, "Custom cert store \"" + certStore + "\" successfully loaded!");
        else
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Cannot load custom cert store \"" + certStore + "\"");
    }

    // Add system preinstalled certificates
    if (SSL_CTX_set_default_verify_paths(m_Ctx.get()) != 1)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't setup SSL trust store, are any certificates installed? Use flag --cert-store to specify custom path.");
        //! throw
    }

    SSL_CTX_set_timeout(m_Ctx.get(), 10L);
}

CResponse CHttpsDownloader::get(CURLHandler &url)
{
    bool isHttps = url.isHttps();
    string host = url.getDomain();
    string resource = "/" + url.getNormURLPath();

    CLogger::getInstance().log(CLogger::LogLevel::Info, "Downloading " + url.getNormURL());

    if (isHttps)
    {
        auto bio = unique_ptr<BIO, DeleterOf<BIO>>(BIO_new_connect((host + ":" + HTTPS_PORT).c_str()));

        if (bio.get() == nullptr)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't create connection!");
            return CResponse(CResponse::EStatus::CONN_ERROR);
        }

        BIO_set_nbio(bio.get(), 1);

        int connectStatus = BIO_do_connect_retry(bio.get(), 10, -1);
        if (connectStatus == -1)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't connect, error occured!");
            return CResponse(CResponse::EStatus::CONN_ERROR);
        }
        else if (connectStatus == 0)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't connect, timed out!");
            return CResponse(CResponse::EStatus::TIMED_OUT);
        }

        auto ssl_bio = unique_ptr<BIO, DeleterOf<BIO>>(BIO_new_ssl(m_Ctx.get(), 1));
        BIO_push(ssl_bio.get(), bio.release());

        SSL_set_tlsext_host_name(getSSL(ssl_bio.get()), host.c_str());

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        SSL_set1_host(getSSL(ssl_bio.get()), host.c_str());
#endif

        int handshakeResult;
        do
        {
            handshakeResult = BIO_do_handshake(ssl_bio.get());

        } while (BIO_should_retry(ssl_bio.get()));

        if (handshakeResult <= 0)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't make SSL handshake!");
            return CResponse(CResponse::EStatus::CONN_ERROR);
        }
        else
        {
            CLogger::getInstance().log(CLogger::LogLevel::Verbose, "SSL handshake success!");
        }

        SSL *sslpointer = getSSL(ssl_bio.get());
        verifyCertificate(sslpointer, host.c_str());

        sendHttpRequest(ssl_bio.get(), resource, host);
        return receiveHttpMessage(ssl_bio.get(), url);
    }
    else
    {
        auto bio = unique_ptr<BIO, DeleterOf<BIO>>(BIO_new_connect((host + ":" + HTTP_PORT).c_str()));

        if (bio.get() == nullptr)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't create connection to " + url.getNormURL() + " !");
            return CResponse(CResponse::EStatus::CONN_ERROR);
        }

        BIO_set_nbio(bio.get(), 1);

        int connectStatus = BIO_do_connect_retry(bio.get(), 10, -1);
        if (connectStatus == -1)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't connect, error occured!");
            return CResponse(CResponse::EStatus::CONN_ERROR);
        }
        else if (connectStatus == 0)
        {
            CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't connect, timed out!");
            return CResponse(CResponse::EStatus::TIMED_OUT);
        }

        sendHttpRequest(bio.get(), resource, host);
        return receiveHttpMessage(bio.get(), url);
    }
}

string CHttpsDownloader::receiveData(BIO *bio)
{
    stringstream ss;
    int dataLength;
    do
    {
        char buffer[1024];

        dataLength = BIO_read(bio, buffer, sizeof(buffer));

        if (dataLength > 0)
            ss << string(buffer, dataLength);

    } while (BIO_should_retry(bio) || dataLength > 0);

    if (ss.str().length() < 1)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Verbose, "Can't receive any more data!");
    }

    return ss.str();
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

CResponse CHttpsDownloader::receiveHttpMessage(BIO *bio, CURLHandler &currentUrl)
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

    // Split headers
    vector<string> headers = splitHeaders(header);

    // Check HTTP validity
    regex re_httpStatus("HTTP/\\d\\.\\d\\s+(\\d+)\\s+(.*)", std::regex_constants::icase);
    smatch result;

    if (regex_match(headers[0], result, re_httpStatus) == false)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "The server didn't send valid HTTP response!");
        return CResponse(CResponse::EStatus::SERVER_ERROR);
    }

    // Set status code
    int statusCode = std::stoi(result[1].str());

    CResponse response;
    response.setStatusCode(statusCode);

    // Parse other headers
    for (const string &line : headers)
    {
        size_t colon = line.find(':');

        if (colon == string::npos)
            continue;

        string key = line.substr(0, colon);
        string value = line.substr(colon + 2); // +2 to skip colon and whitespace

        if (key == "Location")
            response.setMovedUrl(value, currentUrl);

        if (key == "Content-Type")
            response.setContentType(value);

        if (key == "Content-Disposition")
            response.setContentDisposition(value);

        if (key == "Last-Modified")
            response.setLastModified(value);
    }

    // If finished, don't download body
    if (response.getStatus() == CResponse::EStatus::FINISHED)
        return response;

    // Read data if possible (until no more data present)
    while (true)
    {
        string newData = receiveData(bio);

        if (newData.length() <= 0)
            break;

        body += newData;
    }

    response.setStatus(CResponse::EStatus::FINISHED);
    response.setBody(body);

    return response;
}

void CHttpsDownloader::sendHttpRequest(BIO *bio, const string &resource, const string &host)
{
    string request = "";

    request += "GET " + resource + " HTTP/1.0\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";

    auto &cfg = CConfig::getInstance();
    string cookies = cfg["cookies"];
    string userAgent = cfg["cookies"];

    if (!cookies.empty())
        request += "Cookie: " + cookies + "\r\n";

    if (!userAgent.empty())
        request += "User-Agent: " + userAgent + "\r\n";

    request += "\r\n";

    BIO_write(bio, request.data(), request.size());
    BIO_flush(bio);
}

SSL *CHttpsDownloader::getSSL(BIO *bio)
{
    SSL *ssl = nullptr;
    BIO_get_ssl(bio, &ssl);

    if (ssl == nullptr)
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Can't modify connection to use SSL!");

    return ssl;
}

void CHttpsDownloader::verifyCertificate(SSL *ssl, const std::string &expectedHostname)
{
    int error = SSL_get_verify_result(ssl);

    if (error != X509_V_OK)
    {
        const char *message = X509_verify_cert_error_string(error);
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Certificate verification error: " + string(message) + " - " + std::to_string(error));
        //! throw
    }

    X509 *cert = SSL_get_peer_certificate(ssl);

    if (cert == nullptr)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "No certificate was presented by the server!");
        //! throw
    }

    X509_free(cert);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (X509_check_host(cert, expectedHostname.data(), expectedHostname.size(), 0, nullptr) != 1)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Certificate verification error in X509_check_host");
        //! throw
    }
#else
    // X509_check_host is called automatically during verification,
    // because we set it up in main().
    (void)expectedHostname;
#endif
}
