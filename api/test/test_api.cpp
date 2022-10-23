#include <api/api.h>
#include <users/users.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

class MockedUsers: public Users {
public:
    ~MockedUsers() {}

    bool Create(const UserInfo&) override {
        return create_mock_return;
    }

    bool Validate(const UserInfo&) override {
        return validate_mock_return;
    }

    void SetCreateResult(bool value) {
        create_mock_return = value;
    }

    void SetValidateResult(bool value) {
        validate_mock_return = value;
    }

private:
    bool create_mock_return;
    bool validate_mock_return;
};

class APITest: public ::testing::Test {
protected:

    void SetUp() override {
        mocked_users = std::make_shared<MockedUsers>();
        api = std::make_shared<API>(mocked_users);
        running_server = std::make_shared<std::thread>([&](){this->api->Run(this->test_host, this->test_port);});
        // wait for some time for the service to be set up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override {
        api->Stop();
        running_server->join();
    }

protected:
    std::shared_ptr<API> api;
    std::shared_ptr<MockedUsers> mocked_users;
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
        auto result = client.Post("/v1/users/register", req_body.dump(), "text/plain");
        EXPECT_EQ(result.error(), httplib::Error::Success);
        EXPECT_NE(result->body.find("success"), std::string::npos);
    }

    {
        httplib::Client client(test_host, test_port);
        nlohmann::json req_body;
        req_body["name"] = "Alice";
        client.set_basic_auth("alice@columbia.edu", "123456");
        mocked_users->SetCreateResult(false);
        auto result = client.Post("/v1/users/register", req_body.dump(), "text/plain");
        EXPECT_EQ(result.error(), httplib::Error::Success);
        EXPECT_NE(result->body.find("failed"), std::string::npos);
    }

    {
        httplib::Client client(test_host, test_port);
        nlohmann::json req_body;
        client.set_basic_auth("bob@columbia.edu", "123456");
        mocked_users->SetCreateResult(true);
        auto result = client.Post("/v1/users/register", req_body.dump(), "text/plain");
        EXPECT_EQ(result.error(), httplib::Error::Success);
        EXPECT_NE(result->body.find("failed"), std::string::npos);
    }

    {
        httplib::Client client(test_host, test_port);
        nlohmann::json req_body;
        req_body["name"] = "Bob";
        mocked_users->SetCreateResult(true);
        auto result = client.Post("/v1/users/register", req_body.dump(), "text/plain");
        EXPECT_EQ(result.error(), httplib::Error::Success);
        EXPECT_NE(result->body.find("failed"), std::string::npos);
    }

    {
        httplib::Client client(test_host, test_port);
        nlohmann::json req_body;
        client.set_basic_auth("", "123456");
        req_body["name"] = "Bob";
        mocked_users->SetCreateResult(true);
        auto result = client.Post("/v1/users/register", req_body.dump(), "text/plain");
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
        } catch (std::exception& e) {
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
        } catch (std::exception& e) {
            EXPECT_TRUE(false);
        }
    }

    {
        httplib::Client client(test_host, test_port);
        client.set_basic_auth(token, "");
        auto result = client.Get("/v1/tasklists/test_tasklists_name_1");
        EXPECT_EQ(result.error(), httplib::Error::Success);
        EXPECT_NE(result->body.find("success"), std::string::npos);
    }
}

TEST_F(APITest, TaskListsCreate) {
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
        } catch (std::exception& e) {
            EXPECT_TRUE(false);
        }
    }

    {
        httplib::Client client(test_host, test_port);
        client.set_basic_auth(token, "");
        nlohmann::json request_body;
        request_body["tasklists_name"] = "tasklists_test_name_1";
        auto result = client.Post("/v1/tasklists/create", request_body.dump(), "text/plain");
        EXPECT_EQ(result.error(), httplib::Error::Success);
        EXPECT_NE(result->body.find("success"), std::string::npos);
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}