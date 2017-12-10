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

bool CopyFile(std::string source_path, std::string new_path);

bool IsSuperuser();

/* Determine install directory based on operating system, permissions, and
 * values in constants.hh */
std::string GetInstallDir();

std::string PopenSubprocess(const std::string& command);

// Linux-specific components
#ifdef __linux__

#include <unistd.h>

int LinkFile(std::string source_file, std::string new_file);

#endif  // __linux__

// Windows-specific components
#ifdef WIN32

#include <windows.h>

#endif  // WIN32

}  // namespace util

#endif  // UTIL_HH