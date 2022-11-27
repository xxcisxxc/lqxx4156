// #include <db/DB.h>
// #include <exception>
// #include <gmock/gmock.h>
// #include <gtest/gtest.h>
// #include <memory>
// #include <tasklists/tasklistsWorker.h>
// #include <tasks/tasksWorker.h>
// #include <users/users.h>
// using namespace ::testing;

// class IntgTest : public ::testing::Test {
// protected:
//   void SetUp() override {
//     db = std::make_shared<DB>();
//     taskListsWorker = std::make_shared<TaskListsWorker>(*db);
//     tasksWorker = std::make_shared<TasksWorker>(db, taskListsWorker);
//     users = std::make_shared<Users>(db);
//   }

//   void TearDown() override {
//     delete (db);
//     delete (taskListsWorker);
//     delete (tasksWorker);
//     delete (users);
//   }

//   std::shared_ptr<DB> *db;
//   std::shared_ptr<TaskListsWorker> *taskListsWorker;
//   std::shared_ptr<TasksWorker> tasksWorker;
//   std::shared_ptr<Users> users;
// };

// /*
//  * @brief Integration test for users
//  */

// TEST_F(IntgTest, Users_Create) {
//   // should be able to create a user
//   EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu",
//   "123456"))); EXPECT_TRUE(users->Create(UserInfo("bob", "bob@columbia.edu",
//   "123456"))); EXPECT_TRUE(
//       users->Create(UserInfo("alice2", "alice2@columbia.edu", "123456")));

//   // duplicate email failed
//   EXPECT_FALSE(
//       users->Create(UserInfo("alice3", "alice@columbia.edu", "123456")));

//   // email format failed
//   EXPECT_FALSE(
//       users->Create(UserInfo("bob", "bob@", "123456")));

//   // delete user

// };

// TEST_F(IntgTest, Users_Delete) {
//   // should be able to create a user
//   EXPECT_TRUE(users->Delete(UserInfo("alice", "alice@columbia.edu",
//   "123456")));

//   // duplicate email failed
//   EXPECT_FALSE(
//       users->Delete(UserInfo("alice3", "alice@columbia.edu", "123456")));

//   // no such user
//   EXPECT_FALSE(
//       users->Delete(UserInfo("bob", "bob@", "123456")));

//   // delete user

// };

// TEST_F(IntgTest, Users_Validate) {
//   // no such user
//   EXPECT_FALSE(users->Validate(UserInfo("", "alice@columbia.edu",
//   "123456")));

//   // should be able to validate a user
//   EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu",
//   "123456"))); EXPECT_TRUE(users->Validate(UserInfo("", "alice@columbia.edu",
//   "123456"))); EXPECT_TRUE(
//       users->Validate(UserInfo("alice", "alice@columbia.edu", "123456")));

//   // wrong name
//   EXPECT_FALSE(
//       users->Validate(UserInfo("not alice", "alice@columbia.edu",
//       "123456")));
//   // wrong password
//   EXPECT_FALSE(users->Validate(
//       UserInfo("alice", "alice@columbia.edu", "wrong password")));

//   // missing password
//   EXPECT_FALSE(users->Validate(UserInfo("", "alice@columbia.edu", "")));
//   EXPECT_FALSE(users->Validate(UserInfo("alice", "alice@columbia.edu", "")));

//   // delete user

// };

// TEST_F(IntgTest, Users_DuplicatedEmail) {
//   // should be successful
//   EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu",
//   "123456")));
//   EXPECT_TRUE(users->DuplicatedEmail(UserInfo("alice@columbia.edu")));

//   // no such user
//   EXPECT_FALSE(users->DuplicatedEmail(UserInfo("bob@columbia.edu")));

//   // delete user

// };

// /*
//  * @brief Integration test for TaskLists
//  */

// TEST_F(IntgTest, TaskLists_QueryOwned) {
//   // setup environment
//   RequestData data;
//   TasklistContent out;

// };

// TEST_F(IntgTest, TaskLists_QueryOthers) {

// };

// TEST_F(IntgTest, TaskLists_Create) {

// };

// TEST_F(IntgTest, TaskLists_Delete) {

// };

// TEST_F(IntgTest, TaskLists_Revise) {

// };

// TEST_F(IntgTest, TaskLists_GetAllTasklist) {

// };

// TEST_F(IntgTest, TaskLists_GetAllAccessTaskList) {

// };

// TEST_F(IntgTest, TaskLists_GetAllGrantTaskList) {

// };

// TEST_F(IntgTest, TaskLists_ReviseGrantTaskList) {

// };

// TEST_F(IntgTest, TaskLists_RemoveGrantTaskList) {

// };

// TEST_F(IntgTest, TaskLists_GetAllPublicTaskList) {

// };

// TEST_F(IntgTest, TaskLists_Exists) {

// };

// /*
//  * @brief Integration test for Tasks
//  */

// TEST_F(IntgTest, Tasks_Query) {

// };

// TEST_F(IntgTest, Tasks_Create) {

// };

// TEST_F(IntgTest, Tasks_Delete) {

// };

// TEST_F(IntgTest, Tasks_Revise) {

// };

// TEST_F(IntgTest, Tasks_GetAllTasksName) {

// };

// int main(int argc, char **argv) {
//   testing::InitGoogleTest(&argc, argv);

//   return RUN_ALL_TESTS();
// }
