#define UNICODE
#define _UNICODE

#include "request.hh"

#include <string>
#include <tuple>
#include <vector>

namespace request {

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
#include <unistd.h>
#include <cstdlib>
#include <cstring>

// Write function for returning curl response as a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                            void* out_string) {
    ((std::string*)out_string)->append((char*)contents);

    return nmemb * size;
}

// Write function for downloading to file
static size_t WriteToFile(void* ptr, size_t size, size_t nmemb, void* stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
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

std::string Get(const std::string& url) { return RequestHandler(url); }

std::string Post(const std::string& url, const std::string& post_form) {
    return RequestHandler(url, post_form);
}

bool DownloadFile(const std::string& url, const std::string& file_path) {
    // Initialize curl session
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl_handle = curl_easy_init();

    // Set curl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteToFile);

    FILE* file_download = fopen(file_path.c_str(), "wb");
    if (file_download) {
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file_download);
        curl_easy_perform(curl_handle);
        fclose(file_download);
    } else
        return false;

    // Cleanup
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return true;
}

}  // namespace request

#endif  // __linux__

#ifdef WIN32

#include <atlbase.h>
#include <stdio.h>
#include <windows.h>
#include <wininet.h>
#include <memory>

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

        // Use unique_ptr for RAII cleanup of char*
        std::unique_ptr<char[]> form(new char[data.length() + 1]);
        strcpy(form.get(), data.c_str());
        int form_len = strlen(form.get());

        // Send request
        BOOL request_success = HttpSendRequestA(
            request_, header.c_str(), header_len, form.get(), form_len);

        // Read request response
        if (request_success) {
            while (InternetReadFile(request_, &buffer_, strlen(buffer_),
                                    &bytes_read_) &&
                   bytes_read_ > 0) {
                response_.append(buffer_, bytes_read_);
            }
        }
    }
}

static std::string RequestHandler(const std::string& http_method,
                                  const std::string& url,
                                  const std::string& data = "") {
    // Use InternetCrackUrlA() to parse URL
    URL_COMPONENTSA url_parts;
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
std::string Get(const std::string& url) { return RequestHandler("GET", url); }

// POST request
std::string Post(const std::string& url, const std::string& post_form) {
    return RequestHandler("POST", url, post_form);
}

bool DownloadFile(const std::string& url, const std::string& file_path) {
    USES_CONVERSION;
    LPCWSTR w_url = A2W(url.c_str());
    LPCWSTR w_file_path = A2W(file_path.c_str());

    HINTERNET internet =
        InternetOpenW(L"", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    // Get URL scheme
    URL_COMPONENTSW url_parts;
    wchar_t scheme[24];
    url_parts.lpszScheme = scheme;
    url_parts.dwSchemeLength = sizeof(scheme);
    url_parts.dwStructSize = sizeof(url_parts);
    BOOL crackSuccess = InternetCrackUrlW(w_url, wcslen(w_url), 0, &url_parts);

    // Set dwFlags
    DWORD flags = (INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE);
    if (url_parts.nScheme == INTERNET_SCHEME_HTTPS) {
        flags = (flags | INTERNET_FLAG_SECURE);
    }

    HINTERNET conn = InternetOpenUrlW(internet, w_url, NULL, NULL, flags, NULL);

    if (conn) {
        BYTE data[4000];
        INTERNET_BUFFERSW ibuf = {0};
        ibuf.dwStructSize = sizeof(ibuf);
        ibuf.lpvBuffer = data;
        ibuf.dwBufferLength = sizeof(data);
        FILE* outfile = _wfopen(w_file_path, L"wb");

        // Download content and write to file
        while (InternetReadFileExW(conn, &ibuf, NULL, NULL) &&
               ibuf.dwBufferLength > 0) {
            fwrite(ibuf.lpvBuffer, ibuf.dwBufferLength, 1, outfile);
        }

        fclose(outfile);

        return true;
    }

    else
        return false;
}

}  // namespace request

#endif  // WIN32