#include "catch.hpp"
#include "request.hh"


TEST_CASE("GET request") {
    REQUIRE(request::get("https://httpbin.org/get") != "");
}

TEST_CASE("POST request") {
    REQUIRE(request::post("https://httpbin.org/post", "test=1&hello=goodbye") != "");
}
