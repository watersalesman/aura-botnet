#ifndef AUTHFILE_HH
#define AUTHFILE_HH

#include "picosha2.h"

#include <fstream>
#include <random>
#include <string>

class AuthFile {
   public:
    AuthFile(const std::string& file_path) { path_ = file_path; }
    bool Exists();
    void Init();
    void Retrieve();
    std::string GetHash() { return hash_; }

   private:
    std::string path_, hash_;

    void CalcHash_(const std::string&);
    void CalcHash_(std::ifstream&);
};

#endif  // AUTHFILE_HH