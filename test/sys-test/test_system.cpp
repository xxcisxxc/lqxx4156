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

class APITest : public ::testing::Test {
protected:
  void SetUp() override {
    const std::string db_host = "neo4j://neo4j:hello4156@localhost:7687";
    db = std::make_shared<DB>(db_host);
    db->deleteEverything();

    api = std::make_shared<Api>(nullptr, nullptr, nullptr, db, nullptr);
    running_server = std::make_shared<std::thread>(
        [&]() { this->api->Run(this->test_host, this->test_port); });

    /* wait for some time for the service to be set up */
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  void TearDown() override {
    api->Stop();
    running_server->join();
    db->deleteEverything();
  }

protected:
  std::shared_ptr<Api> api;
  std::shared_ptr<DB> db; /* Not mocked for now */
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
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }
}

TEST_F(APITest, UserLogin) {
  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("alice@columbia.edu", "123456");
    auto result = client.Post("/v1/users/login", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("token"), std::string::npos);
  }
}

TEST_F(APITest, UsersLogout) {
  std::string token;

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth("alice@columbia.edu", "123456");
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
    request_body["visibility"] = "shared";
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
    client.set_basic_auth("alice@columbia.edu", "123456");
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

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }
}

TEST_F(APITest, TaskLists) {
  std::string token;
  db->deleteEverything();

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("alice@columbia.edu", "123456");
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
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["name"] = "tasklists_test_name_2";
    request_body["content"] = "some_content_2";
    request_body["visibility"] = "shared";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Delete("/v1/task_lists/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("failed"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    auto result = client.Get("/v1/task_lists");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  db->deleteEverything();
}

TEST_F(APITest, TaskListsCreate) {
  std::string token;

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth("alice@columbia.edu", "123456");
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
    request_body["visibility"] = "shared";
    auto result =
        client.Post("/v1/task_lists/create", request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }
}

TEST_F(APITest, Tasks) {
  std::string token;

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("alice@columbia.edu", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("alice@columbia.edu", "123456");
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
    request_body["name"] = "tasks_test_name_1";
    request_body["content"] = "some_content_1";
    request_body["date"] = "01/01/2022";
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
    request_body["date"] = "01/02/2022";
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
    EXPECT_NE(result->body.find("01/01/2022"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("01/02/2022"), std::string::npos);
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
    EXPECT_NE(result->body.find("01/02/2022"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_1"), std::string::npos);
    EXPECT_EQ(result->body.find("01/01/2022"), std::string::npos);
  }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    request_body["content"] = "some_content_1_new";
    request_body["date"] = "01/03/2022";
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
    EXPECT_NE(result->body.find("01/03/2022"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("01/02/2022"), std::string::npos);
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
    EXPECT_NE(result->body.find("01/03/2022"), std::string::npos);
    EXPECT_EQ(result->body.find("some_content_2"), std::string::npos);
    EXPECT_EQ(result->body.find("01/02/2022"), std::string::npos);
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
}

TEST_F(APITest, Share) {
  std::string token;
  std::string token_test_user_1;
  std::string token_test_user_2;
  std::string token_test_user_3;

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "Alice";
    client.set_basic_auth("Alice@columbia.edu", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "test_user_1";
    client.set_basic_auth("test_user_1@test.com", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "test_user_2";
    client.set_basic_auth("test_user_2@test.com", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    req_body["name"] = "test_user_3";
    client.set_basic_auth("test_user_3@test.com", "123456");
    auto result =
        client.Post("/v1/users/register", req_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  /* Create user tokens and task lists */

  {
    httplib::Client client(test_host, test_port);
    nlohmann::json req_body;
    client.set_basic_auth("Alice@columbia.edu", "123456");
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
    client.set_basic_auth("test_user_1@test.com", "123456");
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
    client.set_basic_auth("test_user_2@test.com", "123456");
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
    client.set_basic_auth("test_user_3@test.com", "123456");
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
        {{"user", "test_user_1@test.com"}, {"permission", true}},
        {{"user", "test_user_2@test.com"}, {"permission", false}}};
    auto result = client.Post("/v1/share/tasklists_test_name_1/create",
                              request_body.dump(), "text/plain");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
  }

  // {
  //   httplib::Client client(test_host, test_port);
  //   client.set_basic_auth(token, "");
  //   nlohmann::json request_body;
  //   request_body["user_permission"] = {
  //       {{"user", "test_user_1@test.com"}, {"permission", true}},
  //       {{"user", "test_user_2@test.com"}, {"permission", false}}};
  //   auto result = client.Post("/v1/share/tasklists_test_name_2/create",
  //                             request_body.dump(), "text/plain");
  //   EXPECT_EQ(result.error(), httplib::Error::Success);
  //   EXPECT_NE(result->body.find("success"), std::string::npos);
  // }

  // {
  //   httplib::Client client(test_host, test_port);
  //   client.set_basic_auth(token, "");
  //   nlohmann::json request_body;
  //   request_body["user_permission"] = {
  //       {{"user", "test_user_1@test.com"}, {"permission", true}},
  //       {{"user", "test_user_2@test.com"}, {"permission", false}}};
  //   auto result = client.Post("/v1/share/tasklists_test_name_3/create",
  //                             request_body.dump(), "text/plain");
  //   EXPECT_EQ(result.error(), httplib::Error::Success);
  //   EXPECT_NE(result->body.find("failed"), std::string::npos);
  // }

  /* Getting test */

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token, "");
    nlohmann::json request_body;
    auto result = client.Get("/v1/share/tasklists_test_name_1");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_1@test.com"), std::string::npos);
    EXPECT_NE(result->body.find("test_user_2@test.com"), std::string::npos);
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
    EXPECT_NE(result->body.find("failed"), std::string::npos);
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

  // {
  //   httplib::Client client(test_host, test_port);
  //   client.set_basic_auth(token_test_user_1, "");
  //   auto result = client.Get("/v1/task_lists?share=true");
  //   EXPECT_EQ(result.error(), httplib::Error::Success);
  //   EXPECT_NE(result->body.find("success"), std::string::npos);
  //   EXPECT_NE(result->body.find("tasklists_test_name_1"), std::string::npos);
  //   EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
  //   EXPECT_EQ(result->body.find("tasklists_test_name_3"), std::string::npos);
  // }

  {
    httplib::Client client(test_host, test_port);
    client.set_basic_auth(token_test_user_1, "");
    auto result = client.Get("/v1/public/all");
    EXPECT_EQ(result.error(), httplib::Error::Success);
    EXPECT_NE(result->body.find("success"), std::string::npos);
    EXPECT_EQ(result->body.find("tasklists_test_name_1"), std::string::npos);
    EXPECT_NE(result->body.find("tasklists_test_name_2"), std::string::npos);
    EXPECT_NE(result->body.find("Alice@columbia.edu"), std::string::npos);
    EXPECT_EQ(result->body.find("tasklists_test_name_3"), std::string::npos);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
