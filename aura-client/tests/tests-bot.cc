#include <json.hpp>

#include "bot.hh"
#include "catch.hpp"
#include "constants.hh"
#include "helper.hh"

using json = nlohmann::json;

SCENARIO("using the Command class") {
    GIVEN("a test JSON command") {
        std::string test_file = "hello.txt";
        std::string command = "touch ";
#ifdef WIN32
        command = "type nul > ";
#endif
        std::string response = "{\"shell\": \"default\", \"command_text\": \"" +
                               command + test_file + "\", \"files\": []}";
        json res_json = json::parse(response.c_str());
        Command cmd(response);

        THEN("construct construct successfully") {
            REQUIRE(cmd.command_text == command + test_file);
            REQUIRE(res_json.is_object());
        }

        THEN("execute command") {
            REQUIRE(util::PopenSubprocess(cmd.Execute()) == "");
            REQUIRE_FALSE(std::remove(test_file.c_str()) == 0);
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
            "\", \"files\": []}";
        Command cmd(response);

        THEN("Execute() runs properly") {
            cmd.Execute();
            REQUIRE_FALSE(std::remove(test_file.c_str()) == 0);
        }
    }
#endif  // WIN#2

#ifdef __linux__
    GIVEN("a command object set to run in Bash") {
        std::string test_file = "hello.txt";
        std::remove(test_file.c_str());
        std::string command = "echo hello > " + test_file;
        std::string response = "{\"shell\": \"bash\", \"command_text\": \"" +
                               command + "\", \"files\": []}";
        Command cmd(response);

        THEN("Execute() runs properly") {
            cmd.Execute();
            REQUIRE_FALSE(std::remove(test_file.c_str()) == 0);
        }
    }
#endif  // __linux__

    GIVEN("testing Command object with file dependencies") {
        CreateTestFile("test1");
        CreateTestFile("test2");
        std::string command = "cat dep1 dep2";
#ifdef WIN32
        command = "type dep1 dep2";
#endif
        std::string response =
            "{\"shell\": \"default\", \"command_text\": \"" + command +
            "\","
            "\"files\": ["
            "{\"name\": \"netfile1\", \"type\": \"network\",\"path\": "
            "\"https://httpbin.org/image/jpeg\"},"
            "{\"name\": \"dep1\", \"type\": \"local\",\"path\": \"../test1\"},"
            "{\"name\": \"dep2\", \"type\": \"local\",\"path\": \"../test2\"},"
            "{\"name\": \"invalid\", \"type\": \"local\",\"path\": \"\"},"
            "{\"name\": \"\", \"type\": \"local\",\"path\": \"invalid\"},"
            "{\"name\": \"dep2\", \"type\": \"local\",\"path\": "
            "\"nonexistent\"}"
            "]}";
        Command cmd(response);
        WHEN("executing command") {
            std::string result = cmd.Execute();
            THEN("local files are retrieved") {
                REQUIRE(result == "Test ContentTest Content");
            }
        }
        REQUIRE(std::remove("netfile1") != 0);
        REQUIRE(std::remove("dep1") != 0);
        REQUIRE(std::remove("dep2") != 0);
        REQUIRE(std::remove("test1") == 0);
        REQUIRE(std::remove("test2") == 0);
    }
}
