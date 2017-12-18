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
