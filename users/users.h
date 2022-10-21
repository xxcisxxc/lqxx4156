#pragma once

#include <string>

class Users {
public:
    Users();

    virtual ~Users();

    virtual bool Create(const std::string& name, const std::string& email, const std::string& password);

    virtual bool Validate(const std::string& name, const std::string& email, const std::string& password);

    virtual bool DuplicatedEmail(const std::string& email);
};
