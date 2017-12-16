#include "catch.hpp"
#include "request.hh"

TEST_CASE("GET request") {
    REQUIRE_FALSE(request::Get("https://httpbin.org/get") == "");
}

TEST_CASE("POST request") {
    REQUIRE_FALSE(request::Post("https://httpbin.org/post",
                                "test=1&hello=goodbye") == "");
}