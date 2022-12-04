/**
 * @file users.h
 * @author Shichen Xu
 * @brief Users' business logic.
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "common/utils.h"
#include "db/DB.h"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

/**
 * @brief User information, mainly for passing arguments conveniently.
 */
struct UserInfo {
public:
  UserInfo() {}

  UserInfo(std::string _email) : email(std::move(_email)) {}

  UserInfo(std::string _name, std::string _email, std::string _password)
      : name(std::move(_name)), email(std::move(_email)),
        passwd(std::move(_password)) {}

  bool operator==(const UserInfo &info) const {
    return this->name == info.name && this->email == info.email &&
           this->passwd == info.passwd;
  }

public:
  std::string name;
  std::string email;
  std::string passwd;
};

/**
 * @brief Users.
 *
 */
class Users {
public:
  Users(std::shared_ptr<DB> = nullptr);

  virtual ~Users();

  virtual bool Create(const UserInfo &);

  virtual bool Validate(const UserInfo &);

  virtual bool DuplicatedEmail(const UserInfo &);

  virtual bool Delete(const UserInfo &);

private:
  std::shared_ptr<DB> db;
};
