#ifndef AUTHFILE_HH
#define AUTHFILE_HH

#include "picosha2.h"

#include <experimental/filesystem>
#include <fstream>
#include <random>
#include <string>

namespace fs = std::experimental::filesystem;

class AuthFile {
   public:
    AuthFile(const fs::path& file_path) { path_ = file_path; }
    bool Exists();
    void Init();
    void Retrieve();
    std::string GetHash() { return hash_; }

   private:
    fs::path path_;
    std::string hash_;

    void CalcHash_(const std::string&);
    void CalcHash_(std::ifstream&);
};

#endif  // AUTHFILE_HH