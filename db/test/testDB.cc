#include "DB.h"
#include <algorithm>
#include <gtest/gtest.h>

const std::string host = "neo4j://neo4j:hello4156@localhost:7687";

TEST(TestDB, testconnection) {
  DB *db;

  // Create a new DB object
  // Must connect to the right host
  EXPECT_THROW(db = new DB(""), std::exception);
  EXPECT_THROW(db = new DB("neo4j://neo4j@localhost"), std::exception);
  EXPECT_THROW(db = new DB("neo4j+s://neo4j:hello4156@localhost:7687"),
               std::exception);
  EXPECT_NO_THROW(db = new DB(host));

  // Delete the DB object
  EXPECT_NO_THROW(delete db);
}

TEST(TestDB, testCreateUserNode) {
  DB db(host);
  std::map<std::string, std::string> user_info;

  // Create a user node
  // There must be a primary key (email) in the user_info
  EXPECT_EQ(db.createUserNode(user_info), ERR_KEY);
  user_info["email"] = "test0@test.com"; // TODO: there must be a unit test to
                                         // test the format of email
  // There must be a password in the user_info
  EXPECT_EQ(db.createUserNode(user_info), ERR_RFIELD);
  user_info["passwd"] = "test";
  EXPECT_EQ(db.createUserNode(user_info), SUCCESS);
  // Create a user node with the same primary key
  EXPECT_EQ(db.createUserNode(user_info), ERR_DUP_NODE);
  // Insert node with multiple fields
  user_info.clear();
  user_info["email"] = "test1@test.com";
  user_info["passwd"] = "test";
  user_info["field1"] = "value1";
  user_info["field2"] = "value2";
  user_info["field3"] = "value3";
  EXPECT_EQ(db.createUserNode(user_info), SUCCESS);
}

TEST(TestDB, testCreateTaskListNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::map<std::string, std::string> task_list_info;

  // Create a task list node
  // There must be a primary key (task list name) in the task_list_info
  EXPECT_EQ(db.createTaskListNode(user_pkey, task_list_info), ERR_KEY);
  task_list_info["name"] = "test0-task-list";
  // User node must exist
  EXPECT_EQ(db.createTaskListNode("wrong@test.com", task_list_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.createTaskListNode(user_pkey, task_list_info), SUCCESS);
  // Create a task list node with the same primary key
  EXPECT_EQ(db.createTaskListNode(user_pkey, task_list_info), ERR_DUP_NODE);
  // Insert node with multiple fields
  task_list_info.clear();
  task_list_info["name"] = "test1-task-list";
  task_list_info["field1"] = "value1";
  task_list_info["field2"] = "value2";
  task_list_info["field3"] = "value3";
  EXPECT_EQ(db.createTaskListNode(user_pkey, task_list_info), SUCCESS);
}

TEST(TestDB, testCreateTaskNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::string task_list_pkey = "test0-task-list";
  std::map<std::string, std::string> task_info;

  // Create a task node
  // There must be a primary key (task name) in the task_info
  EXPECT_EQ(db.createTaskNode(user_pkey, task_list_pkey, task_info), ERR_KEY);
  task_info["name"] = "test0-task";
  // User node must exist
  EXPECT_EQ(db.createTaskNode("wrong@test.com", task_list_pkey, task_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.createTaskNode(user_pkey, "wrong-task-list", task_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.createTaskNode(user_pkey, task_list_pkey, task_info), SUCCESS);
  // Create a task node with the same primary key
  EXPECT_EQ(db.createTaskNode(user_pkey, task_list_pkey, task_info),
            ERR_DUP_NODE);
  // Insert node with multiple fields
  task_info.clear();
  task_info["name"] = "test1-task";
  task_info["field1"] = "value1";
  task_info["field2"] = "value2";
  task_info["field3"] = "value3";
  EXPECT_EQ(db.createTaskNode(user_pkey, task_list_pkey, task_info), SUCCESS);
  EXPECT_EQ(db.createTaskNode(user_pkey, "test1-task-list", task_info),
            SUCCESS);
}

TEST(TestDB, testReviseUserNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::map<std::string, std::string> user_info;

  // Revise a user node
  // Primary Key cannot be revised
  user_info["email"] = "revised@test.com";
  EXPECT_EQ(db.reviseUserNode(user_pkey, user_info), ERR_KEY);
  user_info.clear();
  // There must be some fields to be revised
  EXPECT_EQ(db.reviseUserNode(user_pkey, user_info), ERR_RFIELD);
  user_info["newfield"] = "newvalue";
  // Node must exist
  EXPECT_EQ(db.reviseUserNode("wrong@test.com", user_info), ERR_NO_NODE);
  EXPECT_EQ(db.reviseUserNode(user_pkey, user_info), SUCCESS);
  // Revise multiple fields
  user_info.clear();
  user_info["field1"] = "revised1";
  user_info["field2"] = "revised2";
  user_info["field3"] = "revised3";
  EXPECT_EQ(db.reviseUserNode("test1@test.com", user_info), SUCCESS);
}

TEST(TestDB, testReviseTaskListNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::string tast_list_pkey = "test0-task-list";
  std::map<std::string, std::string> task_list_info;

  // Revise a task list node
  // Primary Key cannot be revised
  task_list_info["name"] = "revised-task-list";
  EXPECT_EQ(db.reviseTaskListNode(user_pkey, tast_list_pkey, task_list_info),
            ERR_KEY);
  task_list_info.clear();
  // There must be some fields to be revised
  EXPECT_EQ(db.reviseTaskListNode(user_pkey, tast_list_pkey, task_list_info),
            ERR_RFIELD);
  task_list_info["newfield"] = "newvalue";
  // Node must exist
  EXPECT_EQ(db.reviseTaskListNode(user_pkey, "wrong-task-list", task_list_info),
            ERR_NO_NODE);
  EXPECT_EQ(
      db.reviseTaskListNode("wrong@test.com", tast_list_pkey, task_list_info),
      ERR_NO_NODE);
  EXPECT_EQ(db.reviseTaskListNode(user_pkey, tast_list_pkey, task_list_info),
            SUCCESS);
  // Revise multiple fields
  task_list_info.clear();
  task_list_info["field1"] = "revised1";
  task_list_info["field2"] = "revised2";
  task_list_info["field3"] = "revised3";
  EXPECT_EQ(db.reviseTaskListNode(user_pkey, "test1-task-list", task_list_info),
            SUCCESS);
}

TEST(TestDB, testReviseTaskNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::string tast_list_pkey = "test0-task-list";
  std::string task_pkey = "test0-task";
  std::map<std::string, std::string> task_info;

  // Revise a task node
  // Primary Key cannot be revised
  task_info["name"] = "revised-task";
  EXPECT_EQ(db.reviseTaskNode(user_pkey, tast_list_pkey, task_pkey, task_info),
            ERR_KEY);
  task_info.clear();
  // There must be some fields to be revised
  EXPECT_EQ(db.reviseTaskNode(user_pkey, tast_list_pkey, task_pkey, task_info),
            ERR_RFIELD);
  task_info["newfield"] = "newvalue";
  // Node must exist
  EXPECT_EQ(
      db.reviseTaskNode(user_pkey, tast_list_pkey, "wrong-task", task_info),
      ERR_NO_NODE);
  EXPECT_EQ(
      db.reviseTaskNode(user_pkey, "wrong-task-list", task_pkey, task_info),
      ERR_NO_NODE);
  EXPECT_EQ(
      db.reviseTaskNode("wrong@test.com", tast_list_pkey, task_pkey, task_info),
      ERR_NO_NODE);
  EXPECT_EQ(db.reviseTaskNode(user_pkey, tast_list_pkey, task_pkey, task_info),
            SUCCESS);
  // Revise multiple fields
  task_info.clear();
  task_info["field1"] = "revised1";
  task_info["field2"] = "revised2";
  task_info["field3"] = "revised3";
  EXPECT_EQ(
      db.reviseTaskNode(user_pkey, tast_list_pkey, "test1-task", task_info),
      SUCCESS);
}

TEST(TestDB, testGetUserNode) {
  DB db(host);
  std::map<std::string, std::string> user_info;

  // Get the user node
  // Node must be in the DB
  user_info["newfield"] = "";
  EXPECT_EQ(db.getUserNode("wrong@test.com", user_info), ERR_NO_NODE);
  EXPECT_EQ(db.getUserNode("test0@test.com", user_info), SUCCESS);
  EXPECT_EQ(user_info.size(), 1);
  EXPECT_EQ(user_info["newfield"], "newvalue");
  // Get the user node with non-exist fields
  user_info.clear();
  user_info["nofield"];
  EXPECT_EQ(db.getUserNode("test0@test.com", user_info), SUCCESS);
  EXPECT_EQ(user_info.size(), 1);
  EXPECT_EQ(user_info["nofield"], "");
  // Get the user node with one specific field
  user_info.clear();
  user_info["field1"] = "";
  EXPECT_EQ(db.getUserNode("test1@test.com", user_info), SUCCESS);
  EXPECT_EQ(user_info.size(), 1);
  EXPECT_EQ(user_info["field1"], "revised1");
  // Get the user node with multiple specific fields
  user_info["field1"] = "";
  user_info["field2"] = "";
  EXPECT_EQ(db.getUserNode("test1@test.com", user_info), SUCCESS);
  EXPECT_EQ(user_info.size(), 2);
  EXPECT_EQ(user_info["field1"], "revised1");
  EXPECT_EQ(user_info["field2"], "revised2");
  // Get the user node with all fields
  user_info.clear();
  EXPECT_EQ(db.getUserNode("test1@test.com", user_info), SUCCESS);
  EXPECT_EQ(user_info.size(), 5);
  EXPECT_EQ(user_info["field1"], "revised1");
  EXPECT_EQ(user_info["field2"], "revised2");
  EXPECT_EQ(user_info["field3"], "revised3");
}

TEST(TestDB, testGetTaskListNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::map<std::string, std::string> task_list_info;

  // Get the task list node
  // Node must be in the DB
  task_list_info["newfield"] = "";
  EXPECT_EQ(db.getTaskListNode(user_pkey, "wrong-task-list", task_list_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.getTaskListNode(user_pkey, "test0-task-list", task_list_info),
            SUCCESS);
  EXPECT_EQ(task_list_info.size(), 1);
  EXPECT_EQ(task_list_info["newfield"], "newvalue");
  // Get the task list node with non-exist fields
  task_list_info.clear();
  task_list_info["nofield"];
  EXPECT_EQ(db.getTaskListNode(user_pkey, "test0-task-list", task_list_info),
            SUCCESS);
  EXPECT_EQ(task_list_info.size(), 1);
  EXPECT_EQ(task_list_info["nofield"], "");
  // Get the task list node with one specific field
  task_list_info.clear();
  task_list_info["field1"] = "";
  EXPECT_EQ(db.getTaskListNode(user_pkey, "test1-task-list", task_list_info),
            SUCCESS);
  EXPECT_EQ(task_list_info.size(), 1);
  EXPECT_EQ(task_list_info["field1"], "revised1");
  // Get the task list node with multiple specific fields
  task_list_info["field1"] = "";
  task_list_info["field2"] = "";
  EXPECT_EQ(db.getTaskListNode(user_pkey, "test1-task-list", task_list_info),
            SUCCESS);
  EXPECT_EQ(task_list_info.size(), 2);
  EXPECT_EQ(task_list_info["field1"], "revised1");
  EXPECT_EQ(task_list_info["field2"], "revised2");
  // Get the task list node with all fields
  task_list_info.clear();
  EXPECT_EQ(db.getTaskListNode(user_pkey, "test1-task-list", task_list_info),
            SUCCESS);
  EXPECT_EQ(task_list_info.size(), 4);
  EXPECT_EQ(task_list_info["field1"], "revised1");
  EXPECT_EQ(task_list_info["field2"], "revised2");
  EXPECT_EQ(task_list_info["field3"], "revised3");
}

TEST(TestDB, testGetTaskNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::string task_list_pkey = "test0-task-list";
  std::map<std::string, std::string> task_info;

  // Get the task node
  // Node must be in the DB
  task_info["newfield"] = "";
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "wrong-task", task_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "test0-task", task_info),
            SUCCESS);
  EXPECT_EQ(task_info.size(), 1);
  EXPECT_EQ(task_info["newfield"], "newvalue");
  // Get the task node with non-exist fields
  task_info.clear();
  task_info["nofield"];
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "test0-task", task_info),
            SUCCESS);
  EXPECT_EQ(task_info.size(), 1);
  EXPECT_EQ(task_info["nofield"], "");
  // Get the task node with one specific field
  task_info.clear();
  task_info["field1"] = "";
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "test1-task", task_info),
            SUCCESS);
  EXPECT_EQ(task_info.size(), 1);
  EXPECT_EQ(task_info["field1"], "revised1");
  // Get the task node with multiple specific fields
  task_info["field1"] = "";
  task_info["field2"] = "";
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "test1-task", task_info),
            SUCCESS);
  EXPECT_EQ(task_info.size(), 2);
  EXPECT_EQ(task_info["field1"], "revised1");
  EXPECT_EQ(task_info["field2"], "revised2");
  // Get the task node with all fields
  task_info.clear();
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "test1-task", task_info),
            SUCCESS);
  EXPECT_EQ(task_info.size(), 4);
  EXPECT_EQ(task_info["field1"], "revised1");
  EXPECT_EQ(task_info["field2"], "revised2");
  EXPECT_EQ(task_info["field3"], "revised3");
}

TEST(TestDB, TestGetAllUserNodes) {
  DB db(host);
  std::vector<std::string> user_pkeys;

  // Get all user nodes
  EXPECT_EQ(db.getAllUserNodes(user_pkeys), SUCCESS);
  EXPECT_EQ(user_pkeys.size(), 2);
  EXPECT_TRUE(std::find(user_pkeys.begin(), user_pkeys.end(),
                        "test0@test.com") != user_pkeys.end());
  EXPECT_TRUE(std::find(user_pkeys.begin(), user_pkeys.end(),
                        "test1@test.com") != user_pkeys.end());
}

TEST(TestDB, TestGetAllTaskListNodes) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::vector<std::string> task_list_pkeys;

  // Get all task list nodes
  EXPECT_EQ(db.getAllTaskListNodes("wrong@test.com", task_list_pkeys),
            ERR_NO_NODE);
  EXPECT_EQ(db.getAllTaskListNodes(user_pkey, task_list_pkeys), SUCCESS);
  EXPECT_EQ(task_list_pkeys.size(), 2);
  EXPECT_TRUE(std::find(task_list_pkeys.begin(), task_list_pkeys.end(),
                        "test0-task-list") != task_list_pkeys.end());
  EXPECT_TRUE(std::find(task_list_pkeys.begin(), task_list_pkeys.end(),
                        "test1-task-list") != task_list_pkeys.end());
}

TEST(TestDB, TestGetAllTaskNodes) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::string task_list_pkey = "test0-task-list";
  std::vector<std::string> task_pkeys;

  // Get all task nodes
  EXPECT_EQ(db.getAllTaskNodes("wrong@test.com", task_list_pkey, task_pkeys),
            ERR_NO_NODE);
  EXPECT_EQ(db.getAllTaskNodes(user_pkey, "wrong-task-list", task_pkeys),
            ERR_NO_NODE);
  EXPECT_EQ(db.getAllTaskNodes(user_pkey, task_list_pkey, task_pkeys), SUCCESS);
  EXPECT_EQ(task_pkeys.size(), 2);
  EXPECT_TRUE(std::find(task_pkeys.begin(), task_pkeys.end(), "test0-task") !=
              task_pkeys.end());
  EXPECT_TRUE(std::find(task_pkeys.begin(), task_pkeys.end(), "test1-task") !=
              task_pkeys.end());
}

TEST(TestDB, TestDeleteTaskNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::string task_list_pkey = "test0-task-list";
  std::map<std::string, std::string> void_info;

  // Delete the task node
  // Node must be in the DB
  EXPECT_EQ(db.deleteTaskNode(user_pkey, task_list_pkey, "wrong-task"),
            SUCCESS);
  EXPECT_EQ(db.deleteTaskNode(user_pkey, task_list_pkey, "test0-task"),
            SUCCESS);
  EXPECT_EQ(db.getTaskNode(user_pkey, task_list_pkey, "test0-task", void_info),
            ERR_NO_NODE);
}

TEST(TestDB, TestDeleteTaskListNode) {
  DB db(host);
  std::string user_pkey = "test0@test.com";
  std::map<std::string, std::string> void_info;

  // Delete the task list node
  // Node can not be in the DB
  EXPECT_EQ(db.deleteTaskListNode(user_pkey, "wrong-task-list"), SUCCESS);
  EXPECT_EQ(db.deleteTaskListNode(user_pkey, "test0-task-list"), SUCCESS);
  EXPECT_EQ(db.getTaskListNode(user_pkey, "test0-task-list", void_info),
            ERR_NO_NODE);
  // Ensure delete the task list node with all tasks
  EXPECT_EQ(
      db.getTaskNode(user_pkey, "test0-task-list", "test1-task", void_info),
      ERR_NO_NODE);
}

TEST(TestDB, TestDeleteUserNode) {
  DB db(host);
  std::map<std::string, std::string> void_info;

  // Delete the user node
  // Node can not be in the DB
  EXPECT_EQ(db.deleteUserNode("wrong@test.com"), SUCCESS);
  EXPECT_EQ(db.deleteUserNode("test0@test.com"), SUCCESS);
  EXPECT_EQ(db.getUserNode("test0@test.com", void_info), ERR_NO_NODE);
  // Ensure delete the user node with all task lists
  EXPECT_EQ(db.getTaskListNode("test0@test.com", "test1-task-list", void_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.getTaskNode("test0@test.com", "test1-task-list", "test1-task",
                           void_info),
            ERR_NO_NODE);
  EXPECT_EQ(db.deleteUserNode("test1@test.com"), SUCCESS);
  EXPECT_EQ(db.getUserNode("test1@test.com", void_info), ERR_NO_NODE);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}