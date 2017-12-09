#include "catch.hpp"
#include "request.hh"

TEST_CASE("GET request") {
    REQUIRE(request::Get("https://httpbin.org/get") != "");
}

TEST_CASE("POST request") {
    REQUIRE(request::Post("https://httpbin.org/post", "test=1&hello=goodbye") !=
            "");
}