#include "catch.hpp"
#include "aura.hh"

SCENARIO("using Command class") {

    GIVEN("a test JSON command") {
        std::string testFile = "hello.txt";
        std::string response =
            "{\"shell\": \"default\", \"command_text\": \"touch "
            + testFile
            + "\"}";
        rapidjson::Document json;
        json.Parse(response.c_str());
        Command cmd(response);

        THEN("construct construct successfully") {
            REQUIRE(cmd.commandText == "touch hello.txt");
            REQUIRE(json.IsObject());
        }

        THEN("execute command") {
            cmd.execute();
            REQUIRE(std::remove(testFile.c_str()) == 0);
        }
    }

    GIVEN("an invalid JSON response") {
        std::string response = "<>?><<<<<><NOTVALID";
        Command cmd(response);

        THEN("return empty commandText") {
            REQUIRE(cmd.commandText == "");
        }
    }
}
