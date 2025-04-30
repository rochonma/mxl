#include "Utils.hpp"
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <stdexcept>
#include <string>

std::string mxl::tests::readFile(std::filesystem::path const& filepath)
{
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}