#include <string>

#include "authfile.hh"
#include "catch.hpp"
#include "helper.hh"

SCENARIO("Working with AuthFile class") {
    GIVEN("a new AuthFile instance") {
        std::string authfile_path("test_auth.file");
        std::remove(authfile_path.c_str());
        AuthFile auth(authfile_path);

        WHEN("uninitialized") {
            THEN("the auth file should not exist") {
                REQUIRE_FALSE(auth.Exists());
            }
            THEN("auth hash should be empty") {
                REQUIRE(auth.GetHash().empty());
            }
        }

        WHEN("initialized") {
            auth.Init();

            THEN("auth file should exist") { REQUIRE(auth.Exists()); }
            THEN("auth hash should not be empty") {
                REQUIRE_FALSE(auth.GetHash().empty());
            }
            GIVEN("a second AuthFile instance with the same file path") {
                AuthFile new_auth(authfile_path);

                THEN("new auth file should already exist") {
                    REQUIRE(new_auth.Exists());
                }

                WHEN("retrieving existing seed") {
                    new_auth.Retrieve();
                    THEN("new auth hash should match old auth hash") {
                        REQUIRE(new_auth.GetHash() == auth.GetHash());
                    }
                }
            }
        }
    }
}