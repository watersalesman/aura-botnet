#include "catch.hpp"
#include "util.hh"

SCENARIO("copying files around") {
    GIVEN("the current binary being run") {
        std::string bin = "tests-util";
#ifdef WIN32
        bin += ".exe";
#endif
        std::string bin_copy = "tests-util-COPY";

        THEN("make a copy of the file") {
            REQUIRE(util::CopyFile(bin, bin_copy));
            std::remove(bin_copy.c_str());
        }

#ifdef __linux
        THEN("make a hard link") {
            REQUIRE(util::CopyFile(bin, bin_copy));
            std::remove(bin_copy.c_str());
        }
#endif
    }
}
