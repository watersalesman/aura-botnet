#ifndef AUTHFILE_HH
#define AUTHFILE_HH

#include "picosha2.h"

#include <fstream>
#include <random>
#include <string>

const int RNG_ITERATIONS = 1000000;

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

bool AuthFile::Exists() {
    std::ifstream infile(path_);
    return (infile.good());
}

// Use <random> header for portability
std::string GenerateData(int rng_num_iter) {
    std::string random_data;
    std::random_device random_dev;
    std::mt19937 random_num(random_dev());
    for (int i = 0; i < rng_num_iter; ++i) {
        random_data.push_back((char)random_num());
    }

    return random_data;
}

void AuthFile::Init() {
    std::string random_data;
    std::ofstream file_stream(path_, std::ios::binary | std::ios::trunc);
    if (file_stream.is_open()) {
        random_data = GenerateData(RNG_ITERATIONS);
        file_stream << random_data;
    }

    CalcHash_(random_data);
}

void AuthFile::Retrieve() {
    if (Exists()) {
        std::ifstream file_stream(path_, std::ios::binary);
        if (file_stream.is_open()) CalcHash_(file_stream);
    }
}

void AuthFile::CalcHash_(const std::string& str) {
    hash_ = picosha2::hash256_hex_string(str);
}

// This method may use less memory than getting hex_str from string
void AuthFile::CalcHash_(std::ifstream& file_stream) {
    std::vector<unsigned char> hash(32);
    picosha2::hash256(std::istreambuf_iterator<char>(file_stream),
                      std::istreambuf_iterator<char>(), hash.begin(),
                      hash.end());
    hash_ = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

#endif  // AUTHFILE_HH