#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <cstring>
#include <cstdlib>
#include <curl/curl.h>

namespace {


// Struct for handling response data of varying sizes
struct MemoryStruct
{
    char *memory;
    size_t size;
};


// Write function to use to return curl response as a char*
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL)
    {
        return 1;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


char *requestHandler(const char *url, const char *postForm = NULL)
{
    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    CURL *curl;
    CURLcode response;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        // If POST form data is passed, include them in the request body
        if (postForm) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postForm);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        response = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    return chunk.memory;
}


} //namespace

namespace request {


std::string get(std::string url)
{
    //Convert string to char*
    const char *charUrl = url.c_str();

    //Send request and receive response as char*
    const char *charResponse = requestHandler(charUrl);

    // Ensure response is not NULL, then convert it to a string
    if (charResponse == NULL) charResponse = "";
    std::string response(charResponse);

    return response;
}


std::string post(std::string url, std::string postForm)
{
    //Convert strings to char*
    const char *charUrl = url.c_str();
    const char *charPostForm = postForm.c_str();

    //Send request and receive response as char*
    const char *charResponse = requestHandler(charUrl, charPostForm);

    // Ensure response is not NULL, then convert it to a string
    if (charResponse == NULL) charResponse = "";
    std::string response(charResponse);

    return response;
}


} //namespace request

#endif // REQUEST_HH
