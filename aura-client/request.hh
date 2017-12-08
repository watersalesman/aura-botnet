#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <tuple>
#include <vector>

namespace request {

class PostForm {
   public:
    void addField(const std::string& field, const std::string& value);
    std::string toString();

   private:
    std::vector<std::tuple<std::string, std::string>> data_;
};

void PostForm::addField(const std::string& field, const std::string& value) {
    data_.push_back(std::make_tuple(field, value));
}

std::string PostForm::toString() {
    std::string formString, field, value;
    for (int i = 0; i < data_.size(); ++i) {
        if (i) formString += "&";
        std::tie(field, value) = data_[i];
        formString += field + "=" + value;
    }

    return formString;
}

}  // namespace request

#ifdef __linux__

#include <curl/curl.h>
#include <cstring>

// Write function for returning curl response as a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     void* outString) {
    ((std::string*)outString)->append((char*)contents);

    return nmemb * size;
}

std::string requestHandler(const std::string& url,
                           const std::string& postForm = "") {
    std::string response;
    CURL* curl;
    CURLcode resCode;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // If POST form data is passed, include them in the request body
        if (postForm != "") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postForm.c_str());
        }

        // Set write function and string to write to
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        resCode = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    return response;
}

namespace request {

// GET request
std::string get(const std::string& url) { return requestHandler(url); }

// POST request
std::string post(const std::string& url, const std::string& postForm) {
    return requestHandler(url, postForm);
}

}  // namespace request

#endif  // __linux__

#ifdef WIN32

#include <stdio.h>
#include <windows.h>
#include <wininet.h>
#include <vector>

class WinINet {
   public:
    WinINet(const char* host, int port);
    ~WinINet();
    std::string getHost() { return host_; }
    int getPort() { return port_; }
    void request(const std::string& method, INTERNET_SCHEME scheme,
                 const std::string& uri, const std::string& data);
    std::string getResponse() { return responseStr_; }

   private:
    std::string host_;
    int port_;
    char userAgent_[512];
    DWORD usSize_;
    char buffer_[4000];
    DWORD bytesRead_;
    std::string responseStr_;
    HINTERNET internet_, connection_, request_;
};

WinINet::WinINet(const char* host, int port = INTERNET_DEFAULT_HTTP_PORT) {
    host_ = host;
    port_ = port;

    usSize_ = sizeof(userAgent_);
    ObtainUserAgentString(0, userAgent_, &usSize_);

    internet_ =
        InternetOpenA(userAgent_, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    connection_ = InternetConnectA(internet_, host, port, NULL, NULL,
                                   INTERNET_SERVICE_HTTP, 0, 0);
}

WinINet::~WinINet() {
    InternetCloseHandle(request_);
    InternetCloseHandle(connection_);
    InternetCloseHandle(internet_);
}

void WinINet::request(const std::string& method, INTERNET_SCHEME scheme,
                      const std::string& uri, const std::string& data) {
    responseStr_ = "";
    std::string httpMethod = method;

    if (httpMethod != "POST") {
        httpMethod = "GET";
    }

    // Determine if using SSL
    DWORD requestFlags = (INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE);
    if (scheme == INTERNET_SCHEME_HTTPS) {
        requestFlags = (requestFlags | INTERNET_FLAG_SECURE);
    }

    // Create request
    request_ = HttpOpenRequestA(connection_, httpMethod.c_str(), uri.c_str(),
                                "HTTP/1.1", NULL, NULL, requestFlags, NULL);

    if (request_) {
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
        BOOL requestSuccess = HttpSendRequestA(request_, header.c_str(),
                                               headerlen, form, formlen);

        // Read request response
        if (requestSuccess) {
            while (InternetReadFile(request_, &buffer_, strlen(buffer_),
                                    &bytesRead_) &&
                   bytesRead_ > 0) {
                responseStr_.append(buffer_, bytesRead_);
            }
        }
        delete[] form;
    }
}

static std::string requestHandler(const std::string& httpMethod,
                                  const std::string& url,
                                  const std::string& data = "") {
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
    BOOL crackSuccess =
        InternetCrackUrlA(url.c_str(), url.length(), 0, &urlParts);

    // Pass retrieved URL components
    WinINet http(urlParts.lpszHostName, urlParts.nPort);
    http.request(httpMethod, urlParts.nScheme, urlParts.lpszUrlPath, data);
    return http.getResponse();
}

namespace request {

// GET request
std::string get(std::string url) { return requestHandler("GET", url); }

// POST request
std::string post(std::string url, std::string data) {
    return requestHandler("POST", url, data);
}

}  // namespace request

#endif  // WIN32

#endif  // REQUEST_HH
