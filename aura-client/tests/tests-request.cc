#include <fstream>
#include <string>

#include "catch.hpp"
#include "helper.hh"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "request.hh"

SCENARIO("Working with request::Get()") {
    GIVEN("the url https://httpbin.org/get") {
        WHEN("sending a GET request") {
            std::string host = "httpbin.org";
            std::string url = "https://" + host + "/get";
            std::string response = request::Get(url);

            THEN("response is not empty") { REQUIRE_FALSE(response.empty()); }
            WHEN("parsing response") {
                rapidjson::Document json;
                json.Parse(response.c_str());

                THEN("response is a valid JSON object") {
                    REQUIRE(json.IsObject());
                }
                THEN("url field is correct") {
                    REQUIRE(json["url"].IsString());
                    REQUIRE(json["url"].GetString() == url);
                }
                THEN("Host field is correct") {
                    REQUIRE(json["headers"].IsObject());
                    REQUIRE(json["headers"]["Host"].IsString());
                    REQUIRE(json["headers"]["Host"].GetString() == host);
                }
            }
        }
    }
}

SCENARIO("Working with request::Post() and PostField class") {
    GIVEN("the url https://httpbin.org/post and a POST form") {
        // URL
        std::string host = "httpbin.org";
        std::string url = "https://" + host + "/post";

        // POST Form
        std::string field1 = "one", field2 = "two", value1 = "1", value2 = "2";
        request::PostForm post_data;
        post_data.AddField(field1, value1);
        post_data.AddField(field2, value2);

        WHEN("sending a POST request") {
            std::string response = request::Post(url, post_data.ToString());

            THEN("response is not empty") { REQUIRE_FALSE(response.empty()); }

            WHEN("parsing response") {
                rapidjson::Document json;
                json.Parse(response.c_str());

                THEN("response is a valid JSON object") {
                    REQUIRE(json.IsObject());
                }
                THEN("url field is correct") {
                    REQUIRE(json["url"].IsString());
                    REQUIRE(json["url"].GetString() == url);
                }
                THEN("Host field is correct") {
                    REQUIRE(json["headers"].IsObject());
                    REQUIRE(json["headers"]["Host"].IsString());
                    REQUIRE(json["headers"]["Host"].GetString() == host);
                }

                THEN("POST data matches original POST form") {
                    REQUIRE(json["form"].IsObject());
                    REQUIRE(json["form"][field1.c_str()].IsString());
                    REQUIRE(json["form"][field1.c_str()].GetString() == value1);
                    REQUIRE(json["form"][field2.c_str()].IsString());
                    REQUIRE(json["form"][field2.c_str()].GetString() == value2);
                }
            }
        }
    }
}

SCENARIO("Working with request::DownloadFile()") {
    GIVEN("the jpeg file at url https://httpbin.org/image/jpeg") {
        std::string url = "https://httpbin.org/image/jpeg";
        std::string file_name = "test.jpeg";
        WHEN("Downloading the file") {
            request::DownloadFile(url, file_name);
            THEN("File is downloaded properly") {
                std::ifstream file_stream(file_name.c_str(), std::ios::binary);
                REQUIRE(file_stream.good());
                REQUIRE(GetFileHash(file_stream) ==
                        "c028d7aa15e851b0eefb31638a1856498a237faf1829050832d3b9"
                        "b19f9ab75f");
            }
            REQUIRE(std::remove(file_name.c_str()) == 0);
        }
    }
}