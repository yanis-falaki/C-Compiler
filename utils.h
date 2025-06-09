#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Utils {

inline std::string readFile(const fs::path& filePath) {
    std::ifstream file {filePath};
    if (!file) {
        throw std::runtime_error("Failed to read file: " + filePath.string());
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

/// Outputs a substring centered on a particular position of a string_view.
/// 
/// @param sv Input string_view.
/// @param pos Position to be centered on.
/// @param window_size Number of characters on either side of the center position.
/// @return Centered string.
inline std::string stringCenteredOnPos(std::string_view sv, size_t pos, int32_t window_size=30) {
    auto len = sv.size();
    int start_index = std::max(0, int(pos) - window_size);
    int left_taken = int(pos) - start_index;
    int right_pad = window_size - left_taken;
    int end_index = std::min(int(len), int(pos) + window_size + right_pad);

    return std::string(sv.substr(start_index, end_index));
}

}