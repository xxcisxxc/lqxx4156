#include <memory>
#include <users/users.h>

Users::Users(std::shared_ptr<DB> _db): db(_db) {
    if (!db) {
        db = std::make_shared<DB>();
    }
}

Users::~Users() {}

bool Users::Create(const UserInfo& user_info) {
    
    return true;
};

bool Users::Validate(const UserInfo& user_info) {
    // just for avoiding warnings
    return true;
};

bool Users::DuplicatedEmail(const UserInfo& user_info) {
    // just for avoiding warnings
    return false;
}
