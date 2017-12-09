#include "catch.hpp"
#include "system.hh"

SCENARIO("copying files around") {
    GIVEN("the current binary being run") {
        std::string bin = "tests-system";
#ifdef WIN32
        bin += ".exe";
#endif
        std::string bin_copy = "tests-system-COPY";

        THEN("make a copy of the file") {
            REQUIRE(util::CopyFile(bin, bin_copy));
            std::remove(bin_copy.c_str());
        }

#ifdef __linux
        THEN("make a hard link") {
            REQUIRE(util::CopyFile(bin, "tests-system-COPY"));
            std::remove(bin_copy.c_str());
        }
#endif
    }
}
