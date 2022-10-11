#pragma once

#include <string>

class Users {
public:
    Users() {}

    virtual ~Users() {}

    virtual bool Create(const std::string& name, const std::string& email, const std::string& password) {
        // to be implemented, just for avoiding warnings
        return true;
    };

    virtual bool Validate(const std::string& name, const std::string& email, const std::string& password) {
        // just for avoiding warnings
        return true;
    };
};
