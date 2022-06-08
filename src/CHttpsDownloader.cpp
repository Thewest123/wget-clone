/**
 * @file CHttpsDownloader.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CHttpsDownloader
 *
 */

#include "CHttpsDownloader.h"
#include "CLogger.h"
#include "CConfig.h"
#include "Utils.h"
#include "CURLHandler.h"

using std::unique_ptr, std::regex, std::smatch, std::regex_match, std::stringstream;

CHttpsDownloader::CHttpsDownloader()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    m_Ctx = unique_ptr<SSL_CTX, TDeleter<SSL_CTX>>(SSL_CTX_new(SSLv23_client_method()));
#else
    m_Ctx = unique_ptr<SSL_CTX, TDeleter<SSL_CTX>>(SSL_CTX_new(TLS_client_method()));
#endif

    // Add user defined certificates
    string certStore = static_cast<string>(CConfig::getInstance()["cert_store"]);

    if (!certStore.empty())
    {
        int result = SSL_CTX_load_verify_locations(m_Ctx.get(), certStore.c_str(), NULL);
        if (result == 1)
            CLogger::getInstance().log(CLogger::ELogLevel::Info, "Custom SSL trust store \"" + certStore + "\" successfully loaded!");
        else
            CLogger::getInstance().log(CLogger::ELogLevel::Error, "Cannot load custom SSL trust store \"" + certStore + "\"!");
    }

    // Add system preinstalled certificates
    if (SSL_CTX_set_default_verify_paths(m_Ctx.get()) != 1)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't setup SSL trust store! Use flag --cert-store to specify custom path.");
    }

    SSL_CTX_set_timeout(m_Ctx.get(), 10L);
}

CResponse CHttpsDownloader::get(CURLHandler &url)
{
    // Setup variables
    string host = url.getDomain();
    string port = url.isHttps() ? HTTPS_PORT : HTTP_PORT;
    string resource = "/" + url.getNormURLPath();

    CLogger::getInstance().log(CLogger::ELogLevel::Info, "Downloading " + url.getNormURL());

    // Create connection with BIO
    auto bio = unique_ptr<BIO, TDeleter<BIO>>(BIO_new_connect((host + ":" + port).c_str()));

    if (bio.get() == nullptr)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't create connection!");
        return CResponse(CResponse::EStatus::CONN_ERROR);
    }

    // Make BIO non-blocking
    BIO_set_nbio(bio.get(), 1);

    // Establish connection
    // (Socket connection for old OpenSSL version @inspiredBy https://stackoverflow.com/a/39060166)
    int connectResult = BIO_do_connect(bio.get());
    int fd;
    fd_set confds;

    // Return if connection failed and shouldn't try again
    if ((connectResult <= 0) && !BIO_should_retry(bio.get()))
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't connect, error occured!");
        return CResponse(CResponse::EStatus::CONN_ERROR);
    }

    // Return if socket failed
    if (BIO_get_fd(bio.get(), &fd) < 0)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't get socket file descriptor");
        return CResponse(CResponse::EStatus::CONN_ERROR);
    }

    // Try again repeatedly until timeout
    if (connectResult <= 0)
    {
        FD_ZERO(&confds);
        FD_SET(fd, &confds);
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 10;
        connectResult = select(fd + 1, NULL, &confds, NULL, &tv);
        if (connectResult == 0)
        {
            CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't connect, timed out!");
            return CResponse(CResponse::EStatus::TIMED_OUT);
        }
    }
    // End of @inspiredBy

    // If not HTTPS, simply download and return
    if (!url.isHttps())
    {
        // Send HTTP request
        sendHttpRequest(bio.get(), resource, host);

        // Download the content
        return receiveHttpMessage(bio.get(), url);
    }

    // Make SSL handshake if HTTPS

    // Create new BIO with SSL from SSL Context
    auto ssl_bio = unique_ptr<BIO, TDeleter<BIO>>(BIO_new_ssl(m_Ctx.get(), 1));

    // Push (copy) current BIO to new SSL bio and discard it
    BIO_push(ssl_bio.get(), bio.release());

    // Set expected hostname for certificate
    SSL_set_tlsext_host_name(getSSL(ssl_bio.get()), host.c_str());

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    // Set expected DNS hostname
    SSL_set1_host(getSSL(ssl_bio.get()), host.c_str());
#endif

    // Try to make a handshake
    int handshakeResult;
    do
    {
        handshakeResult = BIO_do_handshake(ssl_bio.get());

    } while (BIO_should_retry(ssl_bio.get()));

    // Return if failed
    if (handshakeResult <= 0)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't make SSL handshake!");
        return CResponse(CResponse::EStatus::CONN_ERROR);
    }
    else
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "SSL handshake successful!");
    }

    // Get SSL certificate from server and verify it
    SSL *sslpointer = getSSL(ssl_bio.get());
    if (!verifyCertificate(sslpointer, host.c_str()))
        return CResponse(CResponse::EStatus::CONN_ERROR);

    // Send HTTP request with SSL
    sendHttpRequest(ssl_bio.get(), resource, host);

    // Download the content
    return receiveHttpMessage(ssl_bio.get(), url);
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

    } while (BIO_should_retry(bio));

    return ss.str();
}

CResponse CHttpsDownloader::receiveHttpMessage(BIO *bio, CURLHandler &currentUrl)
{
    string content = receiveData(bio);
    string headerDelimiter = "\r\n\r\n";

    size_t headerEnd = content.find(headerDelimiter);

    // Read data until delimiter
    while (headerEnd == string::npos)
    {
        content += receiveData(bio);
        headerEnd = content.find(headerDelimiter);
    }

    // Copy everything after delimiter to body
    string body = content.substr(headerEnd + headerDelimiter.length());

    // Remove part after delimiter from header
    string header = content.substr(0, headerEnd + 2);

    // Split headers
    vector<string> headers = Utils::splitString(header, "\r\n");

    // Check HTTP response validity
    regex re_httpStatus("HTTP/\\d\\.\\d\\s+(\\d+)\\s+(.*)", std::regex_constants::icase);
    smatch result;

    if (regex_match(headers[0], result, re_httpStatus) == false)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "The server didn't send valid HTTP response!");
        return CResponse(CResponse::EStatus::SERVER_ERROR);
    }

    // Set status code
    int statusCode = std::stoi(result[1].str());

    CResponse response(CResponse::EStatus::IN_PROGRESS);
    response.setStatusCode(statusCode);

    // Parse other headers
    for (const string &line : headers)
    {
        size_t colon = line.find(':');

        if (colon == string::npos)
            continue;

        string key = line.substr(0, colon);
        string value = line.substr(colon + 2); // +2 to skip colon and whitespace

        if (key == "Content-Length")
            response.m_ContentLength = std::stoi(value);

        if (key == "Location")
            response.setMovedUrl(value, currentUrl);

        if (key == "Content-Type")
            response.setContentType(value);

        if (key == "Content-Disposition")
            response.setContentDisposition(value);

        if (key == "Last-Modified")
            response.setLastModified(value);
    }

    // If finished, don't download body (eg. 404 or 301 etc occured)
    if (response.getStatus() == CResponse::EStatus::FINISHED ||
        response.getStatus() == CResponse::EStatus::MOVED)
        return response;

    // Read data if possible
    while (true)
    {
        string newData = receiveData(bio);

        // If server sent Content-Length, we can check it
        if (response.m_ContentLength != -1)
            if (body.length() >= static_cast<size_t>(response.m_ContentLength))
                break;

        // Otherwise we have to try until there are no more data present
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
    // Construct the GET header
    stringstream ss;

    ss << "GET " << resource << " HTTP/1.0"
       << "\r\n";

    ss << "Host: " << host
       << "\r\n";

    ss << "Connection: close"
       << "\r\n";

    // Add other values from config
    auto &cfg = CConfig::getInstance();
    string cookies = cfg["cookies"];
    string userAgent = cfg["cookies"];

    if (!cookies.empty())
        ss << "Cookie: " << cookies << "\r\n";

    if (!userAgent.empty())
        ss << "User-Agent: " << userAgent << "\r\n";

    // End the header
    ss << "\r\n";

    string request = ss.str();

    // Send
    BIO_write(bio, request.data(), request.size());
    BIO_flush(bio);
}

SSL *CHttpsDownloader::getSSL(BIO *bio)
{
    // Extract SSL certificate from BIO
    SSL *ssl = nullptr;
    BIO_get_ssl(bio, &ssl);

    if (ssl == nullptr)
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Can't modify connection to use SSL!");

    return ssl;
}

bool CHttpsDownloader::verifyCertificate(SSL *ssl, const std::string &expectedHostname)
{
    int result = SSL_get_verify_result(ssl);

    if (result != X509_V_OK)
    {
        const char *message = X509_verify_cert_error_string(result);
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Certificate verification error: " + string(message) + " - " + std::to_string(result));
        return false;
    }

    auto cert = unique_ptr<X509, TDeleter<X509>>(SSL_get_peer_certificate(ssl));

    if (cert.get() == nullptr)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "No certificate was presented by the server!");
        return false;
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (X509_check_host(cert, expectedHostname.data(), expectedHostname.size(), 0, nullptr) != 1)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Certificate verification error in X509_check_host");
        return false;
    }
#else
    // X509_check_host is called automatically during verification,
    // because we set it up in CHttpsDownloader::get() with SSL_set1_host().
    (void)expectedHostname;
#endif

    return true;
}
