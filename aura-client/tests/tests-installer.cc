#include "catch.hpp"
#include "constants.hh"
#include "helper.hh"
#include "installer.hh"

SCENARIO("Working with Installer class") {
    GIVEN("a new Installer instance") {
        std::remove(BIN_NEW.c_str());
        Installer install("");

        GIVEN("the client file(s)") {
            REQUIRE(CreateTestFile(BIN));

#ifdef __linux__
            // Create dummy systemd files
            REQUIRE(CreateTestFile(SERVICE));
            REQUIRE(CreateTestFile(TIMER));
#endif  // __linux__

            WHEN("installing the client") {
                install.InstallFiles();

                THEN("install filed to correct locations") {
                    REQUIRE(FileExists(BIN_NEW));
#ifdef __linux__
                    // Confirm successful copy and cleanup intrusive files
                    std::string service_path =
                        std::getenv("HOME") + ("/" + SERVICE_DEST + "/");
                    REQUIRE(std::remove((service_path + SERVICE).c_str()) == 0);
                    REQUIRE(std::remove((service_path + TIMER).c_str()) == 0);
#endif  // __linux__
                }
            }
        }
    }
}