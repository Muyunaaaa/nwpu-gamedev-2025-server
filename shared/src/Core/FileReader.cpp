#include "Core/FileReader.hpp"

#include <fstream>

MOE_BEGIN_NAMESPACE

FileReader* FileReader::s_instance = nullptr;

Optional<Vector<uint8_t>> DefaultFileReader::readFile(
        StringView filename, size_t& outFileSize) {
    std::ifstream file(std::string(filename),
                       std::ios::binary |
                               std::ios::ate |
                               std::ios::in);

    if (!file.is_open()) {
        //moe::Logger::error("Failed to open file: {}", filename);
        return std::nullopt;
    }

    outFileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    auto buffer = Vector<uint8_t>{};
    buffer.resize(outFileSize);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), outFileSize)) {
        //todo
        //::error("Failed to read file: {}", filename);
        return std::nullopt;
    }

    return Vector<uint8_t>(std::move(buffer));
}

MOE_END_NAMESPACE