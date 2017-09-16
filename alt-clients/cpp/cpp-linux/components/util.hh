#ifndef UTIL_HH
#define UTIL_HH

#include <string>
#include <cstdlib>
#include <stdio.h>

namespace util {


std::string getCmdOutput (std::string cmd) {
    FILE *pipe;
    char buf[512];
    std::string output;

    if ( !(pipe = popen(cmd.c_str(), "r")) ) {
        std::exit(1);
    }

    while ( fgets(buf, sizeof(buf), pipe) != NULL ) {
        output += buf;
    }
    pclose(pipe);

    return output;
}


bool isRoot () {
    bool hasRoot;
    std::string cmd = "id -u | tr -d '\n'";
    std::string uid = getCmdOutput(cmd);

    if ( uid == "0" ) {
        hasRoot = true;
    } else {
        hasRoot = false;
    }

    return hasRoot;
}


} // namespace util

#endif // UTIL_HH
