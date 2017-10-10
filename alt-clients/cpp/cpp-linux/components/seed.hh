#include <string>
#include <fstream>
#include <random>

#include "picosha2.h"

const int SEED_RNG_ITERATIONS = 1000000;


namespace seed {


class Seed {
    public:
        Seed (std::string filePath) { _path = filePath; }

        bool exists();
        void initSeed();
        void getSeed();
        std::string getHash () { return _hash; }

    private:
        std::string _path, _hash;

        void calcHash(std::string);
        void calcHash(std::ifstream&);
};


bool Seed::exists ()
{
    std::ifstream inFile;
    return (inFile.good());
}


std::string genSeedData() {
    // Use <random> header for portability
    std::string seedData;
    std::random_device randomDev;
    std::mt19937 randNum(randomDev());
    for (int i=0; i < SEED_RNG_ITERATIONS; ++i) {
        seedData.push_back( (char)randNum() );
    }

    return seedData;
}


void Seed::initSeed ()
{
    std::string seedData;
    std::ofstream seedFile (_path, std::ios::binary|std::ios::trunc);
    if ( seedFile.is_open() ) {
        seedData = genSeedData();
        seedFile << seedData;
    }

    calcHash(seedData);
}


void Seed::getSeed ()
{
    if ( exists() ) {
        std::ifstream seedFile (_path, std::ios::binary);
        if ( seedFile.is_open() ) {
            calcHash(seedFile);
        }
    }
}


void Seed::calcHash (std::string str)
{
    _hash = picosha2::hash256_hex_string(str);
}


void Seed::calcHash (std::ifstream& seedFile)
{
    // This method may use less memory than getting hex_str from string
    std::vector<unsigned char> hash(32);
    picosha2::hash256(
            std::istreambuf_iterator<char>(seedFile),
            std::istreambuf_iterator<char>(),
            hash.begin(),
            hash.end()
    );
    std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
    _hash = hex_str;
}


} //namespace seed
