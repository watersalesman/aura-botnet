#define UNICODE
#define _UNICODE

#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <tuple>
#include <vector>

namespace request {

// Handles creation of POST forms and conversion to string
class PostForm {
   public:
    void AddField(const std::string& field, const std::string& value);
    std::string ToString();

   private:
    std::vector<std::tuple<std::string, std::string>> data_;
};

// GET request
std::string Get(const std::string& url);

// POST request
std::string Post(const std::string& url, const std::string& post_form);

}  // namespace request

#ifdef WIN32

#include <stdio.h>
#include <windows.h>
#include <wininet.h>

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

#endif  // WIN32

#endif  // REQUEST_HH
