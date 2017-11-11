#ifndef WIN_REQUEST_HH
#define WIN_REQUEST_HH

#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#pragma comment (lib, "urlmon.lib")
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>

class WinINet {
    public:
        WinINet(const char* host, int port);
        ~WinINet();
        std::string getHost() { return _host; }
        int getPort() { return _port; }
        void request(std::string httpMethod, INTERNET_SCHEME scheme, std::string uri, std::string data);
        std::string getResponse() { return _responseStr; }

    private:
        std::string _host;
        int _port;
        char _userAgent[512];
        DWORD _uaSize;
        char _buffer[4000];
        DWORD _bytesRead;
        std::string _responseStr;
        HINTERNET _internet, _connection, _request;
};

WinINet::WinINet(const char* host, int port = INTERNET_DEFAULT_HTTP_PORT) {
    _host = host;
    _port = port;
	_uaSize = sizeof(_userAgent);
    ObtainUserAgentString(0, _userAgent, &_uaSize);
    _internet = InternetOpenA(_userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    _connection = InternetConnectA(_internet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
}

WinINet::~WinINet() {
    InternetCloseHandle(_request);
    InternetCloseHandle(_connection);
    InternetCloseHandle(_internet);
}

void WinINet::request(std::string httpMethod, INTERNET_SCHEME scheme, std::string uri, std::string data = "") {
    _responseStr = "";

    if (httpMethod != "POST") {
        httpMethod = "GET";
    }

    // Determine if using SSL
	DWORD requestFlags = (INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE);
	if (scheme == INTERNET_SCHEME_HTTPS) {
		requestFlags = (requestFlags | INTERNET_FLAG_SECURE);
	}
    // Create request
    _request = HttpOpenRequestA(_connection, httpMethod.c_str(), uri.c_str(), "HTTP/1.1", NULL, NULL, requestFlags, NULL);
    if (_request) {
        // Prepare header and optional post form
        std::string header;
        if (httpMethod == "POST") {
            header = "Content-Type: application/x-www-form-urlencoded";
        }
        int headerlen = header.size();
        char* form = new char[data.length() + 1];
        strcpy(form, data.c_str());
        int formlen = strlen(form);

        // Send request
        BOOL requestSuccess = HttpSendRequestA(_request, header.c_str(), headerlen, form, formlen);

        // Read request response
        if (requestSuccess) {
            while(InternetReadFile(_request, &_buffer, strlen(_buffer), &_bytesRead) && _bytesRead > 0) {
                _responseStr.append(_buffer, _bytesRead);
            }
        }
        delete[] form;
    }
}

static std::string requestHandler(std::string httpMethod, std::string url, std::string data="") {
    // Use InternetCrackUrlA() to parse URL
	URL_COMPONENTS urlParts;
	char scheme[20], host[128], user[256], pass[256], uri[512], extraInfo[512];
	urlParts.lpszScheme = scheme;
	urlParts.dwSchemeLength = sizeof(scheme);
	urlParts.lpszHostName = host;
	urlParts.dwHostNameLength = sizeof(host);
	urlParts.lpszUserName = user;
	urlParts.dwUserNameLength = sizeof(user);
	urlParts.lpszPassword = pass;
	urlParts.dwPasswordLength = sizeof(pass);
	urlParts.lpszUrlPath = uri;
	urlParts.dwUrlPathLength = sizeof(uri);
	urlParts.lpszExtraInfo = extraInfo;
	urlParts.dwExtraInfoLength = sizeof(extraInfo);
	urlParts.dwStructSize = sizeof(urlParts);
	BOOL crackSuccess = InternetCrackUrlA(url.c_str(), url.length(), 0, &urlParts);

    // Pass retrieved URL components
	WinINet http(urlParts.lpszHostName, urlParts.nPort);
	http.request(httpMethod, urlParts.nScheme, urlParts.lpszUrlPath, data);
	return http.getResponse();
}

namespace request {
    // GET
	std::string get(std::string url) {
		return requestHandler("GET", url);
	}

    // POST
	std::string post(std::string url, std::string data) {
		return requestHandler("POST", url, data);
	}

} //namespace request

#endif // WIN_REQUEST_HH
