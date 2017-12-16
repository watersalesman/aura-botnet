#include "catch.hpp"
#include "constants.hh"
#include "helper.hh"
#include "installer.hh"

SCENARIO("Working with Installer class") {
    GIVEN("a new Installer instance") {
        std::remove(AUTH_FILE.c_str());
        std::remove(BIN_NEW.c_str());
        Installer install("");

        THEN("the Installer instance should be seen as new") {
            REQUIRE(install.IsNew());
        }

        THEN("auth file is initialized in install directory") {
            REQUIRE(FileExists(AUTH_FILE));
        }

        WHEN("Another install class is created") {
            Installer not_new_install("");

            THEN("it should be not be seen as new") {
                REQUIRE_FALSE(not_new_install.IsNew());
            }
        }

        GIVEN("the client executable") {
            REQUIRE(CreateTestFile(BIN));

            WHEN("installing the client") {
                install.InstallFiles();

                THEN("install to install directory") {
                    REQUIRE(FileExists(BIN_NEW));
                }
            }
        }
    }
}