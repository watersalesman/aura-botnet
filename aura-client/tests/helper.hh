#include <picosha2.h>
#include <fstream>
#include <string>

bool CreateTestFile(const std::string& filename) {
    std::ofstream file_stream(filename, std::ios::trunc);
    file_stream << "Test Content";

    return file_stream.good();
}

bool FileExists(const std::string& filename) {
    std::ifstream file_stream(filename);
    return file_stream.good();
}

std::string GetFileHash(std::ifstream& file_stream) {
    std::vector<unsigned char> hash(32);
    picosha2::hash256(std::istreambuf_iterator<char>(file_stream),
                      std::istreambuf_iterator<char>(), hash.begin(),
                      hash.end());
    return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}
