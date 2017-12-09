#include "picosha2.h"

#include <fstream>
#include <random>
#include <string>

class Seed {
   public:
    Seed(const std::string& file_path) { path_ = file_path; }
    bool Exists();
    void InitSeed();
    void GetSeed();
    std::string GetHash() { return hash_; }

   private:
    std::string path_, hash_;

    void CalcHash(const std::string&);
    void CalcHash(std::ifstream&);
};

bool Seed::Exists() {
    std::ifstream infile(path_);
    return (infile.good());
}

/* Define Command member functions */

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

void Seed::InitSeed() {
    std::string random_data;
    std::ofstream seed_file(path_, std::ios::binary | std::ios::trunc);
    if (seed_file.is_open()) {
        random_data = GenerateData(SEED_RNG_ITERATIONS);
        seed_file << random_data;
    }

    CalcHash(random_data);
}

void Seed::GetSeed() {
    if (Exists()) {
        std::ifstream seed_file(path_, std::ios::binary);
        if (seed_file.is_open()) CalcHash(seed_file);
    }
}

void Seed::CalcHash(const std::string& str) {
    hash_ = picosha2::hash256_hex_string(str);
}

// This method may use less memory than getting hex_str from string
void Seed::CalcHash(std::ifstream& seed_file) {
    std::vector<unsigned char> hash(32);
    picosha2::hash256(std::istreambuf_iterator<char>(seed_file),
                      std::istreambuf_iterator<char>(), hash.begin(),
                      hash.end());
    hash_ = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}
