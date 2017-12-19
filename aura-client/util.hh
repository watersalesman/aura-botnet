#ifndef UTIL_HH
#define UTIL_HH

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

class TempDirectory {
   public:
    TempDirectory(const std::string& path);
    ~TempDirectory();
    fs::path Get();

   private:
    fs::path path_;
};

bool IsSuperuser();

/* Determine install directory based on operating system, permissions, and other
 * factors */
fs::path GetInstallDir();

// Run system command and return output as string
std::string PopenSubprocess(const std::string& command);

}  // namespace util

#endif  // UTIL_HH
