#include "api/api.h"
#include "api/tasklistContent.h"
#include "db/DB.h"
#include "tasklists/tasklistsWorker.h"
#include "tasks/tasksWorker.h"
#include "users/users.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class MockedUsers : public Users {
public:
  ~MockedUsers() {}

  bool Create(const UserInfo &) override { return create_mock_return; }

  bool Validate(const UserInfo &) override { return validate_mock_return; }

  bool DuplicatedEmail(const UserInfo &) override {
    return duplicatedemail_return;
  }

  void SetCreateResult(bool value) { create_mock_return = value; }

  void SetValidateResult(bool value) { validate_mock_return = value; }

  void SetDuplicatedemaiResult(bool value) { duplicatedemail_return = value; }

private:
  bool create_mock_return;
  bool validate_mock_return;
  bool duplicatedemail_return;
};

class MockedTasksWorker;

class MockedTasklistsWorker : public TaskListsWorker {

  friend MockedTasksWorker;

public:
  MockedTasklistsWorker(std::shared_ptr<DB> _db, std::shared_ptr<Users> _users)
      : TaskListsWorker(_db, _users) {}

  ~MockedTasklistsWorker() {}

  returnCode Query(const RequestData &data, TasklistContent &out) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      auto share_it = mocked_share[data.other_user_key].find(data.tasklist_key);
      if (share_it == mocked_share[data.other_user_key].end()) {
        return returnCode::ERR_NO_NODE;
      }
      if (std::find_if(share_it->second.begin(), share_it->second.end(),
                       [&data](shareInfo &info) {
                         return info.user_name == data.user_key;
                       }) == share_it->second.end()) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    const auto it = mocked_data[query_user_key].find(data.tasklist_key);
    if (it == mocked_data[query_user_key].cend()) {
      return returnCode::ERR_NO_NODE;
    }
    out = it->second;
    return returnCode::SUCCESS;
  };

  returnCode GetAllTasklist(const RequestData &data,
                            std::vector<std::string> &outNames) override {
    const auto it = mocked_data.find(data.user_key);
    if (it == mocked_data.cend()) {
      return returnCode::ERR_NO_NODE;
    }
    std::transform(it->second.cbegin(), it->second.cend(),
                   std::back_inserter(outNames),
                   [](auto &&x) { return x.first; });
    return returnCode::SUCCESS;
  }

  returnCode GetAllAccessTaskList(const RequestData &data,
                                  std::vector<shareInfo> &out_list) override {
    if (data.RequestUserIsEmpty()) {
      return returnCode::ERR_KEY;
    }
    for (const auto &ut : mocked_share) {
      const auto &user = ut.first;
      const auto &tasklists = ut.second;
      for (const auto &ts : tasklists) {
        const auto &tasklist = ts.first;
        const auto &share_list = ts.second;
        auto it = std::find_if(share_list.begin(), share_list.end(),
                               [&data](const shareInfo &info) {
                                 return info.user_name == data.user_key;
                               });
        if (it != share_list.end()) {
          shareInfo info;
          info.task_list_name = it->task_list_name;
          info.user_name = user;
          info.permission = it->permission;
          out_list.push_back(std::move(info));
        }
      }
    }
    return returnCode::SUCCESS;
  }

  returnCode Create(const RequestData &data, TasklistContent &in,
                    std::string &outTasklistName) override {
    std::string tasklist_key = data.tasklist_key;
    const auto it = mocked_data[data.user_key].find(data.tasklist_key);
    if (it != mocked_data[data.user_key].cend()) {
      int tag = 1;
      while (mocked_data[data.user_key].find(data.tasklist_key + '(' +
                                             std::to_string(tag) + ')') !=
             mocked_data[data.user_key].cend()) {
        ++tag;
      }
      tasklist_key = data.tasklist_key + '(' + std::to_string(tag) + ')';
      in.name = tasklist_key;
    }
    mocked_data[data.user_key][tasklist_key] = in;
    return returnCode::SUCCESS;
  }

  returnCode Revise(const RequestData &data, TasklistContent &in) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      auto share_it = mocked_share[data.other_user_key].find(data.tasklist_key);
      if (share_it == mocked_share[data.other_user_key].end()) {
        return returnCode::ERR_NO_NODE;
      }
      auto share_info_it = std::find_if(
          share_it->second.begin(), share_it->second.end(),
          [&data](shareInfo &info) { return info.user_name == data.user_key; });
      if (share_info_it == share_it->second.end()) {
        return returnCode::ERR_ACCESS;
      }
      if (share_info_it->permission == false) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    auto it = mocked_data[query_user_key].find(data.tasklist_key);
    if (it == mocked_data[query_user_key].end()) {
      return returnCode::ERR_NO_NODE;
    }
    if (!in.content.empty()) {
      it->second.content = in.content;
    }
    return returnCode::SUCCESS;
  }

  returnCode Delete(const RequestData &data) override {
    auto it = mocked_data[data.user_key].find(data.tasklist_key);
    if (it == mocked_data[data.user_key].end()) {
      return returnCode::ERR_NO_NODE;
    }
    mocked_data[data.user_key].erase(it);
    return returnCode::SUCCESS;
  }

  bool Exists(const RequestData &data) override {
    return mocked_data[data.user_key].find(data.tasklist_key) !=
           mocked_data[data.user_key].end();
  }

  returnCode GetAllGrantTaskList(const RequestData &data,
                                 std::vector<shareInfo> &out_list,
                                 bool &isPublic) override {
    if (data.RequestUserIsEmpty() || data.RequestTaskListIsEmpty()) {
      return returnCode::ERR_RFIELD;
    }
    auto user_it = mocked_share.find(data.user_key);
    if (user_it == mocked_share.end()) {
      return returnCode::ERR_NO_NODE;
    }
    auto tasklist_it = user_it->second.find(data.tasklist_key);
    if (tasklist_it == user_it->second.end()) {
      return returnCode::ERR_NO_NODE;
    }

    isPublic = false;

    if (mocked_data[data.user_key][data.tasklist_key].visibility == "public") {
      isPublic = true;
      return returnCode::SUCCESS;
    }

    if (mocked_data[data.user_key][data.tasklist_key].visibility == "private") {
      return returnCode::ERR_ACCESS;
    }

    out_list = tasklist_it->second;
    return returnCode::SUCCESS;
  }

  returnCode ReviseGrantTaskList(const RequestData &data,
                                 std::vector<shareInfo> &in_list,
                                 std::string &errUser) override {

    if (data.RequestTaskListIsEmpty() || data.RequestUserIsEmpty()) {
      return ERR_KEY;
    }

    if (mocked_data[data.user_key].find(data.tasklist_key) ==
        mocked_data[data.user_key].end()) {
      return returnCode::ERR_NO_NODE;
    }

    auto &share_list = mocked_share[data.user_key][data.tasklist_key];
    for (const auto &share : in_list) {
      auto it = std::find_if(
          share_list.begin(), share_list.end(),
          [&share](shareInfo &x) { return x.user_name == share.user_name; });
      if (it == share_list.end()) {
        share_list.push_back(share);
      } else {
        (*it) = share;
      }
    }

    return returnCode::SUCCESS;
  }

  returnCode RemoveGrantTaskList(const RequestData &data,
                                 std::vector<std::string> &in_list,
                                 std::string &errUser) override {

    if (data.RequestTaskListIsEmpty() || data.RequestUserIsEmpty()) {
      return returnCode::ERR_KEY;
    }

    if (mocked_data[data.user_key].find(data.tasklist_key) ==
        mocked_data[data.user_key].end()) {
      return returnCode::ERR_NO_NODE;
    }

    auto &share_list = mocked_share[data.user_key][data.tasklist_key];
    for (const auto &user : in_list) {
      auto it =
          std::find_if(share_list.begin(), share_list.end(),
                       [&user](shareInfo &x) { return x.user_name == user; });
      if (it != share_list.end()) {
        share_list.erase(it);
      }
    }

    return returnCode::SUCCESS;
  }

  returnCode GetAllPublicTaskList(
      std::vector<std::pair<std::string, std::string>> &out_list) override {
    if (data.RequestUserIsEmpty()) {
      return returnCode::ERR_RFIELD;
    }
    for (const auto &[user_key, tasklist] : mocked_data) {
      if (tasklist.second.visibility == "public") {
        out_list.push_back({user_key, tasklist.first});
      }
    }
    return returnCode::SUCCESS;
  }
  void Clear() { mocked_data.clear(); }

private:
  /* (user_key, tasklist_key) -> TasklistContent */
  std::map<std::string, std::map<std::string, TasklistContent>> mocked_data;
  /* (user_key, tasklsit_key) -> shareInfo */
  std::map<std::string, std::map<std::string, std::vector<shareInfo>>>
      mocked_share;
};

class MockedTasksWorker : public TasksWorker {
public:
  MockedTasksWorker(std::shared_ptr<DB> _db,
                    std::shared_ptr<TaskListsWorker> _taskListWorker)
      : TasksWorker(_db, _taskListWorker) {}

  ~MockedTasksWorker() {}

  returnCode Query(const RequestData &data, TaskContent &out) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      if (!CheckWritePerm(data.user_key, data.other_user_key,
                          data.tasklist_key)) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    const auto &mocked_tasks = mocked_data[query_user_key][data.tasklist_key];
    const auto it = mocked_tasks.find(data.task_key);
    if (it == mocked_tasks.cend()) {
      return returnCode::ERR_NO_NODE;
    }
    out = it->second;
    return returnCode::SUCCESS;
  };

  returnCode GetAllTasksName(const RequestData &data,
                             std::vector<std::string> &outNames) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      if (!CheckWritePerm(data.user_key, data.other_user_key,
                          data.tasklist_key)) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    const auto it = mocked_data[query_user_key].find(data.tasklist_key);
    if (it == mocked_data[query_user_key].cend()) {
      return returnCode::ERR_NO_NODE;
    }
    std::transform(it->second.cbegin(), it->second.cend(),
                   std::back_inserter(outNames),
                   [](auto &&x) { return x.first; });
    return returnCode::SUCCESS;
  }

  returnCode Create(const RequestData &data, TaskContent &in,
                    std::string &outTasklistName) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      if (!CheckWritePerm(data.user_key, data.other_user_key,
                          data.tasklist_key)) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    auto &mocked_tasks = mocked_data[query_user_key][data.tasklist_key];
    std::string task_key = data.task_key;
    const auto it = mocked_tasks.find(data.tasklist_key);
    if (it != mocked_tasks.cend()) {
      int tag = 1;
      while (mocked_tasks.find(data.task_key + '(' + std::to_string(tag) +
                               ')') != mocked_tasks.cend()) {
        ++tag;
      }
      task_key = data.task_key + '(' + std::to_string(tag) + ')';
    }
    in.name = task_key;
    mocked_tasks[task_key] = in;
    return returnCode::SUCCESS;
  }

  returnCode Revise(const RequestData &data, TaskContent &in) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      if (!CheckWritePerm(data.user_key, data.other_user_key,
                          data.tasklist_key)) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    auto &mocked_tasks = mocked_data[query_user_key][data.tasklist_key];
    auto it = mocked_tasks.find(data.task_key);
    if (it == mocked_tasks.end()) {
      return returnCode::ERR_NO_NODE;
    }
    if (!in.content.empty()) {
      it->second.content = in.content;
    }
    if (!in.date.empty()) {
      it->second.date = in.date;
    }
    if (!in.startDate.empty()) {
      it->second.startDate = in.startDate;
    }
    return returnCode::SUCCESS;
  }

  returnCode Delete(const RequestData &data) override {
    std::string query_user_key = data.user_key;
    if (!data.other_user_key.empty()) {
      if (!CheckWritePerm(data.user_key, data.other_user_key,
                          data.tasklist_key)) {
        return returnCode::ERR_ACCESS;
      }
      query_user_key = data.other_user_key;
    }

    auto &mocked_tasks = mocked_data[query_user_key][data.tasklist_key];
    auto it = mocked_tasks.find(data.task_key);
    if (it == mocked_tasks.end()) {
      return returnCode::ERR_NO_NODE;
    }
    mocked_tasks.erase(it);
    return returnCode::SUCCESS;
  }

  bool CheckWritePerm(const std::string &user, const std::string &other_user,
                      const std::string &tasklist) {
    std::shared_ptr<MockedTasklistsWorker> mocked_tasklists_worker =
        std::dynamic_pointer_cast<MockedTasklistsWorker>(taskListsWorker);
    if (!mocked_tasklists_worker) {
      return false;
    }
    const auto &shared_list = mocked_tasklists_worker->mocked_share;
    auto tasklists_it = shared_list.find(other_user);
    if (tasklists_it == shared_list.end()) {
      return false;
    }
    auto sharelist_it = tasklists_it->second.find(tasklist);
    if (sharelist_it == tasklists_it->second.end()) {
      return false;
    }
    auto shareinfo_it =
        std::find_if(sharelist_it->second.begin(), sharelist_it->second.end(),
                     [&user](const shareInfo &info) {
                       return info.user_name == user && info.permission == true;
                     });
    return shareinfo_it != sharelist_it->second.end();
  }

  void Clear() { mocked_data.clear(); }

private:
  /* (user_key, tasklist_key, task_key) -> TasklistContent */
  std::map<std::string,
           std::map<std::string, std::map<std::string, TaskContent>>>
      mocked_data;
};

class APITest : public ::testing::Test {
protected:
  void SetUp() override {

    mocked_users = std::make_shared<MockedUsers>();
    mocked_db = std::make_shared<DB>();
    mocked_tasklists_worker =
        std::make_shared<MockedTasklistsWorker>(mocked_db, mocked_users);
    mocked_tasks_worker =
        std::make_shared<MockedTasksWorker>(mocked_db, mocked_tasklists_worker);

    api = std::make_shared<Api>(mocked_users, mocked_tasklists_worker,
                                mocked_tasks_worker, mocked_db);
    running_server = std::make_shared<std::thread>(
        [&]() { this->api->Run(this->test_host, this->test_port); });

    /* wait for some time for the service to be set up */
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  void TearDown() override {
    api->Stop();
    running_server->join();
  }

protected:
  std::shared_ptr<Api> api;
  std::shared_ptr<MockedUsers> mocked_users;
  std::shared_ptr<MockedTasklistsWorker> mocked_tasklists_worker;
  std::shared_ptr<MockedTasksWorker> mocked_tasks_worker;
  std::shared_ptr<DB> mocked_db; /* Not mocked for now */
  const std::string test_host = "0.0.0.0";
  const uint32_t test_port = 3301;
  std::shared_ptr<std::thread> running_server;
};

TEST_F(APITest, UsersRegister) {
  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    mocked_users->SetCreateResult(true);
    mocked_users->SetDuplicatedemaiResult(false);
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    mocked_users->SetCreateResult(false);
    mocked_users->SetDuplicatedemaiResult(true);
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("bob@columbia.edu", "123456");
    mocked_users->SetCreateResult(true);
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Bob";
    mocked_users->SetCreateResult(true);
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("", "123456");
    req_body["name"] = "Bob";
    mocked_users->SetCreateResult(true);
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }
}

TEST_F(APITest, UserLogin) {
  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(false);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_EQ(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("token"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }
}

TEST_F(APITest, UsersLogout) {
  std::string token;
  mocked_tasklists_worker->Clear();

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1";
    request_body["content"] = "some_content_1";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Post("/v1/users/logout");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth("", "");
    auto result = client.Post("/v1/users/logout");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed token invalid"), std::string::npos);
  }

  /* Sleep for 1 second so a new token will be generated */
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  mocked_tasklists_worker->Clear();
}

TEST_F(APITest, TaskLists) {
  std::string token;
  mocked_tasklists_worker->Clear();

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1";
    request_body["content"] = "some_content_1";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_2";
    request_body["content"] = "some_content_2";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1_new";
    request_body["content"] = "some_content_1_new";
    auto result = client.Post("/v1/task_lists/tasklists_test_name_1",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_1_new";
    auto result = client.Post("/v1/task_lists/tasklists_test_name_1",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1_new"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Delete("/v1/task_lists/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
  }

  mocked_tasklists_worker->Clear();
}

TEST_F(APITest, TaskListsCreate) {
  std::string token;
  mocked_tasklists_worker->Clear();

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  mocked_tasklists_worker->Clear();
}

TEST_F(APITest, Tasks) {
  std::string token;
  mocked_tasklists_worker->Clear();
  mocked_tasks_worker->Clear();

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("Alice", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1";
    request_body["content"] = "some_content_1";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasks_test_name_1";
    request_body["content"] = "some_content_1";
    request_body["date"] = "some_date_1";
    auto result =
        client.Post("/v1/task_lists/tasklists_test_name_1/tasks/create",
                    request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasks_test_name_2";
    request_body["content"] = "some_content_2";
    request_body["date"] = "some_date_2";
    auto result =
        client.Post("/v1/task_lists/tasklists_test_name_1/tasks/create",
                    request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1/tasks");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_date_1"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("some_date_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_2");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_2"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_2"), std::string::npos);
    EXPECT_NE(result->body.find("some_date_2"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_1"), std::string::npos);
    EXPECT_EQ(result->body.find("some_date_1"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_1_new";
    request_body["date"] = "some_date_1_new";
    auto result = client.Post(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1",
        request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1_new"), std::string::npos);
    EXPECT_NE(result->body.find("some_date_1_new"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("some_date_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_1_another";
    auto result = client.Post(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1",
        request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1_another"), std::string::npos);
    EXPECT_NE(result->body.find("some_date_1_new"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("some_date_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Delete(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1/tasks");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_2"), std::string::npos);
  }

  mocked_tasklists_worker->Clear();
  mocked_tasks_worker->Clear();
}

TEST_F(APITest, Share) {
  std::string token;
  std::string token_test_user_1;
  std::string token_test_user_2;
  std::string token_test_user_3;
  mocked_tasklists_worker->Clear();
  mocked_tasks_worker->Clear();

  /* Create user tokens and task lists */

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("Alice@columbia.edu", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("test_user_1", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token_test_user_1 = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("test_user_2", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token_test_user_2 = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("test_user_3", "123456");
    mocked_users->SetValidateResult(true);
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);

    try {
      token_test_user_3 = nlohmann::json::parse(result->body).at("token");
    } catch (std::exception &e) {
      EXPECT_TRUE(false);
    }
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1";
    request_body["content"] = "some_content_1";
    request_body["visibility"] = "shared";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_2";
    request_body["content"] = "some_content_2";
    request_body["visibility"] = "public";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_3";
    request_body["content"] = "some_content_3";
    request_body["visibility"] = "private";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  /* Creation test */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_permission"] = {
        {{"user", "test_user_1"}, {"permission", true}},
        {{"user", "test_user_2"}, {"permission", false}}};
    auto result = client.Post("/v1/share/tasklists_test_name_1/create",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_permission"] = {
        {{"user", "test_user_1"}, {"permission", true}},
        {{"user", "test_user_2"}, {"permission", false}}};
    auto result = client.Post("/v1/share/tasklists_test_name_2/create",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_permission"] = {
        {{"user", "test_user_1"}, {"permission", true}},
        {{"user", "test_user_2"}, {"permission", false}}};
    auto result = client.Post("/v1/share/tasklists_test_name_3/create",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  /* Getting test */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_1"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_2");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("public"), std::string::npos);
  }

  /* Deletion test */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_list"] = {"test_user_1"};
    auto result = client.Delete("/v1/share/tasklists_test_name_1",
                                request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("test_user_1"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_permission"] = {
        {{"user", "test_user_1"}, {"permission", true}}};
    auto result = client.Post("/v1/share/tasklists_test_name_1/create",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_1"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_list"] = {"test_user_1", "test_user_2"};
    auto result = client.Delete("/v1/share/tasklists_test_name_1",
                                request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("test_user_1"), std::string::npos);
    EXPECT_EQ(result->body.find("test_user_2"), std::string::npos);
  }

  /* Add back */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["user_permission"] = {
        {{"user", "test_user_1"}, {"permission", true}},
        {{"user", "test_user_2"}, {"permission", false}}};
    auto result = client.Post("/v1/share/tasklists_test_name_1/create",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_1"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_2"), std::string::npos);
  }

  /* Task lists related */

  /* Get task lists */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_2, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_3, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  /* Update task lists */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_1_new";
    auto result = client.Post(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu",
        request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1_new"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_3, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_3_new";
    auto result = client.Post(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu",
        request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    nlohmann::json request_body;
    request_body["visibility"] = "public"; /* Should be ignored */
    auto result = client.Post(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu",
        request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("shared"), std::string::npos);
    EXPECT_EQ(result->body.find("public"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get("/v1/task_lists?share=true");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_3"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get("/public/all");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
    EXPECT_NE(result->body.find("Alice@columbia.edu"), std::string::npos);
    EXPECT_EQ(result->body.find("tasklists_test_name_3"), std::string::npos);
  }

  /* Tasks related */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    nlohmann::json request_body;
    request_body["name"] = "tasks_test_name_1";
    request_body["content"] = "some_content_1";
    request_body["date"] = "some_date_1";
    auto result = client.Post("/v1/task_lists/tasklists_test_name_1/tasks/"
                              "create?other=Alice@columbia.edu",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    nlohmann::json request_body;
    request_body["name"] = "tasks_test_name_2";
    request_body["content"] = "some_content_2";
    request_body["date"] = "some_date_2";
    auto result = client.Post("/v1/task_lists/tasklists_test_name_1/tasks/"
                              "create?other=Alice@columbia.edu",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_2, "");
    nlohmann::json request_body;
    request_body["name"] = "tasks_test_name_3";
    request_body["content"] = "some_content_3";
    request_body["date"] = "some_date_3";
    auto result = client.Post("/v1/task_lists/tasklists_test_name_1/tasks/"
                              "create?other=Alice@columbia.edu",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1/tasks/"
                             "tasks_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1/tasks");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_1_new";
    request_body["date"] = "some_date_1_new";
    auto result = client.Post("/v1/task_lists/tasklists_test_name_1/tasks/"
                              "tasks_test_name_1?other=Alice@columbia.edu",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get(
        "/v1/task_lists/tasklists_test_name_1/tasks/tasks_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("some_content_1_new"), std::string::npos);
    EXPECT_NE(result->body.find("some_date_1_new"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Delete("/v1/task_lists/tasklists_test_name_1/tasks/"
                                "tasks_test_name_1?other=Alice@columbia.edu");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1/tasks");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("tasks_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasks_test_name_2"), std::string::npos);
  }

  mocked_tasklists_worker->Clear();
  mocked_tasks_worker->Clear();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
