#include "util.hh"

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

bool CopyFile(std::string source_file, std::string new_file) {
    std::ifstream source_stream(source_file, std::ios::binary);
    std::ofstream new_stream(new_file, std::ios::binary | std::ios::trunc);
    if (source_stream.is_open() && new_stream.is_open()) {
        new_stream << source_stream.rdbuf();
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

#endif  // __linux__

#ifdef WIN32

#include <windows.h>

bool IS_SUPERUSER;
bool IS_SUPERUSER_IS_CACHED = false;

// Use CopyFileA from windows.h
bool CopyFile(std::string source_file, std::string new_file) {
    return CopyFileA(source_file.c_str(), new_file.c_str(), false);
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
        install_dir = ADMIN_INSTALL_DIR + "/";
    } else {
        install_dir = std::getenv("USERPROFILE") + ("/" + INSTALL_DIR + "/");
    }

    return install_dir;
}

#endif  // WIN32

}  // namespace util
