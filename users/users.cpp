#include <users/users.h>

Users::Users() {}

Users::~Users() {}

bool Users::Create(const std::string &name, const std::string &email,
                   const std::string &password) {
  // to be implemented, just for avoiding warnings
  return true;
};

bool Users::Validate(const std::string &name, const std::string &email,
                     const std::string &password) {
  // just for avoiding warnings
  return true;
};

bool Users::DuplicatedEmail(const std::string &email) {
  // just for avoiding warnings
  return false;
}
