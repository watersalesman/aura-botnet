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

bool IsSuperuser();

/* Determine install directory based on operating system, permissions, and other
 * factors */
std::string GetInstallDir();

// Run system command and return output as string
std::string PopenSubprocess(const std::string& command);

}  // namespace util

#endif  // UTIL_HH
