#pragma once

#include <string>
#include <memory>
#include <db/DB.h>

class Users {
public:
    Users(std::shared_ptr<DB> = nullptr);

    virtual ~Users();

    virtual bool Create(const std::string& name, const std::string& email, const std::string& password);

    virtual bool Validate(const std::string& name, const std::string& email, const std::string& password);

    virtual bool DuplicatedEmail(const std::string& email);
};
