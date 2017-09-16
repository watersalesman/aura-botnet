#include <string>
#include <fstream>
#include <sys/stat.h>

#include "picosha2.h"

const int NUM_LINES_SEED = 10000;

namespace seed {


class Seed {
    public:
        Seed (std::string filePath) { path_ = filePath; }

        bool exists();
        void initSeed();
        void getSeed();
        std::string getHash () { return hash_; }

    private:
        std::string path_, hash_;

        std::string genSeedData();
        void calcHash(std::string);
        void calcHash(std::ifstream&);
};


bool Seed::exists ()
{
    struct stat buf;
    return (stat (path_.c_str(), &buf) == 0);
}


std::string Seed::genSeedData ()
{
    std::string line, data;
    std::ifstream urandom ("/dev/urandom", std::ios::binary);
    if ( urandom.is_open() ) {
        for ( int i = 0; i < NUM_LINES_SEED; i++ ) {
            std::getline (urandom, line);
            data += line;
        }
    }

    return data;
}


void Seed::initSeed ()
{
    std::ofstream seedFile (path_, std::ios::binary|std::ios::trunc);
    if ( seedFile.is_open() ) {
        std::string seedData = genSeedData();
        seedFile << seedData;
        calcHash(seedData);
    }

}


void Seed::getSeed ()
{
    if ( exists() ) {
        std::ifstream seedFile (path_, std::ios::binary);
        if ( seedFile.is_open() ) {
            calcHash(seedFile);
        }
    }
}


void Seed::calcHash (std::string str)
{
    hash_ = picosha2::hash256_hex_string(str);
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
    hash_ = hex_str;
}


} //namespace seed
