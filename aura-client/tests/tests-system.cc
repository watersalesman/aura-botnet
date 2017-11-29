#include "catch.hpp"
#include "system.hh"

SCENARIO("copying files around") {

    GIVEN("the current binary being run") {
        std::string bin = "tests-system";
        std::string binCopy = "tests-system-COPY";

        THEN("make a copy of the file") {
            REQUIRE(util::copyFile(bin, binCopy));
            std::remove(binCopy.c_str());
        }

#ifdef __linux
        THEN("make a hard link") {
            REQUIRE(util::copyFile(bin, "tests-system-COPY"));
            std::remove(binCopy.c_str());
        }
#endif
    }
}
