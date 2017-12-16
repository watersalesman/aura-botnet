#include "bot.hh"
#include "catch.hpp"
#include "constants.hh"
#include "helper.hh"

SCENARIO("using the Bot class") {
    GIVEN("a brand new Bot instance") {
        std::remove(AUTH_FILE.c_str());
        std::remove(BIN_NEW.c_str());
        Bot test_bot("");

        THEN("The Bot should be seen as new") { REQUIRE(test_bot.IsNew()); }

        THEN("Another Bot class should not be seen as new") {
            Bot not_new_bot("");
            REQUIRE(not not_new_bot.IsNew());
        }

        THEN("Auth file is created in install directory") {
            REQUIRE(FileExists(AUTH_FILE));
        }

        THEN("Install client to install directory") {
            test_bot.Install();
            REQUIRE(CreateTestFile(BIN));
            REQUIRE(FileExists(BIN_NEW));
        }
    }
}

SCENARIO("using the Command class") {
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
        std::remove(test_file.c_str());
        std::string command = "echo hello > " + test_file;
        std::string response =
            "{\"shell\": \"powershell\", \"command_text\": \"" + command +
            "\"}";
        Command cmd(response);

        THEN("Execute() runs properly") {
            cmd.Execute();
            REQUIRE(FileExists(test_file));
        }
    }
#endif

#ifdef __linux__
    GIVEN("a command object set to run in Bash") {
        std::string test_file = "hello.txt";
        std::remove(test_file.c_str());
        std::string command = "echo hello > " + test_file;
        std::string response =
            "{\"shell\": \"bash\", \"command_text\": \"" + command + "\"}";
        Command cmd(response);

        THEN("Execute() runs properly") {
            cmd.Execute();
            REQUIRE(FileExists(test_file));
        }
    }
#endif
}
