/**
 * @file api.cpp
 * @author Shichen Xu
 * @brief Implementation for class API.
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <api/api.h>
#include <common/utils.h>
#include <liboauthcpp/src/base64.h>
#include <jwt/jwt.hpp>
#include <memory>

#define API_DEFINE_HTTP_HANDLER(name)\
    void API::name(const httplib::Request& req, httplib::Response& res) noexcept

#define API_ADD_HTTP_HANDLER(server, path, method, func)\
    do {\
        (server)->method((path), \
        [this](const httplib::Request& req, httplib::Response& res){this->func(req, res);});\
    } while (false)

inline void BuildHttpRespBody(nlohmann::json* js) {
    return;
}

template<typename FirstValue, typename... Rest>
inline void BuildHttpRespBody(nlohmann::json* js, const std::string& field, FirstValue&& value, Rest&&... rest) {
    (*js)[field] = value;
    BuildHttpRespBody(js, rest...);
}

#define API_RETURN_HTTP_RESP(code, ...)\
    do {\
        nlohmann::json result;\
        BuildHttpRespBody(&result, __VA_ARGS__);\
        res.status = (code);\
        res.set_content(result.dump(), "text/plain");\
        return;\
    } while (false)

static inline bool DecodeEmailAndPasswordFromBasicAuth(
    const std::string& auth, std::string* email, std::string* password) noexcept {
    if (email == nullptr || password == nullptr) {
        return false;
    }

    const auto splited_auth = Common::Split(auth, " ");
    if (splited_auth.size() != 2 || splited_auth[0] != "Basic") {
        return false;
    }

    const auto email_password = Common::Split(base64_decode(splited_auth[1]), ":");
    if (email_password.size() != 2) {
        return false;
    }

    *email = email_password[0];
    *password = email_password[1];
    return true;
}

static inline std::string EncodeTokenFromEmail(
    const std::string& email,
    const std::chrono::seconds& expire_seconds,
    const std::string& secret_key) noexcept {

    jwt::jwt_object jwt_obj{
        jwt::params::algorithm("HS256"),
        jwt::params::secret(secret_key),
        jwt::params::payload({{"email", email}})};
    
    jwt_obj.add_claim("exp", std::chrono::system_clock::now() + expire_seconds);
    return jwt_obj.signature();
}

static inline std::string DecodeEmailFromToken(
    const std::string& token,
    const std::string& secret_key) noexcept {
    
    std::error_code err;
    const auto jwt_obj = jwt::decode(
        jwt::string_view(token),
        jwt::params::algorithms({"HS256"}),
        err,
        jwt::params::secret(secret_key),
        jwt::params::verify(true));
    
    // token not valid or expired
    if (err) {
        return {};
    }
    return jwt_obj.payload().get_claim_value<std::string>("email");
}

static inline std::string DecodeTokenFromBasicAuth(const std::string& auth) noexcept {
    const auto splited_auth = Common::Split(auth, " ");
    if (splited_auth.size() != 2 || splited_auth[0] != "Basic") {
        return {};
    }

    const auto token_null = Common::Split(base64_decode(splited_auth[1]), ":");
    if (token_null.empty()) {
        return {};
    }

    return token_null[0];
}

#define API_CHECK_REQUEST_TOKEN(user_email, token)\
    do {\
        const auto auth_header = req.headers.find("Authorization");\
        if (auth_header == req.headers.cend() ||\
            (user_email = DecodeEmailFromToken(\
                token = DecodeTokenFromBasicAuth(auth_header->second), token_secret_key)\
            ).empty()) {\
            API_RETURN_HTTP_RESP(500, "msg", "failed basic auth");\
        }\
    } while (false)

/* Default arguments not cool, modify later */
API::API(std::shared_ptr<Users> _users,
      std::shared_ptr<TaskListsWorker> _tasklists_worker,
      std::shared_ptr<TasksWorker> _tasks_worker,
      std::shared_ptr<DB> _db,
      std::shared_ptr<httplib::Server> _svr):
    users(_users),  tasklists_worker(_tasklists_worker), tasks_worker(_tasks_worker), db(_db), svr(_svr),
    token_secret_key(Common::RandomString(128)) {

    if (!db) {
        db = std::make_shared<DB>();
    }

    if (!users) {
        users = std::make_shared<Users>(db);
    }

    if (!tasklists_worker) {
        tasklists_worker = std::make_shared<TaskListsWorker>(*db);
    }

    if (!tasks_worker) {
        /* Dangerous, modify later */
        tasks_worker = std::make_shared<TasksWorker>(db.get(), tasklists_worker.get());
    }
    
    if (!svr) {
        svr = std::make_shared<httplib::Server>();
    }
}

API::~API() { Stop(); }

API_DEFINE_HTTP_HANDLER(UsersRegister) {
    std::string user_name;
    std::string user_passwd;
    std::string user_email;

    const auto auth_header = req.headers.find("Authorization");
    if (auth_header == req.headers.cend() ||
        !DecodeEmailAndPasswordFromBasicAuth(auth_header->second, &user_email, &user_passwd)) {
        API_RETURN_HTTP_RESP(500, "msg", "failed basic auth");
    }

    if (user_email.empty() || user_passwd.empty()) {
        API_RETURN_HTTP_RESP(500, "msg", "failed no email or password");
    }
    
    try {
        const auto json_body = nlohmann::json::parse(req.body);
        user_name = json_body.at("name");
    } catch (std::exception& e) {
        API_RETURN_HTTP_RESP(500, "msg", "failed body format error");
    }

    // check if user email is duplicated
    if (users->DuplicatedEmail(UserInfo("", user_email, ""))) {
        API_RETURN_HTTP_RESP(500, "msg", "failed duplicated email");
    }

    // create user
    if (users->Create(UserInfo(user_name, user_email, user_passwd))) {
        API_RETURN_HTTP_RESP(200, "msg", "success");
    } else {
        API_RETURN_HTTP_RESP(500, "msg", "failed create user");
    }
}

API_DEFINE_HTTP_HANDLER(UsersLogin) {
    std::string user_passwd;
    std::string user_email;

    const auto auth_header = req.headers.find("Authorization");
    if (auth_header == req.headers.cend() ||
        !DecodeEmailAndPasswordFromBasicAuth(auth_header->second, &user_email, &user_passwd)) {
        API_RETURN_HTTP_RESP(500, "msg", "failed basic auth");
    }

    if (user_email.empty() || user_passwd.empty()) {
        API_RETURN_HTTP_RESP(500, "msg", "failed no email or password");
    }

    if (users->Validate(UserInfo("", user_email, user_passwd))) {
        const std::string token = EncodeTokenFromEmail(user_email, std::chrono::seconds(3600), token_secret_key);
        if (token.empty()) {
            API_RETURN_HTTP_RESP(500, "msg", "failed create token");
        } else {
            API_RETURN_HTTP_RESP(200, "msg", "success", "token", token);
        }
    } else {
        API_RETURN_HTTP_RESP(500, "msg", "failed user login");
    }
}

API_DEFINE_HTTP_HANDLER(UsersLogout) {
    std::string user_email;
    std::string token;
    API_CHECK_REQUEST_TOKEN(user_email, token);
    
    // invalid date the email and token
    
    API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(TaskLists) {
    const std::vector<std::string> splited_path = Common::Split(req.path, "/");

    /* Special resolve for '/v1/task_lists/{task_list_name}/tasks/{task_name}' */
    if (splited_path.size() >= 2 && *(splited_path.rbegin() + 1) == "tasks") {
        Tasks(req, res);
        return;
    }

    std::string user_email;
    std::string token;
    API_CHECK_REQUEST_TOKEN(user_email, token);

    if (Common::LowerCase(splited_path.back()) == "task_lists") {
        /* Get all task lists */
        RequestData tasklist_req;
        std::vector<std::string> out_names;
        tasklist_req.user_key = user_email;
        if (tasklists_worker->GetAllTasklist(tasklist_req, out_names) != returnCode::SUCCESS) {
            API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
        }
        nlohmann::json data;
        std::for_each(out_names.cbegin(), out_names.cend(), [&data](auto&& name) {data.push_back(name);});
        API_RETURN_HTTP_RESP(200, "msg", "success", "data", data);
    } else {
        /* Get one certain task list */
        RequestData tasklist_req;
        TasklistContent tasklist_content;
        tasklist_req.user_key = user_email;
        tasklist_req.task_key = splited_path.back();
        if (tasklists_worker->Query(tasklist_req, tasklist_content) != returnCode::SUCCESS) {
            API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
        }
        nlohmann::json data {
            {"name", tasklist_content.name},
            {"content", tasklist_content.content},
            {"date", tasklist_content.date},
        };
        API_RETURN_HTTP_RESP(200, "msg", "success", "data", data);
    }
}

API_DEFINE_HTTP_HANDLER(TaskListsCreate) {
    std::string token;
    std::string out_tasklist_name;
    RequestData tasklist_req;
    TasklistContent tasklist_content;
    API_CHECK_REQUEST_TOKEN(tasklist_req.user_key, token);

    try {
        const auto json_body = nlohmann::json::parse(req.body);
        tasklist_req.task_key = json_body.at("name");
        tasklist_content.name = json_body.at("name");
        try {
            tasklist_content.content = json_body.at("content");
            tasklist_content.date = json_body.at("date");
        } catch (...) {}
    } catch (std::exception& e) {
        API_RETURN_HTTP_RESP(500, "msg", "failed body format error");
    }

    if (tasklists_worker->Create(tasklist_req, tasklist_content, out_tasklist_name) != returnCode::SUCCESS) {
        API_RETURN_HTTP_RESP(500, "msg", "failed create tasklist");
    }

    API_RETURN_HTTP_RESP(200, "msg", "success", "name", out_tasklist_name);
}

API_DEFINE_HTTP_HANDLER(Tasks) {
    std::string user_email;
    std::string token;
    std::string task_name;
    std::string tasklist_name;
    API_CHECK_REQUEST_TOKEN(user_email, token);

    try {
        const auto request_body = nlohmann::json::parse(req.body);
        tasklist_name = request_body.at("tasklist_name");
        task_name = request_body.at("task_name");
    } catch (...) {}

    if (tasklist_name.empty()) {
        API_RETURN_HTTP_RESP(500, "msg", "failed need tasklist name");
    }

    if (task_name.empty()) {
        /* Get all tasks. */
        RequestData task_req;
        std::vector<std::string> out_names;
        task_req.user_key = user_email;
        task_req.tasklist_key = tasklist_name;
        if (tasks_worker->GetAllTasksName(task_req, out_names) != returnCode::SUCCESS) {
            API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
        }
        nlohmann::json data;
        std::for_each(out_names.cbegin(), out_names.cend(), [&data](auto&& name) {data.push_back(name);});
        API_RETURN_HTTP_RESP(200, "msg", "success", "data", data);
    } else {
        /* Get one certain task. */
        RequestData task_req;
        TaskContent task_content;
        task_req.user_key = user_email;
        task_req.task_key = task_name;
        task_req.tasklist_key = tasklist_name;
        if (tasks_worker->Query(task_req, task_content) != returnCode::SUCCESS) {
            API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
        }
        nlohmann::json data {
            {"name", task_content.name},
            {"content", task_content.content},
            {"date", task_content.date},
        };
        API_RETURN_HTTP_RESP(200, "msg", "success", "data", data);
    }
}

API_DEFINE_HTTP_HANDLER(TasksCreate) {
    std::string token;
    std::string out_task_name;
    std::string tasklist_name;
    RequestData task_req;
    TaskContent task_content;
    API_CHECK_REQUEST_TOKEN(task_req.user_key, token);

    try {
        const auto json_body = nlohmann::json::parse(req.body);
        task_req.task_key = json_body.at("task_name");
        task_req.tasklist_key = json_body.at("tasklist_name");
        task_content.name = json_body.at("task_name");
        try {
            task_content.content = json_body.at("content");
            task_content.date = json_body.at("date");
        } catch (...) {}
    } catch (std::exception& e) {
        API_RETURN_HTTP_RESP(500, "msg", "failed body format error");
    }

    if (tasks_worker->Create(task_req, task_content, out_task_name) != returnCode::SUCCESS) {
        API_RETURN_HTTP_RESP(500, "msg", "failed create tasklist");
    }

    API_RETURN_HTTP_RESP(200, "msg", "success", "name", out_task_name);
}

void API::Run(const std::string &host, uint32_t port) {
  API_ADD_HTTP_HANDLER(svr, "/v1/users/register", Post, UsersRegister);
  API_ADD_HTTP_HANDLER(svr, "/v1/users/login", Post, UsersLogin);
  API_ADD_HTTP_HANDLER(svr, "/v1/users/logout", Post, UsersLogout);
  API_ADD_HTTP_HANDLER(svr, "/v1/task_lists", Get, TaskLists);
  API_ADD_HTTP_HANDLER(svr, "/v1/task_lists/create", Post, TaskListsCreate);
  API_ADD_HTTP_HANDLER(svr, "/v1/tasks", Post, Tasks);
  API_ADD_HTTP_HANDLER(svr, "/v1/tasks/create", Post, TasksCreate);
  
    svr->Get(R"(/numbers/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string numbers = req.matches[1];
        res.set_content(numbers, "text/plain");
  });

  svr->Get("/numbers", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content("numbers", "text/plain");
  });

  svr->listen(host, port);
}

void API::Stop() {
  if (svr && svr->is_running()) {
    svr->stop();
  }
}
