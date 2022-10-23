#pragma once

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <type_traits>

namespace Common {

/**
 * @brief Split a string into several strings against a given delimiter.
 * 
 * @param str String to be splited.
 * @param delim Delimiter.
 * @param res Result will be put there.
 */
inline void Split(const std::string& str, const std::string& delim, std::vector<std::string>* res) {
    size_t pos = 0;
    size_t next_pos;
    while ((next_pos = str.find_first_of(delim, pos)) != std::string::npos) {
        res->push_back(str.substr(pos, next_pos - pos));
        pos = next_pos + delim.size();
    }
}

/**
 * @brief Split a string into several strings against a given delimiter.
 * 
 * @param str String to be splited.
 * @param delim Delimiter.
 * @return std::vector<std::string> Result will be put there.
 */
inline std::vector<std::string> Split(const std::string& str, const std::string& delim) {
    std::vector<std::string> res;
    Split(str, delim, &res);
    return res;
}

/**
 * @brief Generate a random string, only including numbers and alphabets.
 * 
 * @param len Length of the string to be generated.
 * @return std::string 
 */
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

/**
 * @brief Judge if a type T is any form of a string type.
 * is_string<T>::value will be true for any form of strings,
 * including std::string, C style char string,
 * regardless of its constness or reference type.
 *
 * @tparam T The type to be judged.
 */
template<typename T>
struct is_string {
    using Decayed = typename std::decay<T>::type;
    static constexpr bool value = std::is_same<Decayed, std::string>::value ||
        std::is_same<Decayed, char *>::value ||
        std::is_same<Decayed, const char *>::value;
};

static constexpr bool all_true(const std::initializer_list<bool>& il) {
    for (bool val : il) {
        if (!val) return false;
    }
    return true;
}

/**
 * @brief Judge if the first type is of the same type of the rests.
 * is_same_to_all<int64_t, long, long int>::value == true
 * is_same_to_all<uint32_t, unsigned, unsigned int>::value == true
 * @tparam T First type.
 * @tparam Args Types to be judged.
 */
template<typename T, typename... Args>
struct is_same_to_all {
    static constexpr std::initializer_list<bool> values
        {std::is_same<std::decay_t<Args>, T>::value...};
    static constexpr bool value = all_true(values);
};

template<typename... Args>
struct are_all_string {
    static constexpr std::initializer_list<bool> values {is_string<Args>::value...};
    static constexpr bool value = all_true(values);
};

/* String type implementation for GetEnv */
template<typename Str, std::enable_if_t<is_string<Str>::value, bool> = true>
inline Str __GetEnv(const std::string& env) noexcept {
    char* env_str = std::getenv(env.c_str());
    if (env_str == nullptr) {
        return {};
    }
    return Str(env_str);
}

/* Integers implementation for GetEnv */
template<typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
inline Integer __GetEnv(const std::string& env) noexcept {
    try {
        return static_cast<Integer>(std::stoll(__GetEnv<std::string>(env)));
    } catch (std::exception& e) {
        return static_cast<Integer>(0);
    }
}

/* Float numbers implementation for GetEnv */
template<typename Float, std::enable_if_t<std::is_floating_point<Float>::value, bool> = true>
inline Float __GetEnv(const std::string& env) noexcept {
    try {
        return static_cast<Float>(std::stof(__GetEnv<std::string>(env)));
    } catch (std::exception& e) {
        return static_cast<Float>(0.0);
    }
}

/**
 * @brief Get the value of an environment variable in the form of a given type.
 * 
 * @tparam T The type of the environment you want the return value to be.
 * @param env The name of the environment variable.
 * @return T Return value.
 */
template<typename T>
inline T GetEnv(const std::string& env) noexcept {
    return __GetEnv<T>(env);
}

}   // namespace Common
