/**
 * @file CHttpDownloader.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <iostream>

using namespace std;

class CHttpDownloader
{
public:
    CHttpDownloader(const string &outputPath)
        : m_OutputPath(outputPath){};

    void setHeader(const string &header)
    {
    }

    /**
     * @brief Make GET request to the URL and save it to file
     *
     * @param url
     */
    void get(const string &url)
    {
    }

private:
    string m_OutputPath = "";
};