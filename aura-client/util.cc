#include "util.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <string>

#include "constants.hh"
#include "request.hh"

namespace fs = std::experimental::filesystem;

namespace util {

TempDirectory::TempDirectory(const std::string& path) {
    path_ = path;
    fs::create_directories(path_);
}

TempDirectory::~TempDirectory() { fs::remove_all(path_); }

fs::path TempDirectory::Get() { return path_; }

#ifdef __linux__

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

fs::path GetInstallDir() {
    fs::path install_dir = INSTALL_DIR;
    install_dir = IsSuperuser() ? ROOT_HOME / install_dir
                                : std::getenv("HOME") / install_dir;

    return install_dir;
}

#endif  // __linux__

#ifdef WIN32

bool IS_SUPERUSER;
bool IS_SUPERUSER_IS_CACHED = false;

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

fs::path GetInstallDir() {
    fs::path install_dir;
    install_dir = IsSuperuser()
                      ? ADMIN_INSTALL_DIR
                      : std::getenv("USERPROFILE") + ("/" + INSTALL_DIR);

    return install_dir;
}

#endif  // WIN32

}  // namespace util
