#pragma once

#include <common/utils.h>
#include <string>
#include <memory>
#include <db/DB.h>
#include <type_traits>
#include <utility>

struct UserInfo {
public:
    UserInfo() {}

    template<typename Email>
    UserInfo(Email&& _email): email(std::forward<Email>(_email)) {}

    template<typename Name, typename Email, typename Password>
    UserInfo(Name&& _name, Email&& _email, Password&& _password): 
        name(std::forward<Name>(_name)),
        email(std::forward<Email>(_email)),
        password(std::forward<Password>(_password)) {}

public:
    std::string name;
    std::string email;
    std::string password;
};

class Users {
public:
    Users(std::shared_ptr<DB> = nullptr);

    virtual ~Users();

    virtual bool Create(const UserInfo&);

    virtual bool Validate(const UserInfo&);

    virtual bool DuplicatedEmail(const UserInfo&);

private:
    std::shared_ptr<DB> db;
};
