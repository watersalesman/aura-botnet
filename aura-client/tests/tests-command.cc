#include "catch.hpp"
#include "aura.hh"
#include <iostream>

SCENARIO("using Command class") {

    GIVEN("a test JSON command") {
        std::string testFile = "hello.txt";
        std::string command = "touch ";
#ifdef WIN32
        command = "type nul > ";
#endif
        std::string response =
            "{\"shell\": \"default\", \"command_text\": \""
            + command
            + testFile
            + "\"}";
        rapidjson::Document json;
        json.Parse(response.c_str());
        Command cmd(response);

        THEN("construct construct successfully") {
            REQUIRE(cmd.commandText == command + testFile);
            REQUIRE(json.IsObject());
        }

        THEN("execute command") {
            REQUIRE(util::popenSubprocess(cmd.execute()) == "");
            REQUIRE(std::remove(testFile.c_str()) == 0);
        }
    }

    GIVEN("an invalid JSON response") {
        std::string response = "<>?><<<<<><NOTVALID";
        Command cmd(response);

        THEN("set empty commandText") {
            REQUIRE(cmd.commandText == "");
        }

        THEN("set default shell") {
            REQUIRE(cmd.shell == "default");
        }

        THEN("execute() returns empty string") {
            REQUIRE(cmd.execute() == "");
        }
    }

#ifdef WIN32
    GIVEN("a command object set to run in PowerShell") {
        std::string testFile = "hello.txt";
        std::string command = "echo hello > " + testFile;
        std::string response =
            "{\"shell\": \"powershell\", \"command_text\": \""
            + command
            + "\"}";
        Command cmd(response);

        THEN("execute() runs properly") {
            cmd.execute();
            REQUIRE(std::remove(testFile.c_str()) == 0);
        }

    }
#endif
#ifdef __linux__
    GIVEN("a command object set to run in Bash") {
        std::string testFile = "hello.txt";
        std::string command = "echo hello > " + testFile;
        std::string response =
            "{\"shell\": \"bash\", \"command_text\": \""
            + command
            + "\"}";
        Command cmd(response);

        THEN("execute() runs properly") {
            cmd.execute();
            REQUIRE(std::remove(testFile.c_str()) == 0);
        }

    }
#endif
}
