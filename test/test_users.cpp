#include <db/DB.h>
#include <gtest/gtest.h>
#include <memory>
#include <users/users.h>

class MockedDB : public DB {
public:
  MockedDB() : DB() {}

  returnCode
  createUserNode(const std::map<std::string, std::string> &user_info) override {
    const auto pk_it = user_info.find("email");
    if (pk_it == user_info.cend()) {
      return returnCode::ERR_KEY;
    }
    if (mocked_data.find(pk_it->second) != mocked_data.end()) {
      return returnCode::ERR_DUP_NODE;
    }

    mocked_data[pk_it->second] = user_info;
    return returnCode::SUCCESS;
  }

  returnCode
  reviseUserNode(const std::string &user_pkey,
                 const std::map<std::string, std::string> &user_info) override {
    return returnCode::SUCCESS;
  }

  returnCode
  getUserNode(const std::string &user_pkey,
              std::map<std::string, std::string> &user_info) override {
    if (mocked_data.find(user_pkey) == mocked_data.end()) {
      return returnCode::ERR_NO_NODE;
    }
    user_info = mocked_data[user_pkey];
    return returnCode::SUCCESS;
  }

  void Clear() { mocked_data.clear(); }

private:
  std::map<std::string, std::map<std::string, std::string>> mocked_data;
};

class UsersTest : public ::testing::Test {
protected:
  void SetUp() override {
    mocked_db = std::make_shared<MockedDB>();
    users = std::make_shared<Users>(mocked_db);
  }

  void TearDown() override {}

protected:
  std::shared_ptr<Users> users;
  std::shared_ptr<MockedDB> mocked_db;
};

TEST_F(UsersTest, Create) {
  mocked_db->Clear();
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->Create(UserInfo("bob", "bob@columbia.edu", "123456")));
  EXPECT_TRUE(
      users->Create(UserInfo("alice2", "alice2@columbia.edu", "123456")));
  EXPECT_FALSE(
      users->Create(UserInfo("alice3", "alice@columbia.edu", "123456")));
}

TEST_F(UsersTest, Validate) {
  mocked_db->Clear();
  EXPECT_FALSE(users->Validate(UserInfo("", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->Validate(UserInfo("", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(
      users->Validate(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_FALSE(
      users->Validate(UserInfo("not alice", "alice@columbia.edu", "123456")));
  EXPECT_FALSE(users->Validate(
      UserInfo("alice", "alice@columbia.edu", "wrong password")));
  EXPECT_FALSE(users->Validate(UserInfo("", "alice@columbia.edu", "")));
  EXPECT_FALSE(users->Validate(UserInfo("alice", "alice@columbia.edu", "")));
}

TEST_F(UsersTest, DuplicatedEmail) {
  mocked_db->Clear();
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->DuplicatedEmail(UserInfo("alice@columbia.edu")));
  EXPECT_FALSE(users->DuplicatedEmail(UserInfo("bob@columbia.edu")));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
