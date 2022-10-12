#pragma once

#include <string>
#include <vector>

namespace Utils {

void Split(const std::string& str, const std::string& delim, std::vector<std::string>* res) {
    size_t pos = 0;
    size_t next_pos;
    while ((next_pos = str.find_first_of(delim, pos)) != std::string::npos) {
        res->push_back(str.substr(pos, next_pos - pos));
        pos = next_pos + delim.size();
    }
}

std::vector<std::string> Split(const std::string& str, const std::string& delim) {
    std::vector<std::string> res;
    Split(str, delim, &res);
    return res;
}

}   // namespace utils
