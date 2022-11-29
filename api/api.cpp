/**
 * @file api.cpp
 * @author Shichen Xu
 * @brief Implementation for class API.
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "api.h"
#include "base64.h"
#include "common/utils.h"
#include "db/DB.h"
#include "requestData.h"
#include "tasklistContent.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <jwt/jwt.hpp>
#include <memory>
#include <mutex>
#include <nlohmann/json_fwd.hpp>
#include <utility>

#define API_REQ() __api_req_x92k_no_conflict
#define API_RES() __api_res_s8iw_no_conflict

#define API_DEFINE_HTTP_HANDLER(name)                                          \
  void Api::name(const httplib::Request &API_REQ(),                            \
                 httplib::Response &API_RES()) noexcept

#define API_ADD_HTTP_HANDLER(server, path, method, func)                       \
  do {                                                                         \
    (server)->method((path), [this](const httplib::Request &API_REQ(),         \
                                    httplib::Response &API_RES()) {            \
      this->func(API_REQ(), API_RES());                                        \
    });                                                                        \
  } while (false)

inline void BuildHttpRespBody(nlohmann::json *js) { return; }

template <typename FirstValue, typename... Rest>
inline void BuildHttpRespBody(nlohmann::json *js, const std::string &field,
                              FirstValue &&value, Rest &&...rest) {
  (*js)[field] = std::forward<FirstValue>(value);
  BuildHttpRespBody(js, std::forward<Rest>(rest)...);
}

#define API_ADD_HTTP_OPTIONS_HANDLER(server, path)                                                                      \
  do {                                                                                                                  \
    (server)->Options((path), [this](const httplib::Request &API_REQ(),                                                 \
                                     httplib::Response &API_RES()) {                                                    \
      API_RES().set_header("Access-Control-Allow-Origin", "*");                                                               \
      API_RES().set_header("Allow", "GET, POST, PUT, DELETE, OPTIONS");                                                              \
      API_RES().set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");  \
      API_RES().set_header("Access-Control-Allow-Methods", "OPTIONS, GET, POST, HEAD");                                       \
    });                                                                                                                 \
  } while (false)                                                                                                       \

#define API_RETURN_HTTP_RESP(code, ...)                                        \
  do {                                                                         \
    nlohmann::json result;                                                     \
    BuildHttpRespBody(&result, __VA_ARGS__);                                   \
    API_RES().status = (code);                                                 \
    API_RES().set_header("Access-Control-Allow-Origin", "*");                  \
    API_RES().set_header("Access-Control-Allow-Methods", " GET, POST, PUT, DELETE, OPTIONS");                      \
    API_RES().set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");   \
    API_RES().set_content(result.dump(), "text/plain");                        \
    return;                                                                    \
  } while (false)

#define API_PARSE_REQ_BODY()                                                   \
  ({                                                                           \
    nlohmann::json json_body;                                                  \
    try {                                                                      \
      json_body = nlohmann::json::parse((API_REQ()).body);                     \
    } catch (...) {                                                            \
      API_RETURN_HTTP_RESP(500, "msg", "failed request body format error");    \
    }                                                                          \
    std::move(json_body);                                                      \
  })

#define API_GET_JSON_REQUIRED(json_body, target, field)                        \
  do {                                                                         \
    if ((json_body).find(#field) == (json_body).end()) {                       \
      API_RETURN_HTTP_RESP(500, "msg",                                         \
                           "failed missing required field " #field);           \
    }                                                                          \
    (target) = (json_body).at(#field);                                         \
  } while (false)

#define API_GET_JSON_OPTIONAL(json_body, target, field)                        \
  do {                                                                         \
    if ((json_body).find(#field) != (json_body).end()) {                       \
      (target) = (json_body).at(#field);                                       \
    }                                                                          \
  } while (false)

#define API_GET_PARAM_OPTIONAL(target, key)                                    \
  do {                                                                         \
    if (API_REQ().has_param(#key)) {                                           \
      (target) = API_REQ().get_param_value(#key);                              \
    }                                                                          \
  } while (false)

static inline bool
DecodeEmailAndPasswordFromBasicAuth(const std::string &auth, std::string *email,
                                    std::string *password) noexcept {
  if (email == nullptr || password == nullptr) {
    return false;
  }

  auto splited_auth = Common::Split(auth, " ");
  if (splited_auth.size() != 2 || splited_auth[0] != "Basic") {
    return false;
  }

  auto email_password = Common::Split(base64_decode(splited_auth[1]), ":");
  if (email_password.size() != 2) {
    return false;
  }

  *email = std::move(email_password[0]);
  *password = std::move(email_password[1]);
  return true;
}

static inline std::string
EncodeTokenFromEmail(const std::string &email,
                     const std::chrono::seconds &expire_seconds,
                     const std::string &secret_key) noexcept {

  jwt::jwt_object jwt_obj{jwt::params::algorithm("HS256"),
                          jwt::params::secret(secret_key),
                          jwt::params::payload({{"email", email}})};

  jwt_obj.add_claim("exp", std::chrono::system_clock::now() + expire_seconds);
  return jwt_obj.signature();
}

static inline std::string
DecodeEmailFromToken(const std::string &token,
                     const std::string &secret_key) noexcept {

  std::error_code err;
  const auto jwt_obj = jwt::decode(
      jwt::string_view(token), jwt::params::algorithms({"HS256"}), err,
      jwt::params::secret(secret_key), jwt::params::verify(true));

  // token not valid or expired
  if (err) {
    return {};
  }
  return jwt_obj.payload().get_claim_value<std::string>("email");
}

static inline std::string
DecodeTokenFromBasicAuth(const std::string &auth) noexcept {
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

#define API_CHECK_REQUEST_TOKEN(user_email, token)                             \
  do {                                                                         \
    const auto auth_header = API_REQ().headers.find("Authorization");          \
    if (auth_header == API_REQ().headers.cend() ||                             \
        (user_email = DecodeEmailFromToken(                                    \
             token = DecodeTokenFromBasicAuth(auth_header->second),            \
             token_secret_key))                                                \
            .empty()) {                                                        \
      API_RETURN_HTTP_RESP(500, "msg", "failed basic auth");                   \
    }                                                                          \
    std::lock_guard<std::mutex> guard(invalid_tokens_lock);                    \
    if (invalid_tokens.find(token) != invalid_tokens.end()) {                  \
      API_RETURN_HTTP_RESP(500, "msg", "failed token invalid");                \
    }                                                                          \
  } while (false)

/* Default arguments not cool, modify later */
Api::Api(std::shared_ptr<Users> _users,
         std::shared_ptr<TaskListsWorker> _tasklists_worker,
         std::shared_ptr<TasksWorker> _tasks_worker, std::shared_ptr<DB> _db,
         std::shared_ptr<httplib::Server> _svr)
    : users(_users), tasklists_worker(_tasklists_worker),
      tasks_worker(_tasks_worker), db(_db), svr(_svr),
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
    tasks_worker =
        std::make_shared<TasksWorker>(db.get(), tasklists_worker.get());
  }

  if (!svr) {
    svr = std::make_shared<httplib::Server>();
  }
}

Api::~Api() { Stop(); }

API_DEFINE_HTTP_HANDLER(UsersRegister) {
  std::string user_name;
  std::string user_passwd;
  std::string user_email;
  nlohmann::json json_body;

  const auto auth_header = API_REQ().headers.find("Authorization");
  if (auth_header == API_REQ().headers.cend() ||
      !DecodeEmailAndPasswordFromBasicAuth(auth_header->second, &user_email,
                                           &user_passwd)) {
    API_RETURN_HTTP_RESP(500, "msg", "failed basic auth");
  }

  if (user_email.empty() || user_passwd.empty()) {
    API_RETURN_HTTP_RESP(500, "msg", "failed no email or password");
  }

  json_body = API_PARSE_REQ_BODY();
  API_GET_JSON_OPTIONAL(json_body, user_name, name);

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

  const auto auth_header = API_REQ().headers.find("Authorization");
  if (auth_header == API_REQ().headers.cend() ||
      !DecodeEmailAndPasswordFromBasicAuth(auth_header->second, &user_email,
                                           &user_passwd)) {
    API_RETURN_HTTP_RESP(500, "msg", "failed basic auth");
  }

  if (user_email.empty() || user_passwd.empty()) {
    API_RETURN_HTTP_RESP(500, "msg", "failed no email or password");
  }

  if (users->Validate(UserInfo("", user_email, user_passwd))) {
    const std::string token = EncodeTokenFromEmail(
        user_email, std::chrono::seconds(3600), token_secret_key);
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

  /* invalid date the email and token */
  {
    std::lock_guard<std::mutex> guard(invalid_tokens_lock);
    invalid_tokens.insert(token);
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(TaskListsAll) {
  std::string token;
  std::string share;
  RequestData tasklist_req;
  std::vector<std::string> out_names;
  std::vector<shareInfo> out_share_info;
  nlohmann::json data;

  API_CHECK_REQUEST_TOKEN(tasklist_req.user_key, token);
  API_GET_PARAM_OPTIONAL(share, share);

  if (share == "true") {
    /* Get all shared task lists */
    if (tasklists_worker->GetAllAccessTaskList(tasklist_req, out_share_info) !=
        returnCode::SUCCESS) {
      API_RETURN_HTTP_RESP(500, "msg", "failed get shared task lists");
    }
    std::transform(out_share_info.begin(), out_share_info.end(),
                   std::back_inserter(data), [](shareInfo &info) {
                     return nlohmann::json{
                         {"user", std::move(info.user_name)},
                         {"permission", info.permission ? "write" : "read"},
                         {"list", info.task_list_name}};
                   });
  } else {
    /* Get all task lists */
    if (tasklists_worker->GetAllTasklist(tasklist_req, out_names) !=
        returnCode::SUCCESS) {
      API_RETURN_HTTP_RESP(500, "msg", "failed get all task lists");
    }
    std::for_each(out_names.cbegin(), out_names.cend(),
                  [&data](auto &name) { data.push_back(std::move(name)); });
  }

  API_RETURN_HTTP_RESP(200, "msg", "success", "data", std::move(data));
}

API_DEFINE_HTTP_HANDLER(TaskListsGet) {
  std::string token;
  RequestData tasklist_req;
  TasklistContent tasklist_content;
  nlohmann::json data;

  API_CHECK_REQUEST_TOKEN(tasklist_req.user_key, token);

  /* Get one certain task list */
  API_GET_PARAM_OPTIONAL(tasklist_req.other_user_key, other);
  tasklist_req.tasklist_key = API_REQ().matches[1];
  if (tasklists_worker->Query(tasklist_req, tasklist_content) !=
      returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
  }
  data = {{"name", std::move(tasklist_content.name)},
          {"content", std::move(tasklist_content.content)},
          {"date", std::move(tasklist_content.date)},
          {"visibility", std::move(tasklist_content.visibility)}};
  API_RETURN_HTTP_RESP(200, "msg", "success", "data", std::move(data));
}

API_DEFINE_HTTP_HANDLER(TaskListsUpdate) {
  std::string token;
  RequestData tasklist_req;
  TasklistContent tasklist_content;
  std::string optional_name;
  nlohmann::json json_body;

  API_CHECK_REQUEST_TOKEN(tasklist_req.user_key, token);

  tasklist_req.tasklist_key = API_REQ().matches[1];
  json_body = API_PARSE_REQ_BODY();

  API_GET_JSON_OPTIONAL(json_body, optional_name, name);
  API_GET_PARAM_OPTIONAL(tasklist_req.other_user_key, other);

  if (!optional_name.empty() && optional_name != tasklist_req.tasklist_key) {
    API_RETURN_HTTP_RESP(500, "msg", "failed tasklist name can not be changed");
  }

  API_GET_JSON_OPTIONAL(json_body, tasklist_content.content, content);
  API_GET_JSON_OPTIONAL(json_body, tasklist_content.date, date);
  API_GET_JSON_OPTIONAL(json_body, tasklist_content.visibility, visibility);

  if (tasklists_worker->Revise(tasklist_req, tasklist_content) !=
      returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed update tasklist");
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(TaskListsDelete) {
  std::string token;
  RequestData tasklist_req;

  API_CHECK_REQUEST_TOKEN(tasklist_req.user_key, token);

  tasklist_req.tasklist_key = API_REQ().matches[1];

  if (tasklists_worker->Delete(tasklist_req) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed delete tasklist");
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(TaskListsCreate) {
  std::string token;
  std::string out_tasklist_name;
  RequestData tasklist_req;
  TasklistContent tasklist_content;
  nlohmann::json json_body;

  API_CHECK_REQUEST_TOKEN(tasklist_req.user_key, token);

  json_body = API_PARSE_REQ_BODY();

  API_GET_JSON_REQUIRED(json_body, tasklist_req.tasklist_key, name);
  API_GET_JSON_REQUIRED(json_body, tasklist_content.name, name);
  API_GET_JSON_OPTIONAL(json_body, tasklist_content.content, content);
  API_GET_JSON_OPTIONAL(json_body, tasklist_content.date, date);
  API_GET_JSON_OPTIONAL(json_body, tasklist_content.visibility, visibility);

  if (tasklists_worker->Create(tasklist_req, tasklist_content,
                               out_tasklist_name) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed create tasklist");
  }

  API_RETURN_HTTP_RESP(200, "msg", "success", "name", out_tasklist_name);
}

API_DEFINE_HTTP_HANDLER(TasksAll) {
  std::string token;
  RequestData task_req;
  std::vector<std::string> out_names;

  API_CHECK_REQUEST_TOKEN(task_req.user_key, token);
  API_GET_PARAM_OPTIONAL(task_req.other_user_key, other);

  task_req.tasklist_key = API_REQ().matches[1];

  /* Get all tasks. */
  if (tasks_worker->GetAllTasksName(task_req, out_names) !=
      returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
  }
  nlohmann::json data;
  std::for_each(out_names.cbegin(), out_names.cend(),
                [&data](auto &name) { data.push_back(std::move(name)); });
  API_RETURN_HTTP_RESP(200, "msg", "success", "data", std::move(data));
}

API_DEFINE_HTTP_HANDLER(TasksGet) {
  std::string token;
  RequestData task_req;
  TaskContent task_content;
  nlohmann::json data;

  API_CHECK_REQUEST_TOKEN(task_req.user_key, token);
  API_GET_PARAM_OPTIONAL(task_req.other_user_key, other);

  task_req.tasklist_key = API_REQ().matches[1];
  task_req.task_key = API_REQ().matches[2];

  if (task_req.tasklist_key.empty()) {
    API_RETURN_HTTP_RESP(500, "msg", "failed need tasklist name");
  }

  /* Get one certain task. */
  if (tasks_worker->Query(task_req, task_content) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
  }
  data = {{"name", std::move(task_content.name)},
          {"content", std::move(task_content.content)},
          {"date", std::move(task_content.date)},
          {"start_date", std::move(task_content.startDate)},
          {"end_date", std::move(task_content.endDate)},
          {"priority", task_content.priority},
          {"status", std::move(task_content.status)}};
  API_RETURN_HTTP_RESP(200, "msg", "success", "data", std::move(data));
}

API_DEFINE_HTTP_HANDLER(TasksUpdate) {
  std::string token;
  RequestData task_req;
  TaskContent task_content;
  nlohmann::json json_body;
  std::string optional_name;

  API_CHECK_REQUEST_TOKEN(task_req.user_key, token);
  API_GET_PARAM_OPTIONAL(task_req.other_user_key, other);

  task_req.task_key = API_REQ().matches[2];
  task_req.tasklist_key = API_REQ().matches[1];

  if (task_req.tasklist_key.empty()) {
    API_RETURN_HTTP_RESP(500, "msg", "failed need tasklist name");
  }

  json_body = API_PARSE_REQ_BODY();
  API_GET_JSON_OPTIONAL(json_body, optional_name, name);

  if (!optional_name.empty() && optional_name != task_req.task_key) {
    API_RETURN_HTTP_RESP(500, "msg", "failed task name can not be changed");
  }

  API_GET_JSON_OPTIONAL(json_body, task_content.content, content);
  API_GET_JSON_OPTIONAL(json_body, task_content.date, date);
  API_GET_JSON_OPTIONAL(json_body, task_content.startDate, start_date);
  API_GET_JSON_OPTIONAL(json_body, task_content.endDate, end_date);
  API_GET_JSON_OPTIONAL(json_body, task_content.priority, priority);
  API_GET_JSON_OPTIONAL(json_body, task_content.status, status);

  if (tasks_worker->Revise(task_req, task_content) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(TasksDelete) {
  std::string token;
  RequestData task_req;

  API_CHECK_REQUEST_TOKEN(task_req.user_key, token);
  API_GET_PARAM_OPTIONAL(task_req.other_user_key, other);

  task_req.task_key = API_REQ().matches[2];
  task_req.tasklist_key = API_REQ().matches[1];

  if (task_req.tasklist_key.empty()) {
    API_RETURN_HTTP_RESP(500, "msg", "failed need tasklist name");
  }

  if (tasks_worker->Delete(task_req) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed internal server error");
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(TasksCreate) {
  std::string token;
  std::string out_task_name;
  RequestData task_req;
  TaskContent task_content;
  nlohmann::json json_body;

  API_CHECK_REQUEST_TOKEN(task_req.user_key, token);
  API_GET_PARAM_OPTIONAL(task_req.other_user_key, other);

  task_req.tasklist_key = API_REQ().matches[1];
  json_body = API_PARSE_REQ_BODY();

  API_GET_JSON_REQUIRED(json_body, task_req.task_key, name);
  API_GET_JSON_REQUIRED(json_body, task_content.name, name);
  API_GET_JSON_OPTIONAL(json_body, task_content.content, content);
  API_GET_JSON_OPTIONAL(json_body, task_content.date, date);
  API_GET_JSON_OPTIONAL(json_body, task_content.startDate, start_date);
  API_GET_JSON_OPTIONAL(json_body, task_content.endDate, end_date);
  API_GET_JSON_OPTIONAL(json_body, task_content.priority, priority);
  API_GET_JSON_OPTIONAL(json_body, task_content.status, status);

  if (tasks_worker->Create(task_req, task_content, out_task_name) !=
      returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed create task");
  }

  API_RETURN_HTTP_RESP(200, "msg", "success", "name", out_task_name);
}

API_DEFINE_HTTP_HANDLER(ShareGet) {
  std::string token;
  RequestData share_info_req;
  bool is_public;
  std::vector<shareInfo> share_info;
  nlohmann::json data;

  API_CHECK_REQUEST_TOKEN(share_info_req.user_key, share_info_req.tasklist_key);
  share_info_req.tasklist_key = API_REQ().matches[1];

  if (tasklists_worker->GetAllGrantTaskList(share_info_req, share_info,
                                            is_public) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg", "failed get share info");
  }

  if (is_public) {
    API_RETURN_HTTP_RESP(200, "msg", "success", "data", "task list is public");
  }

  std::transform(share_info.begin(), share_info.end(), std::back_inserter(data),
                 [](shareInfo &info) {
                   return nlohmann::json{
                       {"user", std::move(info.user_name)},
                       {"permission", info.permission ? "write" : "read"}};
                 });

  API_RETURN_HTTP_RESP(200, "msg", "success", "data", std::move(data));
}

API_DEFINE_HTTP_HANDLER(ShareCreate) {
  std::string token;
  RequestData share_create_req;
  std::vector<shareInfo> share_info;
  std::string err_user;
  nlohmann::json json_body;
  nlohmann::json user_permission;

  API_CHECK_REQUEST_TOKEN(share_create_req.user_key, token);
  json_body = API_PARSE_REQ_BODY();
  share_create_req.tasklist_key = API_REQ().matches[1];

  API_GET_JSON_REQUIRED(json_body, user_permission, user_permission);

  if (!user_permission.is_array()) {
    API_RETURN_HTTP_RESP(500, "msg", "failed user_permission must be array");
  }

  /* Do not want to use for-loop, but API_GET_JSON_REQUIRED can not be used in
   * lambda. * Bad Bad C++. */
  for (auto &json_entry : user_permission) {
    share_info.emplace_back();
    share_info.back().task_list_name = API_REQ().matches[1];
    API_GET_JSON_REQUIRED(json_entry, share_info.back().user_name, user);
    API_GET_JSON_REQUIRED(json_entry, share_info.back().permission, permission);
  }

  if (tasklists_worker->ReviseGrantTaskList(share_create_req, share_info,
                                            err_user) != returnCode::SUCCESS) {
    API_RETURN_HTTP_RESP(500, "msg",
                         err_user.empty()
                             ? "falied all users"
                             : "failed last updated user is " + err_user);
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(ShareDelete) {
  std::string token;
  RequestData share_delete_req;
  std::string err_user;
  std::vector<std::string> user_str_list;
  nlohmann::json json_body;
  nlohmann::json user_json_list;

  API_CHECK_REQUEST_TOKEN(share_delete_req.user_key, token);

  share_delete_req.tasklist_key = API_REQ().matches[1];
  json_body = API_PARSE_REQ_BODY();
  API_GET_JSON_REQUIRED(json_body, user_json_list, user_list);
  API_GET_PARAM_OPTIONAL(share_delete_req.other_user_key, other);

  std::transform(user_json_list.cbegin(), user_json_list.cend(),
                 std::back_inserter(user_str_list), [](auto &&x) { return x; });

  if (!share_delete_req.other_user_key.empty()) {
    if (tasklists_worker->RemoveGrantTaskList(share_delete_req) !=
        returnCode::SUCCESS) {
      API_RETURN_HTTP_RESP(500, "msg", "failed delete sharing");
    }
  } else {
    if (tasklists_worker->RemoveGrantTaskList(
            share_delete_req, user_str_list, err_user) != returnCode::SUCCESS) {
      API_RETURN_HTTP_RESP(500, "msg",
                           err_user.empty()
                               ? "failed all users"
                               : "failed last deleted user is " + err_user);
    }
  }

  API_RETURN_HTTP_RESP(200, "msg", "success");
}

API_DEFINE_HTTP_HANDLER(Health) {
  try {
    std::string numbers = API_REQ().matches[1];
    API_RETURN_HTTP_RESP(200, "msg", "success", "data", numbers);
  } catch (...) {
    API_RETURN_HTTP_RESP(200, "msg", "success");
  }
}

void Api::Run(const std::string &host, uint32_t port) {
  API_ADD_HTTP_HANDLER(svr, "/v1/users/register", Post, UsersRegister);
  API_ADD_HTTP_HANDLER(svr, "/v1/users/login", Post, UsersLogin);
  API_ADD_HTTP_HANDLER(svr, "/v1/users/logout", Post, UsersLogout);
  API_ADD_HTTP_HANDLER(svr, "/v1/task_lists", Get, TaskListsAll);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+))", Get, TaskListsGet);
  API_ADD_HTTP_HANDLER(svr, "/v1/task_lists/create", Post, TaskListsCreate);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+))", Post,
                       TaskListsUpdate);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+))", Delete,
                       TaskListsDelete);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+)/tasks)", Get, TasksAll);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+)/tasks/([^\/]+))", Get,
                       TasksGet);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+)/tasks/create)", Post,
                       TasksCreate);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+)/tasks/([^\/]+))", Post,
                       TasksUpdate);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/task_lists/([^\/]+)/tasks/([^\/]+))", Delete,
                       TasksDelete);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/share/([^\/]+))", Get, ShareGet);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/share/([^\/]+)/create)", Post, ShareCreate);
  API_ADD_HTTP_HANDLER(svr, R"(/v1/share/([^\/]+))", Delete, ShareDelete);
  API_ADD_HTTP_HANDLER(svr, R"(/health/(\d+))", Get, Health);
  
  API_ADD_HTTP_OPTIONS_HANDLER(svr, R"(/.*)");
  svr->listen(host, port);
}

void Api::Stop() {
  if (svr && svr->is_running()) {
    svr->stop();
  }
}

#undef API_ADD_HTTP_HANDLER
#undef API_DEFINE_HTTP_HANDLER
#undef API_RETURN_HTTP_RESP
#undef API_CHECK_REQUEST_TOKEN
#undef API_GET_JSON_REQUIRED
#undef API_GET_JSON_OPTIONAL
#undef API_GET_PARAM_OPTIONAL
#undef API_PARSE_REQ_BODY
#undef API_GET_OPTIONAL_FROM_REQ_HEADER
#undef API_REQ
#undef API_RES
