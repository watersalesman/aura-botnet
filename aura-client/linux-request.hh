#ifndef LINUX_REQUEST_HH
#define LINUX_REQUEST_HH

#include <string>
#include <cstring>
#include <curl/curl.h>

namespace {

// Write function for returning curl response as a string
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *outString) {
    ((std::string *)outString)->append((char *)contents);

    return nmemb * size;
}

std::string requestHandler(std::string url, std::string postForm = "") {

    std::string response;
    CURL *curl;
    CURLcode resCode;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

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

} // namespace

namespace request {

std::string get(std::string url) {
    std::string response = requestHandler(url);

    return response;
}

std::string post(std::string url, std::string postForm) {
    std::string response = requestHandler(url, postForm);

    return response;
}

} //namespace request

#endif // LINUX_REQUEST_HH
