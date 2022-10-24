#include <common/utils.h>
#include <db/DB.h>
#include <map>
#include <memory>
#include <string>
#include <users/users.h>

/* variable user_info_db and user_info must be accessible when using these macros */
#define USER_FIELD_PUT_TO(x) do { user_info_db[#x] = user_info.x; } while (false)
#define USER_FIELD_GET_FROM(x) do { try{ user_info.x = user_info_db.at(#x); } catch(...) {} } while (false)

using UserInfoDbType = std::map<std::string, std::string>;

static inline UserInfoDbType UserInfo2DbType(const UserInfo& user_info) {
    UserInfoDbType user_info_db;
    UTILS_CALL_MACRO_FOR_EACH(USER_FIELD_PUT_TO, name, email, passwd);
    return user_info_db;
}

static inline UserInfo DbType2UserInfo(const UserInfoDbType& user_info_db) {
    UserInfo user_info;
    UTILS_CALL_MACRO_FOR_EACH(USER_FIELD_GET_FROM, name, email, passwd);
    return user_info;
}

Users::Users(std::shared_ptr<DB> _db): db(_db) {
    if (!db) {
        db = std::make_shared<DB>();
    }
}

Users::~Users() {}

bool Users::Create(const UserInfo& user_info) {
    if (db->createUserNode(UserInfo2DbType(user_info)) != returnCode::SUCCESS) {
        return false;
    }
    return true;
};

bool Users::Validate(const UserInfo& user_info) {
    if (user_info.password.empty()) {
        return false;
    }

    UserInfoDbType user_info_db;
    if (db->getUserNode(user_info.email, user_info_db) != returnCode::SUCCESS) {
        return false;
    }

    const auto true_user_info = DbType2UserInfo(user_info_db);
    if (!user_info.name.empty() && user_info.name != true_user_info.name) {
        return false;
    }
    if (user_info.email != true_user_info.email) {
        return false;
    }
    if (user_info.password != true_user_info.password) {
        return false;
    }

    return true;
};

bool Users::DuplicatedEmail(const UserInfo& user_info) {
    UserInfoDbType user_info_db;
    if (db->getUserNode(user_info.email, user_info_db) == returnCode::SUCCESS) {
        return true;
    }
    return false;
}
