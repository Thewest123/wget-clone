/**
 * @file tests.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include "CURLHandler.h"

using namespace std;

int main(int argc, char const *argv[])
{
     cout << "---------- [STARTING TESTS] ----------\n"
          << endl;

     // ============ Utils ============
     cout << "---------- [Testing Utils] -----------" << endl;

     string str = "/loremipsum/";
     cout << "startsWith(): " << boolalpha << Utils::startsWith(str, "/") << endl;
     cout << "endsWith(): " << boolalpha << Utils::endsWith(str, "/") << endl;

     // ============ CURLHandler ============
     cout << "------- [Testing CURLHandler] --------" << endl;

     string url1 = "https://google.com";
     cout << "Input: " << url1 << endl;

     CURLHandler urlHandler(url1);

     cout << "IsHttps: " << boolalpha << urlHandler.isHttps() << endl;
     cout << "Domain: " << urlHandler.getDomain() << endl;
     cout << "NormURL: " << urlHandler.getNormURL() << endl;

     urlHandler.addPath("/path/to/for/../index.html");

     cout << "NormURL: " << urlHandler.getNormURL() << endl;

     cout << endl;

     string url2 = "jakpsatweb.cz/muj/soubor/";
     cout << "Input: " << url2 << endl;

     CURLHandler urlHandler2(url2);

     cout << "IsHttps: " << boolalpha << urlHandler2.isHttps() << endl;
     cout << "Domain: " << urlHandler2.getDomain() << endl;
     cout << "NormURL: " << urlHandler2.getNormURL() << endl;

     urlHandler2.addPath("/path//to/./for/../folder");

     cout << "NormURL: " << urlHandler2.getNormURL() << endl;

     // ============ END ============
     cout << "\n--------- [ALL TESTS PASSED] ---------\n"
          << endl;

     return EXIT_SUCCESS;
}
