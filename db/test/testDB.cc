#include "DB.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>

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
  task_list_info["visibility"] = "public";
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
  EXPECT_EQ(task_list_info.size(), 5);
  EXPECT_EQ(task_list_info["field1"], "revised1");
  EXPECT_EQ(task_list_info["field2"], "revised2");
  EXPECT_EQ(task_list_info["field3"], "revised3");
  EXPECT_EQ(task_list_info["visibility"], "public");
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

TEST(TestDB, TestAddAccess) {
  DB db(host);
  std::string src_user_pkey = "test0@test.com";
  std::string dst_user_pkey = "test1@test.com";
  std::string task_list_pkey0 = "test0-task-list";
  std::string task_list_pkey1 = "test1-task-list";

  // Add access
  // Error: src_user_pkey does not exist
  EXPECT_EQ(
      db.addAccess("wrong@test.com", dst_user_pkey, task_list_pkey0, true),
      ERR_NO_NODE);
  // Error: dst_user_pkey does not exist
  EXPECT_EQ(
      db.addAccess(src_user_pkey, "wrong@test.com", task_list_pkey0, true),
      ERR_NO_NODE);
  // Error: task_list_pkey does not exist
  EXPECT_EQ(db.addAccess(src_user_pkey, dst_user_pkey, "wrong-task-list", true),
            ERR_NO_NODE);
  // Error: task list visibility is private
  EXPECT_EQ(db.addAccess(src_user_pkey, dst_user_pkey, task_list_pkey0, true),
            ERR_ACCESS);
  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"] = "public";
  EXPECT_EQ(
      db.reviseTaskListNode(src_user_pkey, task_list_pkey0, task_list_info),
      SUCCESS);
  EXPECT_EQ(db.addAccess(src_user_pkey, dst_user_pkey, task_list_pkey0, true),
            SUCCESS);
  EXPECT_EQ(db.addAccess(src_user_pkey, dst_user_pkey, task_list_pkey1, false),
            SUCCESS);
}

TEST(TestDB, TestGetAccess) {
  DB db(host);
  std::string src_user_pkey = "test0@test.com";
  std::string dst_user_pkey = "test1@test.com";
  std::string task_list_pkey0 = "test0-task-list";
  std::string task_list_pkey1 = "test1-task-list";
  bool read_write;

  // Check access
  // Error: src_user_pkey does not exist
  EXPECT_EQ(db.checkAccess("wrong@test.com", dst_user_pkey, task_list_pkey0,
                           read_write),
            ERR_NO_NODE);
  // Error: dst_user_pkey does not exist
  EXPECT_EQ(db.checkAccess(src_user_pkey, "wrong@test.com", task_list_pkey0,
                           read_write),
            ERR_NO_NODE);
  // Error: task_list_pkey does not exist
  EXPECT_EQ(db.checkAccess(src_user_pkey, dst_user_pkey, "wrong-task-list",
                           read_write),
            ERR_NO_NODE);
  EXPECT_EQ(
      db.checkAccess(src_user_pkey, dst_user_pkey, task_list_pkey0, read_write),
      SUCCESS);
  EXPECT_TRUE(read_write);
  EXPECT_EQ(
      db.checkAccess(src_user_pkey, dst_user_pkey, task_list_pkey1, read_write),
      SUCCESS);
  EXPECT_FALSE(read_write);
  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"] = "private";
  EXPECT_EQ(
      db.reviseTaskListNode(src_user_pkey, task_list_pkey0, task_list_info),
      SUCCESS);
  EXPECT_EQ(
      db.checkAccess(src_user_pkey, dst_user_pkey, task_list_pkey0, read_write),
      ERR_ACCESS);
}

TEST(TestDB, TestReviseAccess) {
  DB db(host);
  std::string src_user_pkey = "test0@test.com";
  std::string dst_user_pkey = "test1@test.com";
  std::string task_list_pkey = "test0-task-list";
  bool read_write;

  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"] = "public";
  EXPECT_EQ(
      db.reviseTaskListNode(src_user_pkey, task_list_pkey, task_list_info),
      SUCCESS);

  // Revise access
  EXPECT_EQ(db.addAccess(src_user_pkey, dst_user_pkey, task_list_pkey, false),
            SUCCESS);
  EXPECT_EQ(
      db.checkAccess(src_user_pkey, dst_user_pkey, task_list_pkey, read_write),
      SUCCESS);
  EXPECT_FALSE(read_write);
  task_list_pkey = "test1-task-list";
  EXPECT_EQ(db.addAccess(src_user_pkey, dst_user_pkey, task_list_pkey, true),
            SUCCESS);
  EXPECT_EQ(
      db.checkAccess(src_user_pkey, dst_user_pkey, task_list_pkey, read_write),
      SUCCESS);
  EXPECT_TRUE(read_write);
}

TEST(TestDB, TestAllAccess) {
  DB db(host);
  std::string src_user_pkey = "test0@test.com";
  std::string dst_user_pkey = "test1@test.com";
  std::map<std::pair<std::string, std::string>, bool> list_accesses;
  bool read_write;

  // Get all access
  // Error: src_user_pkey does not exist
  EXPECT_EQ(db.allAccess("wrong@test.com", dst_user_pkey, list_accesses),
            ERR_NO_NODE);
  // Error: dst_user_pkey does not exist
  EXPECT_EQ(db.allAccess(src_user_pkey, "wrong@test.com", list_accesses),
            ERR_NO_NODE);
  EXPECT_EQ(db.allAccess(src_user_pkey, dst_user_pkey, list_accesses), SUCCESS);
  EXPECT_EQ(list_accesses.size(), 2);
  read_write = list_accesses[{src_user_pkey, "test0-task-list"}];
  EXPECT_FALSE(read_write);
  read_write = list_accesses[{src_user_pkey, "test1-task-list"}];
  EXPECT_TRUE(read_write);
  // Will not print private task list
  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"] = "private";
  EXPECT_EQ(
      db.reviseTaskListNode(src_user_pkey, "test0-task-list", task_list_info),
      SUCCESS);
  EXPECT_EQ(db.allAccess(src_user_pkey, dst_user_pkey, list_accesses), SUCCESS);
  EXPECT_EQ(list_accesses.size(), 1);
  read_write = list_accesses[{src_user_pkey, "test1-task-list"}];
  EXPECT_TRUE(read_write);
  // Empty list
  EXPECT_EQ(db.allAccess(dst_user_pkey, src_user_pkey, list_accesses), SUCCESS);
  EXPECT_EQ(list_accesses.size(), 0);
}

TEST(TestDB, TestAllGrant) {
  DB db(host);
  std::string src_user_pkey = "test0@test.com";
  std::string task_list_pkey = "test1-task-list";
  std::map<std::string, bool> list_grants;

  // Get all grant
  // Error: src_user_pkey does not exist
  EXPECT_EQ(db.allGrant("wrong@test.com", task_list_pkey, list_grants),
            ERR_NO_NODE);
  // Error: task_list_pkey does not exist
  EXPECT_EQ(db.allGrant(src_user_pkey, "wrong-task-list", list_grants),
            ERR_NO_NODE);
  EXPECT_EQ(db.allGrant(src_user_pkey, task_list_pkey, list_grants), SUCCESS);
  EXPECT_EQ(list_grants.size(), 1);
  bool read_write = list_grants["test1@test.com"];
  EXPECT_TRUE(read_write);
  EXPECT_EQ(db.allGrant(src_user_pkey, "test0-task-list", list_grants),
            SUCCESS);
  EXPECT_EQ(list_grants.size(), 0);
}

TEST(TestDB, TestRemoveAccess) {
  DB db(host);
  std::string src_user_pkey = "test0@test.com";
  std::string dst_user_pkey = "test1@test.com";
  std::string task_list_pkey = "test1-task-list";
  bool read_write;

  // Remove access
  // Error: src_user_pkey does not exist
  EXPECT_EQ(db.removeAccess("wrong@test.com", dst_user_pkey, task_list_pkey),
            SUCCESS);
  // Error: dst_user_pkey does not exist
  EXPECT_EQ(db.removeAccess(src_user_pkey, "wrong@test.com", task_list_pkey),
            SUCCESS);
  // Error: task_list_pkey does not exist
  EXPECT_EQ(db.removeAccess(src_user_pkey, dst_user_pkey, "wrong-task-list"),
            SUCCESS);
  EXPECT_EQ(db.removeAccess(src_user_pkey, dst_user_pkey, task_list_pkey),
            SUCCESS);
  EXPECT_EQ(db.removeAccess(src_user_pkey, dst_user_pkey, task_list_pkey),
            SUCCESS);
  EXPECT_EQ(
      db.checkAccess(src_user_pkey, dst_user_pkey, task_list_pkey, read_write),
      ERR_ACCESS);
  std::map<std::string, bool> list_grants;
  EXPECT_EQ(db.allGrant(src_user_pkey, "test1-task-list", list_grants),
            SUCCESS);
  EXPECT_EQ(list_grants.size(), 0);
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
  // Ensure delete the task list node with all accesses
  std::map<std::string, bool> list_grants;
  EXPECT_EQ(db.allGrant("test0@test.com", "test0-task-list", list_grants),
            ERR_NO_NODE);
  EXPECT_EQ(list_grants.size(), 0);
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
  bool void_bool;
  EXPECT_EQ(db.checkAccess("test0@test.com", "test1@test.com",
                           "test1-task-list", void_bool),
            ERR_NO_NODE);
  EXPECT_EQ(db.deleteUserNode("test1@test.com"), SUCCESS);
  EXPECT_EQ(db.getUserNode("test1@test.com", void_info), ERR_NO_NODE);
}

void create_thread(int id, DB *db) {
  std::string user_pkey = "test" + std::to_string(id) + "@test.com";
  std::map<std::string, std::string> user_info;

  // Create the user node
  user_info["email"] = user_pkey;
  user_info["passwd"] = "test";
  user_info["test"] = std::to_string(id);
  EXPECT_EQ(db->createUserNode(user_info), SUCCESS);
}

void delete_thread(int id, DB *db) {
  std::string user_pkey = "test" + std::to_string(id) + "@test.com";

  // Delete the user node
  EXPECT_EQ(db->deleteUserNode(user_pkey), SUCCESS);
}

TEST(TestDB, TestMultiThread) {
  DB db(host);
  std::map<std::string, std::string> void_info;
  const int thread_num = 100;

  // Create thread_num user nodes
  std::vector<std::thread> threads;
  for (int i = 0; i < thread_num; i++) {
    threads.push_back(std::thread(create_thread, i, &db));
  }
  for (auto &t : threads) {
    t.join();
  }
  for (int i = 0; i < thread_num; i++) {
    EXPECT_EQ(
        db.getUserNode("test" + std::to_string(i) + "@test.com", void_info),
        SUCCESS);
    EXPECT_EQ(void_info["test"], std::to_string(i));
  }

  // Delete thread_num user nodes
  threads.clear();
  for (int i = 0; i < thread_num; i++) {
    threads.push_back(std::thread(delete_thread, i, &db));
  }
  for (auto &t : threads) {
    t.join();
  }
  for (int i = 0; i < thread_num; i++) {
    EXPECT_EQ(
        db.getUserNode("test" + std::to_string(i) + "@test.com", void_info),
        ERR_NO_NODE);
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}