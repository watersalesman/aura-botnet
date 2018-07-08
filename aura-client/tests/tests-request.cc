#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>

#include "catch.hpp"
#include "helper.hh"
#include "request.hh"

using json = nlohmann::json;

SCENARIO("Working with request::Get()") {
    GIVEN("the url http://127.0.0.1:5000/get") {
        WHEN("sending a GET request") {
            std::string host = "127.0.0.1:5000";
            std::string url = "http://" + host + "/get";
            std::string response = request::Get(url);

            THEN("response is not empty") { REQUIRE_FALSE(response.empty()); }
            WHEN("parsing response") {
                auto res_json = json::parse(response);

                THEN("response is a valid JSON object") {
                    REQUIRE(res_json.is_object());
                }
                THEN("url field is correct") {
                    REQUIRE(res_json["url"].is_string());
                    REQUIRE(res_json["url"] == url);
                }
                THEN("Host field is correct") {
                    REQUIRE(res_json["headers"].is_object());
                    REQUIRE(res_json["headers"]["Host"].is_string());
                    REQUIRE(res_json["headers"]["Host"] == host);
                }
            }
        }
    }
}

SCENARIO("Working with request::Post() and PostField class") {
    GIVEN("the url http://127.0.0.1:5000/post and a POST form") {
        // URL
        std::string host = "127.0.0.1:5000";
        std::string url = "http://" + host + "/post";

        // POST Form
        std::string field1 = "one", field2 = "two", value1 = "1", value2 = "2";
        request::PostForm post_data;
        post_data.AddField(field1, value1);
        post_data.AddField(field2, value2);

        WHEN("sending a POST request") {
            std::string response = request::Post(url, post_data.ToString());

            THEN("response is not empty") { REQUIRE_FALSE(response.empty()); }

            WHEN("parsing response") {
                std::cout << response.size() << ": " << response << std::endl;
                auto res_json = json::parse(response);

                THEN("response is a valid JSON object") {
                    REQUIRE(res_json.is_object());
                }
                THEN("url field is correct") {
                    REQUIRE(res_json["url"].is_string());
                    REQUIRE(res_json["url"] == url);
                }
                THEN("Host field is correct") {
                    REQUIRE(res_json["headers"].is_object());
                    REQUIRE(res_json["headers"]["Host"].is_string());
                    REQUIRE(res_json["headers"]["Host"] == host);
                }

                THEN("POST data matches original POST form") {
                    REQUIRE(res_json["form"].is_object());
                    REQUIRE(res_json["form"][field1.c_str()].is_string());
                    REQUIRE(res_json["form"][field1.c_str()] == value1);
                    REQUIRE(res_json["form"][field2.c_str()].is_string());
                    REQUIRE(res_json["form"][field2.c_str()] == value2);
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