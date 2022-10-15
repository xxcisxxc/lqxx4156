#pragma once

#include <string>
#include <vector>

namespace Common {

inline void Split(const std::string& str, const std::string& delim, std::vector<std::string>* res) {
    size_t pos = 0;
    size_t next_pos;
    while ((next_pos = str.find_first_of(delim, pos)) != std::string::npos) {
        res->push_back(str.substr(pos, next_pos - pos));
        pos = next_pos + delim.size();
    }
}

inline std::vector<std::string> Split(const std::string& str, const std::string& delim) {
    std::vector<std::string> res;
    Split(str, delim, &res);
    return res;
}

inline std::string RandomString(const size_t len) {
    static const std::string alphanum = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    srand((unsigned)time(nullptr) * getpid());

    std::string res(len, '\0');
    for (size_t i = 0; i < len; ++i) {
        res[i] = alphanum[rand() % alphanum.size()];
    }
    return res;
}

}   // namespace Common
