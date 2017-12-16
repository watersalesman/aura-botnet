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