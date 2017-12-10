#include <iostream>

#include "bot.hh"
#include "catch.hpp"

SCENARIO("using Command class") {
    GIVEN("a test JSON command") {
        std::string test_file = "hello.txt";
        std::string command = "touch ";
#ifdef WIN32
        command = "type nul > ";
#endif
        std::string response = "{\"shell\": \"default\", \"command_text\": \"" +
                               command + test_file + "\"}";
        rapidjson::Document json;
        json.Parse(response.c_str());
        Command cmd(response);

        THEN("construct construct successfully") {
            REQUIRE(cmd.command_text == command + test_file);
            REQUIRE(json.IsObject());
        }

        THEN("execute command") {
            REQUIRE(util::PopenSubprocess(cmd.Execute()) == "");
            REQUIRE(std::remove(test_file.c_str()) == 0);
        }
    }

    GIVEN("an invalid JSON response") {
        std::string response = "<>?><<<<<><NOTVALID";
        Command cmd(response);

        THEN("set empty command_text") { REQUIRE(cmd.command_text == ""); }

        THEN("set default shell") { REQUIRE(cmd.shell == "default"); }

        THEN("Execute() returns empty string") { REQUIRE(cmd.Execute() == ""); }
    }

#ifdef WIN32
    GIVEN("a command object set to run in PowerShell") {
        std::string test_file = "hello.txt";
        std::string command = "echo hello > " + test_file;
        std::string response =
            "{\"shell\": \"powershell\", \"command_text\": \"" + command +
            "\"}";
        Command cmd(response);

        THEN("Execute() runs properly") {
            cmd.Execute();
            REQUIRE(std::remove(test_file.c_str()) == 0);
        }
    }
#endif

#ifdef __linux__
    GIVEN("a command object set to run in Bash") {
        std::string test_file = "hello.txt";
        std::string command = "echo hello > " + test_file;
        std::string response =
            "{\"shell\": \"bash\", \"command_text\": \"" + command + "\"}";
        Command cmd(response);

        THEN("Execute() runs properly") {
            cmd.Execute();
            REQUIRE(std::remove(test_file.c_str()) == 0);
        }
    }
#endif
}
