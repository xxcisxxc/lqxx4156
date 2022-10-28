#include <algorithm>
#include <api/api.h>
#include <db/DB.h>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <tasklists/tasklistsWorker.h>
#include <users/users.h>

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

class MockedTasklistsWorker : public TaskListsWorker {
public:
  MockedTasklistsWorker(DB &_db) : TaskListsWorker(_db) {}

  ~MockedTasklistsWorker() {}

  returnCode Query(const RequestData &data, TasklistContent &out) override {
    const auto it = mocked_data[data.user_key].find(data.tasklist_key);
    if (it == mocked_data[data.user_key].cend()) {
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
    auto it = mocked_data[data.user_key].find(data.tasklist_key);
    if (it == mocked_data[data.user_key].end()) {
      return returnCode::ERR_NO_NODE;
    }
    if (!in.content.empty()) {
      it->second.content = in.content;
    }
    if (!in.date.empty()) {
      it->second.date = in.date;
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

  void Clear() { mocked_data.clear(); }

private:
  /* (user_key, tasklist_key) -> TasklistContent */
  std::map<std::string, std::map<std::string, TasklistContent>> mocked_data;
};

class MockedTasksWorker : public TasksWorker {
public:
  MockedTasksWorker(DB *_db, TaskListsWorker *_taskListWorker)
      : TasksWorker(_db, _taskListWorker) {}

  ~MockedTasksWorker() {}

  returnCode Query(const RequestData &data, TaskContent &out) override {
    const auto &mocked_tasks = mocked_data[data.user_key][data.tasklist_key];
    const auto it = mocked_tasks.find(data.task_key);
    if (it == mocked_tasks.cend()) {
      return returnCode::ERR_NO_NODE;
    }
    out = it->second;
    return returnCode::SUCCESS;
  };

  returnCode GetAllTasksName(const RequestData &data,
                             std::vector<std::string> &outNames) override {
    const auto it = mocked_data[data.user_key].find(data.tasklist_key);
    if (it == mocked_data[data.user_key].cend()) {
      return returnCode::ERR_NO_NODE;
    }
    std::transform(it->second.cbegin(), it->second.cend(),
                   std::back_inserter(outNames),
                   [](auto &&x) { return x.first; });
    return returnCode::SUCCESS;
  }

  returnCode Create(const RequestData &data, TaskContent &in,
                    std::string &outTasklistName) override {
    auto &mocked_tasks = mocked_data[data.user_key][data.tasklist_key];
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
    auto &mocked_tasks = mocked_data[data.user_key][data.tasklist_key];
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
    return returnCode::SUCCESS;
  }

  returnCode Delete(const RequestData &data) override {
    auto &mocked_tasks = mocked_data[data.user_key][data.tasklist_key];
    auto it = mocked_tasks.find(data.task_key);
    if (it == mocked_tasks.end()) {
      return returnCode::ERR_NO_NODE;
    }
    mocked_tasks.erase(it);
    return returnCode::SUCCESS;
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
        std::make_shared<MockedTasklistsWorker>(*mocked_db);
    mocked_tasks_worker = std::make_shared<MockedTasksWorker>(
        mocked_db.get(), (TaskListsWorker *)(mocked_tasks_worker.get()));

    api = std::make_shared<API>(mocked_users, mocked_tasklists_worker,
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
  std::shared_ptr<API> api;
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
    request_body["date"] = "some_date_1";
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
    request_body["date"] = "some_date_2";
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
    EXPECT_NE(result->body.find("some_date_1"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("some_date_2"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_1_new";
    request_body["content"] = "some_content_1_new";
    request_body["date"] = "some_date_2_new";
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
    request_body["date"] = "some_date_1_new";
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
    EXPECT_NE(result->body.find("some_date_1_new"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("some_date_2"), std::string::npos);
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
    request_body["date"] = "some_date_1";
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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
