#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <tuple>
#include <vector>

namespace request {

class PostForm {
   public:
    void AddField(const std::string& field, const std::string& value);
    std::string ToString();

   private:
    std::vector<std::tuple<std::string, std::string>> data_;
};

void PostForm::AddField(const std::string& field, const std::string& value) {
    data_.push_back(std::make_tuple(field, value));
}

std::string PostForm::ToString() {
    std::string form_string, field, value;
    for (int i = 0; i < data_.size(); ++i) {
        if (i) form_string += "&";
        std::tie(field, value) = data_[i];
        form_string += field + "=" + value;
    }

    return form_string;
}

}  // namespace request

#ifdef __linux__

#include <curl/curl.h>
#include <cstring>

// Write function for returning curl response as a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     void* out_string) {
    ((std::string*)out_string)->append((char*)contents);

    return nmemb * size;
}

std::string RequestHandler(const std::string& url,
                           const std::string& post_form = "") {
    std::string response;
    CURL* curl;
    CURLcode response_code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // If POST form data is passed, include them in the request body
        if (post_form != "") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_form.c_str());
        }

        // Set write function and string to write to
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        response_code = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    return response;
}

namespace request {

// GET request
std::string Get(const std::string& url) { return RequestHandler(url); }

// POST request
std::string Post(const std::string& url, const std::string& post_form) {
    return RequestHandler(url, post_form);
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
    std::string GetHost() { return host_; }
    int GetPort() { return port_; }
    void Request(const std::string& method, INTERNET_SCHEME scheme,
                 const std::string& uri, const std::string& data);
    std::string GetResponse() { return response_; }

   private:
    std::string host_;
    int port_;
    char user_agent_[512];
    DWORD ua_size_;
    char buffer_[4000];
    DWORD bytes_read_;
    std::string response_;
    HINTERNET internet_, connection_, request_;
};

WinINet::WinINet(const char* host, int port = INTERNET_DEFAULT_HTTP_PORT) {
    host_ = host;
    port_ = port;

    ua_size_ = sizeof(user_agent_);
    ObtainUserAgentString(0, user_agent_, &ua_size_);

    internet_ =
        InternetOpenA(user_agent_, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    connection_ = InternetConnectA(internet_, host, port, NULL, NULL,
                                   INTERNET_SERVICE_HTTP, 0, 0);
}

WinINet::~WinINet() {
    InternetCloseHandle(request_);
    InternetCloseHandle(connection_);
    InternetCloseHandle(internet_);
}

void WinINet::Request(const std::string& method, INTERNET_SCHEME scheme,
                      const std::string& uri, const std::string& data) {
    response_ = "";
    std::string http_method = method;

    if (http_method != "POST") {
        http_method = "GET";
    }

    // Determine if using SSL
    DWORD request_flags = (INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE);
    if (scheme == INTERNET_SCHEME_HTTPS) {
        request_flags = (request_flags | INTERNET_FLAG_SECURE);
    }

    // Create request
    request_ = HttpOpenRequestA(connection_, http_method.c_str(), uri.c_str(),
                                "HTTP/1.1", NULL, NULL, request_flags, NULL);

    if (request_) {
        // Prepare header and optional post form
        std::string header;
        if (http_method == "POST") {
            header = "Content-Type: application/x-www-form-urlencoded";
        }
        int header_len = header.size();
        char* form = new char[data.length() + 1];
        strcpy(form, data.c_str());
        int form_len = strlen(form);

        // Send request
        BOOL request_success = HttpSendRequestA(request_, header.c_str(),
                                                header_len, form, form_len);

        // Read request response
        if (request_success) {
            while (InternetReadFile(request_, &buffer_, strlen(buffer_),
                                    &bytes_read_) &&
                   bytes_read_ > 0) {
                response_.append(buffer_, bytes_read_);
            }
        }
        delete[] form;
    }
}

static std::string RequestHandler(const std::string& http_method,
                                  const std::string& url,
                                  const std::string& data = "") {
    // Use InternetCrackUrlA() to parse URL
    URL_COMPONENTS url_parts;
    char scheme[20], host[128], user[256], pass[256], uri[512], extra_info[512];
    url_parts.lpszScheme = scheme;
    url_parts.dwSchemeLength = sizeof(scheme);
    url_parts.lpszHostName = host;
    url_parts.dwHostNameLength = sizeof(host);
    url_parts.lpszUserName = user;
    url_parts.dwUserNameLength = sizeof(user);
    url_parts.lpszPassword = pass;
    url_parts.dwPasswordLength = sizeof(pass);
    url_parts.lpszUrlPath = uri;
    url_parts.dwUrlPathLength = sizeof(uri);
    url_parts.lpszExtraInfo = extra_info;
    url_parts.dwExtraInfoLength = sizeof(extra_info);
    url_parts.dwStructSize = sizeof(url_parts);
    BOOL crackSuccess =
        InternetCrackUrlA(url.c_str(), url.length(), 0, &url_parts);

    // Pass retrieved URL components
    WinINet http(url_parts.lpszHostName, url_parts.nPort);
    http.Request(http_method, url_parts.nScheme, url_parts.lpszUrlPath, data);
    return http.GetResponse();
}

namespace request {

// GET request
std::string Get(std::string url) { return RequestHandler("GET", url); }

// POST request
std::string Post(std::string url, std::string data) {
    return RequestHandler("POST", url, data);
}

}  // namespace request

#endif  // WIN32

#endif  // REQUEST_HH
