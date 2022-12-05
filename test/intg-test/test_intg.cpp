#include <common/errorCode.h>
#include <db/DB.h>
#include <exception>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <tasklists/tasklistsWorker.h>
#include <tasks/tasksWorker.h>
#include <unordered_map>
#include <users/users.h>
using namespace ::testing;

class IntgTest : public ::testing::Test {
protected:
  void SetUp() override {
    // db model
    const std::string db_host = "neo4j://neo4j:hello4156@localhost:7687";
    db = std::make_shared<DB>(db_host);
    db->deleteEverything(); // clear db

    // users controller
    users = std::make_shared<Users>(db);

    // tasklists controller
    taskListsWorker = std::make_shared<TaskListsWorker>(db, users);

    // tasks controller
    tasksWorker = std::make_shared<TasksWorker>(db, taskListsWorker);
  }

  void TearDown() override {
    // no need to delete pointer
  }

  bool SetUpAlice();
  bool SetUpBob();
  bool SetUpSam();
  bool ShareTaskList(std::string, std::string, std::string, bool);
  bool SetUpTasksForAlice();

  // help to memorize the user info
  std::unordered_map<std::string, std::string> name2Email;
  std::unordered_map<std::string, std::string> name2Passwd;

  std::shared_ptr<DB> db;
  std::shared_ptr<TaskListsWorker> taskListsWorker;
  std::shared_ptr<TasksWorker> tasksWorker;
  std::shared_ptr<Users> users;
};

bool IntgTest::SetUpAlice() {
  // setup environment
  // create user first
  users->Create(UserInfo("alice", "alice@columbia.edu", "012345"));
  RequestData data("alice@columbia.edu", "", "", "");
  name2Email["alice"] = "alice@columbia.edu";
  name2Passwd["alice"] = "012345";

  // create four tasklists
  std::string outName;
  TasklistContent in("tasklist0", "tasklist0 description", "private");
  returnCode ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist0") {
    std::cout << "alice creates tasklist0 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist1", "tasklist1 description", "shared");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist1") {
    std::cout << "alice creates tasklist1 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist2", "tasklist2 description", "public");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist2") {
    std::cout << "alice creates tasklist2 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist3", "", "");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist3") {
    std::cout << "alice creates tasklist3 failed" << std::endl;
    return false;
  }

  return true;
}

bool IntgTest::SetUpBob() {
  // create user first
  users->Create(UserInfo("bob", "bob@columbia.edu", "678901"));
  RequestData data("bob@columbia.edu", "", "", "");
  name2Email["bob"] = "bob@columbia.edu";
  name2Passwd["bob"] = "678901";

  // create three tasklists
  std::string outName;
  TasklistContent in("tasklist4", "tasklist4 description", "private");
  returnCode ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist4") {
    std::cout << "bob creates tasklist4 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist5", "tasklist5 description", "shared");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist5") {
    std::cout << "bob creates tasklist5 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist6", "tasklist6 description", "public");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist6") {
    std::cout << "bob creates tasklist6 failed" << std::endl;
    return false;
  }

  return true;
}

bool IntgTest::SetUpSam() {
  // create user first
  users->Create(UserInfo("sam", "sam@columbia.edu", "234567"));
  RequestData data("sam@columbia.edu", "", "", "");
  name2Email["sam"] = "sam@columbia.edu";
  name2Passwd["sam"] = "234567";

  // create three tasklists
  std::string outName;
  TasklistContent in("tasklist7", "tasklist7 description", "private");
  returnCode ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist7") {
    std::cout << "sam creates tasklist7 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist8", "tasklist8 description", "shared");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist8") {
    std::cout << "sam creates tasklist8 failed" << std::endl;
    return false;
  }

  in = TasklistContent("tasklist9", "tasklist9 description", "public");
  ret = taskListsWorker->Create(data, in, outName);
  if (ret != SUCCESS || outName != "tasklist9") {
    std::cout << "sam creates tasklist9 failed" << std::endl;
    return false;
  }

  return true;
}

bool IntgTest::ShareTaskList(std::string src, std::string tasklist_name,
                             std::string dst, bool permission) {
  // this func must be called after SetUp for src and dst
  RequestData data(name2Email[src], tasklist_name, "", "");

  shareInfo info;
  info.user_name = name2Email[dst];
  info.permission = permission;

  std::vector<shareInfo> in_list;
  in_list.push_back(info);

  std::string errUser;
  returnCode ret = taskListsWorker->ReviseGrantTaskList(data, in_list, errUser);

  if (ret != SUCCESS || errUser != "") {
    std::cout << "share tasklist failed" << std::endl;
    return false;
  }

  return true;
}

bool IntgTest::SetUpTasksForAlice() {
  // this func must be called after SetUpAlice()
  RequestData data(name2Email["alice"], "", "", "");

  // create tasks
  TaskContent task0("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                    VERY_URGENT, "To Do");
  TaskContent task1("task1", "4156 Iteration-3", "11/30/2022", "12/29/2022",
                    URGENT, "Doing");
  TaskContent task2("task2", "4156 Iteration-4", "12/31/2022", "01/29/2023",
                    NORMAL, "Done");

  // add tasks to tasklists
  std::string outTaskName;
  data.tasklist_key = "tasklist0";
  returnCode ret = tasksWorker->Create(data, task0, outTaskName);
  if (ret != SUCCESS || outTaskName != "task0") {
    std::cout << "alice creates task0 failed" << std::endl;
    return false;
  }

  data.tasklist_key = "tasklist1";
  ret = tasksWorker->Create(data, task1, outTaskName);
  if (ret != SUCCESS || outTaskName != "task1") {
    std::cout << "alice creates task1 failed" << std::endl;
    return false;
  }

  data.tasklist_key = "tasklist2";
  ret = tasksWorker->Create(data, task2, outTaskName);
  if (ret != SUCCESS || outTaskName != "task2") {
    std::cout << "alice creates task2 failed" << std::endl;
    return false;
  }

  return true;
}

/*
 * @brief Integration test for users
 */

TEST_F(IntgTest, UsersCreate) {
  // should be able to create a user
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->Create(UserInfo("bob", "bob@columbia.edu", "123456")));
  EXPECT_TRUE(
      users->Create(UserInfo("alice2", "alice2@columbia.edu", "123456")));

  // duplicate email failed
  EXPECT_FALSE(
      users->Create(UserInfo("alice3", "alice@columbia.edu", "123456")));

  // email format failed
  EXPECT_FALSE(users->Create(UserInfo("bob", "bob@", "123456")));
};

TEST_F(IntgTest, UsersDelete) {
  // no such user, but still return true
  EXPECT_TRUE(users->Delete(UserInfo("alice", "alice@columbia.edu", "123456")));

  // create user first
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));

  // email format failed
  EXPECT_FALSE(users->Delete(UserInfo("alice", "alice@", "123456")));

  // should be able to delete a user
  EXPECT_TRUE(users->Delete(UserInfo("alice", "alice@columbia.edu", "123456")));

  // Deduplication
  EXPECT_TRUE(users->Delete(UserInfo("alice", "alice@columbia.edu", "123456")));

  // no such user
  EXPECT_FALSE(users->Delete(UserInfo("bob", "bob@", "123456")));
};

TEST_F(IntgTest, UsersValidate) {
  // no such user
  EXPECT_FALSE(users->Validate(UserInfo("", "alice@columbia.edu", "123456")));

  // should be able to validate a user
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->Validate(UserInfo("", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(
      users->Validate(UserInfo("alice", "alice@columbia.edu", "123456")));

  // wrong name
  EXPECT_FALSE(
      users->Validate(UserInfo("not alice", "alice@columbia.edu", "123456")));

  // wrong password
  EXPECT_FALSE(users->Validate(
      UserInfo("alice", "alice@columbia.edu", "wrong password")));

  // wrong email
  EXPECT_FALSE(users->Validate(
      UserInfo("alice", "wrong@columbia.edu", "wrong password")));

  // email format failed
  EXPECT_FALSE(users->Validate(UserInfo("alice", "alice@", "123456")));

  // missing password
  EXPECT_FALSE(users->Validate(UserInfo("", "alice@columbia.edu", "")));
  EXPECT_FALSE(users->Validate(UserInfo("alice", "alice@columbia.edu", "")));
};

TEST_F(IntgTest, UsersDuplicatedEmail) {
  // should be successful
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "123456")));
  EXPECT_TRUE(users->DuplicatedEmail(UserInfo("alice@columbia.edu")));

  // no such user
  EXPECT_FALSE(users->DuplicatedEmail(UserInfo("bob@columbia.edu")));
};

/*
 * @brief Integration test for TaskLists
 */

TEST_F(IntgTest, TaskListsCreate) {
  // setup environment
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "012345")));
  RequestData data("alice@columbia.edu", "", "", "");
  TasklistContent in("tasklist0", "tasklist0 description", "private");
  std::string outName;

  // should be able to create a tasklist
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  // Duplicate tasklist name but should be successful
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0(1)");

  // Duplicate tasklist name but should be successful
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0(2)");

  // only one field: tasklist name
  in = TasklistContent("tasklist1", "", "");
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist1");
  outName = "";

  // request user_key is empty
  data.user_key = "";
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), ERR_RFIELD);
  EXPECT_EQ(outName, "");
  data.user_key = "alice@columbia.edu";

  // no such user
  data.user_key = "bob@columbia.edu";
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), ERR_NO_NODE);
  EXPECT_EQ(outName, "");
  data.user_key = "alice@columbia.edu";

  // tasklist name is empty
  in.name = "";
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), ERR_KEY);
  EXPECT_EQ(outName, "");
  in.name = "tasklist0";

  // error visibility format
  in.visibility = "wrong visibility";
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), ERR_FORMAT);
  EXPECT_EQ(outName, "");
  in.visibility = "private";
};

TEST_F(IntgTest, TaskListsQueryOwned) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // query private tasklists
  data.tasklist_key = "tasklist0";
  TasklistContent out;
  EXPECT_EQ(taskListsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "tasklist0 description");
  EXPECT_EQ(out.visibility, "private");

  // query shared tasklists
  out = TasklistContent();
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(taskListsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist1");
  EXPECT_EQ(out.content, "tasklist1 description");
  EXPECT_EQ(out.visibility, "shared");

  // query public tasklists
  out = TasklistContent();
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(taskListsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist2");
  EXPECT_EQ(out.content, "tasklist2 description");
  EXPECT_EQ(out.visibility, "public");

  // query tasklists with no description
  out = TasklistContent();
  data.tasklist_key = "tasklist3";
  EXPECT_EQ(taskListsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist3");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "private");

  // query no such tasklist
  out = TasklistContent();
  data.tasklist_key = "unknown_tasklist";
  EXPECT_EQ(taskListsWorker->Query(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");

  // tasklist_key is empty
  out = TasklistContent();
  data.tasklist_key = "";
  EXPECT_EQ(taskListsWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");
};

TEST_F(IntgTest, TaskListsQueryOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  RequestData data(name2Email["bob"], "", "", name2Email["alice"]);

  // bob query alice's private tasklists, should fail
  data.tasklist_key = "tasklist0";
  TasklistContent out;
  EXPECT_EQ(taskListsWorker->Query(data, out), ERR_ACCESS);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");

  // bob query alice's shared tasklists, but no access now
  data.tasklist_key = "tasklist1";
  out = TasklistContent();
  EXPECT_EQ(taskListsWorker->Query(data, out), ERR_ACCESS);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");

  // alice shares her tasklist1 with bob
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // bob query alice's shared tasklists, now he has access
  data.tasklist_key = "tasklist1";
  out = TasklistContent();
  EXPECT_EQ(taskListsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist1");
  EXPECT_EQ(out.content, "tasklist1 description");
  EXPECT_EQ(out.visibility, "shared");

  // bob query alice's public tasklists, result tasklist2
  data.tasklist_key = "tasklist2";
  out = TasklistContent();
  EXPECT_EQ(taskListsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist2");
  EXPECT_EQ(out.content, "tasklist2 description");
  EXPECT_EQ(out.visibility, "public");

  // bob query alice's "empty" tasklists, private in default
  data.tasklist_key = "tasklist3";
  out = TasklistContent();
  EXPECT_EQ(taskListsWorker->Query(data, out), ERR_ACCESS);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");
};

TEST_F(IntgTest, TaskListsDelete) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // delete tasklist0
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(taskListsWorker->Delete(data), SUCCESS);

  // delete tasklist1
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(taskListsWorker->Delete(data), SUCCESS);

  // delete tasklist2
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(taskListsWorker->Delete(data), SUCCESS);

  // delete tasklist3
  data.tasklist_key = "tasklist3";
  EXPECT_EQ(taskListsWorker->Delete(data), SUCCESS);

  // no such tasklist, but delete success
  data.tasklist_key = "unknown_tasklist";
  EXPECT_EQ(taskListsWorker->Delete(data), SUCCESS);

  // tasklist key empty
  data.tasklist_key = "";
  EXPECT_EQ(taskListsWorker->Delete(data), ERR_RFIELD);
};

TEST_F(IntgTest, TaskListsReviseOwned) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "012345")));
  RequestData data("alice@columbia.edu", "", "", "");

  // create tasklists
  std::string outName;
  TasklistContent in("tasklist0", "tasklist0 description", "private");
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  // revise tasklist content, success
  data.tasklist_key = "tasklist0";
  in = TasklistContent();
  in.content = "new tasklist0 description";
  EXPECT_EQ(taskListsWorker->Revise(data, in), SUCCESS);
  in.content = "";

  // revise tasklist visibility, success
  in.visibility = "public";
  EXPECT_EQ(taskListsWorker->Revise(data, in), SUCCESS);
  in.visibility = "";

  // revise multiple fields, success
  in.content = "new new tasklist0 description";
  in.visibility = "shared";
  EXPECT_EQ(taskListsWorker->Revise(data, in), SUCCESS);
  in = TasklistContent();

  // revise tasklist key, fail
  in.name = "tasklist0";
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_KEY);
  in.name = "";

  // error visibility
  in.visibility = "unknown_visibility";
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_FORMAT);
  in.visibility = "";

  // no tasklist key, fail
  data.tasklist_key = "";
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_RFIELD);

  // tasklist key not exist, fail
  data.tasklist_key = "unknown_tasklist";
  in.content = "unknown tasklist description";
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_NO_NODE);
  in.content = "";
  data.tasklist_key = "tasklist0";

  // no user key, fail
  data.user_key = "";
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_RFIELD);
  data.user_key = "alice@columbia.edu";

  // no revise fields, fail
  in = TasklistContent();
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_RFIELD);
};

TEST_F(IntgTest, TaskListsReviseOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  RequestData data(name2Email["bob"], "", "", name2Email["alice"]);

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // no access
  data.tasklist_key = "tasklist0";
  TasklistContent in("", "new tasklist0 description", "");
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_ACCESS);

  // shared tasklist, but no permission
  data.tasklist_key = "tasklist1";
  in.content = "new tasklist1 description";
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_ACCESS);

  // set permission to true
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", true));

  // normal revise with access, read-write permission, and does not try to
  // revise visibility, should be successful
  data.tasklist_key = "tasklist1";
  in = TasklistContent("", "new tasklist1 description", "");
  EXPECT_EQ(taskListsWorker->Revise(data, in), SUCCESS);

  // revise visibility, should fail
  in = TasklistContent("", "", "public");
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_REVISE);

  // revise public tasklist, should be successful
  data.tasklist_key = "tasklist2";
  in = TasklistContent("", "new tasklist2 description", "");
  EXPECT_EQ(taskListsWorker->Revise(data, in), SUCCESS);

  // revise visibility, should fail
  in = TasklistContent("", "", "private");
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_REVISE);

  // revise tasklist key, should fail
  data.tasklist_key = "tasklist2";
  in = TasklistContent("tasklist5", "", "");
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_KEY);

  // revise non-exist tasklist, should fail
  data.tasklist_key = "unknown_tasklist";
  in = TasklistContent("", "unknown tasklist description", "");
  EXPECT_EQ(taskListsWorker->Revise(data, in), ERR_NO_NODE);
}

TEST_F(IntgTest, TaskListsGetAllTasklist) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(users->Create(UserInfo("alice", "alice@columbia.edu", "012345")));
  RequestData data("alice@columbia.edu", "", "", "");

  // should be successful, but no tasklist
  std::vector<std::string> out;
  EXPECT_EQ(taskListsWorker->GetAllTasklist(data, out), SUCCESS);
  EXPECT_EQ(out.size(), 0);

  // create four different tasklists
  std::string outName;
  TasklistContent in("tasklist0", "tasklist0 description", "private");
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  in = TasklistContent("tasklist1", "tasklist1 description", "shared");
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist1");

  in = TasklistContent("tasklist2", "tasklist2 description", "public");
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist2");

  in = TasklistContent("tasklist3", "", "");
  EXPECT_EQ(taskListsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist3");

  // should be successful and get all four tasklists
  out = {};
  EXPECT_EQ(taskListsWorker->GetAllTasklist(data, out), SUCCESS);
  EXPECT_EQ(out.size(), 4);
  sort(out.begin(), out.end());
  EXPECT_EQ(out[0], "tasklist0");
  EXPECT_EQ(out[1], "tasklist1");
  EXPECT_EQ(out[2], "tasklist2");
  EXPECT_EQ(out[3], "tasklist3");

  // empty user key
  data.user_key = "";
  out = {};
  EXPECT_EQ(taskListsWorker->GetAllTasklist(data, out), ERR_RFIELD);
  EXPECT_EQ(out.size(), 0);

  // no such user
  data.user_key = "unknown@columbia.edu";
  out = {};
  EXPECT_EQ(taskListsWorker->GetAllTasklist(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.size(), 0);
};

TEST_F(IntgTest, TaskListsGetAllAccessTaskList) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  RequestData data(name2Email["bob"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // bob shares his tasklist5 with alice, with read-write permission
  EXPECT_TRUE(ShareTaskList("bob", "tasklist5", "alice", true));

  // check tasklists that bob can access
  std::vector<shareInfo> outList;
  EXPECT_EQ(taskListsWorker->GetAllAccessTaskList(data, outList), SUCCESS);
  EXPECT_EQ(outList.size(), 1);
  EXPECT_EQ(outList[0].user_name, "alice@columbia.edu");
  EXPECT_EQ(outList[0].task_list_name, "tasklist1");
  EXPECT_EQ(outList[0].permission, false);

  // check tasklists that alice can access
  data.user_key = "alice@columbia.edu";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllAccessTaskList(data, outList), SUCCESS);
  EXPECT_EQ(outList.size(), 1);
  EXPECT_EQ(outList[0].user_name, "bob@columbia.edu");
  EXPECT_EQ(outList[0].task_list_name, "tasklist5");
  EXPECT_EQ(outList[0].permission, true);

  // no such user
  data.user_key = "unknown@columbia.edu";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllAccessTaskList(data, outList), ERR_NO_NODE);
  EXPECT_EQ(outList.size(), 0);

  // empty user key
  data.user_key = "";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllAccessTaskList(data, outList), ERR_RFIELD);
  EXPECT_EQ(outList.size(), 0);
};

TEST_F(IntgTest, TaskListsGetAllGrantTaskList) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // alice check who can access her tasklist0, no one
  data.tasklist_key = "tasklist0";
  std::vector<shareInfo> outList;
  bool isPublic;
  EXPECT_EQ(taskListsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_ACCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);

  // alice check who can access her tasklist1, bob and sam
  data.tasklist_key = "tasklist1";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllGrantTaskList(data, outList, isPublic),
            SUCCESS);
  sort(outList.begin(), outList.end(),
       [](shareInfo a, shareInfo b) { return a.user_name < b.user_name; });
  EXPECT_EQ(outList.size(), 2);
  EXPECT_EQ(outList[0].user_name, "bob@columbia.edu");
  EXPECT_EQ(outList[0].task_list_name, "");
  EXPECT_EQ(outList[0].permission, false);

  EXPECT_EQ(outList[1].user_name, "sam@columbia.edu");
  EXPECT_EQ(outList[1].task_list_name, "");
  EXPECT_EQ(outList[1].permission, true);
  EXPECT_EQ(isPublic, false);

  // alice check who can access her tasklist2, public
  data.tasklist_key = "tasklist2";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllGrantTaskList(data, outList, isPublic),
            SUCCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, true);

  // alice check who can access her tasklist3, private
  data.tasklist_key = "tasklist3";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_ACCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);

  // no such tasklist
  data.tasklist_key = "unknown_tasklist";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_NO_NODE);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);

  // empty tasklist key
  data.tasklist_key = "";
  outList = {};
  EXPECT_EQ(taskListsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_RFIELD);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);
};

/*
 *  wait for future commit
 */
TEST_F(IntgTest, TaskListsGetVisibility) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");
  std::string vis;

  // query private tasklists
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(taskListsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "private");

  data.tasklist_key = "tasklist3";
  EXPECT_EQ(taskListsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "private");

  // query shared tasklists
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(taskListsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "shared");

  // query public tasklists
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(taskListsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "public");

  // no such tasklist
  data.tasklist_key = "unknown_tasklist";
  EXPECT_EQ(taskListsWorker->GetVisibility(data, vis), ERR_NO_NODE);
}

TEST_F(IntgTest, TaskListsReviseGrantTaskList) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist0 with bob, which is private
  data.tasklist_key = "tasklist0";
  shareInfo info;
  info.user_name = name2Email["bob"];
  info.permission = false;

  std::vector<shareInfo> in_list;
  in_list.push_back(info);

  std::string errUser;
  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_ACCESS);
  EXPECT_EQ(errUser, "");

  // alice shares her tasklist1 with bob, with read-only permission
  data.tasklist_key = "tasklist1";
  in_list[0].user_name = name2Email["bob"];
  in_list[0].permission = false;

  errUser = "";
  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            SUCCESS);
  EXPECT_EQ(errUser, "");

  // alice shares her tasklist1 with sam, with read-write permission
  in_list[0].user_name = name2Email["sam"];
  in_list[0].permission = true;

  errUser = "";
  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            SUCCESS);
  EXPECT_EQ(errUser, "");

  // alice changes her permission of tasklist2 (public) to read-only with bob
  // denied
  data.tasklist_key = "tasklist2";
  in_list[0].user_name = name2Email["bob"];
  in_list[0].permission = false;

  errUser = "";
  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_ACCESS);
  EXPECT_EQ(errUser, "");

  // alice adds access to a unknown user
  data.tasklist_key = "tasklist1";
  in_list[0].user_name = "unknown@columbia.edu";
  in_list[0].permission = true;

  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_NO_NODE);
  EXPECT_EQ(errUser, "unknown@columbia.edu");
  errUser = "";

  // alice wants to add access to a unknown tasklist
  data.tasklist_key = "unknown_tasklist";
  in_list[0].user_name = "bob@columbia.edu";
  in_list[0].permission = false;

  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_ACCESS);
  EXPECT_EQ(errUser, "");

  // ERR_RFIELD
  data.tasklist_key = "";
  EXPECT_EQ(taskListsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_RFIELD);
};

TEST_F(IntgTest, TaskListsRemoveGrantTaskList) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // alice remove bob's access to tasklist0
  data.tasklist_key = "tasklist0";
  data.other_user_key = name2Email["bob"];
  EXPECT_EQ(taskListsWorker->RemoveGrantTaskList(data), ERR_ACCESS);

  // alice remove bob's access to tasklist1
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(taskListsWorker->RemoveGrantTaskList(data), SUCCESS);

  // alice remove sam's access to tasklist1
  data.tasklist_key = "tasklist1";
  data.other_user_key = name2Email["sam"];
  EXPECT_EQ(taskListsWorker->RemoveGrantTaskList(data), SUCCESS);

  // alice remove bob's access to tasklist2
  data.tasklist_key = "tasklist2";
  data.other_user_key = name2Email["bob"];
  EXPECT_EQ(taskListsWorker->RemoveGrantTaskList(data), ERR_ACCESS);

  // alice remove unknown_user's access to tasklist1
  data.tasklist_key = "tasklist1";
  data.other_user_key = "unknown@columbia.edu";
  EXPECT_EQ(taskListsWorker->RemoveGrantTaskList(data), ERR_NO_NODE);

  // alice remove empty_user's access to tasklist1
  data.tasklist_key = "tasklist1";
  data.other_user_key = "";
  EXPECT_EQ(taskListsWorker->RemoveGrantTaskList(data), ERR_NO_NODE);
};

TEST_F(IntgTest, TaskListsGetAllPublicTaskList) {
  // none public tasklist
  std::vector<std::pair<std::string, std::string>> out_list;
  EXPECT_EQ(taskListsWorker->GetAllPublicTaskList(out_list), SUCCESS);
  EXPECT_EQ(out_list.size(), 0);

  // create first user
  EXPECT_TRUE(SetUpAlice());

  // get all public tasklist
  out_list = {};
  EXPECT_EQ(taskListsWorker->GetAllPublicTaskList(out_list), SUCCESS);
  EXPECT_EQ(out_list.size(), 1);
  EXPECT_EQ(out_list[0].first, name2Email["alice"]);
  EXPECT_EQ(out_list[0].second, "tasklist2");

  // create second user
  EXPECT_TRUE(SetUpBob());

  // get all public tasklist
  out_list = {};
  EXPECT_EQ(taskListsWorker->GetAllPublicTaskList(out_list), SUCCESS);
  sort(out_list.begin(), out_list.end());
  EXPECT_EQ(out_list.size(), 2);
  EXPECT_EQ(out_list[0].first, name2Email["alice"]);
  EXPECT_EQ(out_list[0].second, "tasklist2");
  EXPECT_EQ(out_list[1].first, name2Email["bob"]);
  EXPECT_EQ(out_list[1].second, "tasklist6");

  // create third user
  EXPECT_TRUE(SetUpSam());

  // get all public tasklist
  out_list = {};
  EXPECT_EQ(taskListsWorker->GetAllPublicTaskList(out_list), SUCCESS);
  sort(out_list.begin(), out_list.end());
  EXPECT_EQ(out_list.size(), 3);
  EXPECT_EQ(out_list[0].first, name2Email["alice"]);
  EXPECT_EQ(out_list[0].second, "tasklist2");
  EXPECT_EQ(out_list[1].first, name2Email["bob"]);
  EXPECT_EQ(out_list[1].second, "tasklist6");
  EXPECT_EQ(out_list[2].first, name2Email["sam"]);
  EXPECT_EQ(out_list[2].second, "tasklist9");
};

TEST_F(IntgTest, TaskListsExists) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // alice check if tasklist0 exists
  data.tasklist_key = "tasklist0";
  EXPECT_TRUE(taskListsWorker->Exists(data));

  // alice check if tasklist1 exists
  data.tasklist_key = "tasklist1";
  EXPECT_TRUE(taskListsWorker->Exists(data));

  // alice check if tasklist2 exists
  data.tasklist_key = "tasklist2";
  EXPECT_TRUE(taskListsWorker->Exists(data));

  // alice check if tasklist3 exists
  data.tasklist_key = "tasklist3";
  EXPECT_TRUE(taskListsWorker->Exists(data));

  // alice check empty tasklist name
  data.tasklist_key = "";
  EXPECT_FALSE(taskListsWorker->Exists(data));

  // bob check if tasklist0 exists
  data.user_key = name2Email["bob"];
  data.tasklist_key = "tasklist0";
  EXPECT_FALSE(taskListsWorker->Exists(data));

  // bob check if tasklist1 exists
  data.tasklist_key = "tasklist1";
  data.other_user_key = name2Email["alice"];
  EXPECT_TRUE(taskListsWorker->Exists(data));

  // bob check if tasklist2 exists, public
  data.tasklist_key = "tasklist2";
  EXPECT_TRUE(taskListsWorker->Exists(data));

  // bob check if tasklist3 exists, private
  data.tasklist_key = "tasklist3";
  EXPECT_FALSE(taskListsWorker->Exists(data));

  // sam check if tasklist1 exists
  data.user_key = name2Email["sam"];
  data.tasklist_key = "tasklist1";
  data.other_user_key = name2Email["alice"];
  EXPECT_TRUE(taskListsWorker->Exists(data));
};

/*
 * @brief Integration test for Tasks
 */

TEST_F(IntgTest, TasksQueryOwned) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // Setup tasks for alice's tasklists
  // tasklist0 with task0, tasklist1 with task1, tasklist2 with task2
  EXPECT_TRUE(SetUpTasksForAlice());

  // query tasks0
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  TaskContent out;
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task0");
  EXPECT_EQ(out.content, "4156 Iteration-2");
  EXPECT_EQ(out.startDate, "10/31/2022");
  EXPECT_EQ(out.endDate, "11/29/2022");
  EXPECT_EQ(out.priority, VERY_URGENT);
  EXPECT_EQ(out.status, "To Do");

  // query tasks1
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task1");
  EXPECT_EQ(out.content, "4156 Iteration-3");
  EXPECT_EQ(out.startDate, "11/30/2022");
  EXPECT_EQ(out.endDate, "12/29/2022");
  EXPECT_EQ(out.priority, URGENT);
  EXPECT_EQ(out.status, "Doing");

  // query tasks2
  data.tasklist_key = "tasklist2";
  data.task_key = "task2";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task2");
  EXPECT_EQ(out.content, "4156 Iteration-4");
  EXPECT_EQ(out.startDate, "12/31/2022");
  EXPECT_EQ(out.endDate, "01/29/2023");
  EXPECT_EQ(out.priority, NORMAL);
  EXPECT_EQ(out.status, "Done");

  // query tasks3, which does not exist
  data.tasklist_key = "tasklist2";
  data.task_key = "task3";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");

  // request failed
  data.tasklist_key = "";
  data.task_key = "";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
};

TEST_F(IntgTest, TasksQueryOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // Setup tasks for alice's tasklists
  // tasklist0 with task0, tasklist1 with task1, tasklist2 with task2
  EXPECT_TRUE(SetUpTasksForAlice());

  // change user to bob
  data.user_key = name2Email["bob"];
  data.other_user_key = name2Email["alice"];

  // bob query alice's tasks0, private tasklist, should fail
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  TaskContent out;
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_ACCESS);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");

  // bob query alice's tasks1
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task1");
  EXPECT_EQ(out.content, "4156 Iteration-3");
  EXPECT_EQ(out.startDate, "11/30/2022");
  EXPECT_EQ(out.endDate, "12/29/2022");
  EXPECT_EQ(out.priority, URGENT);
  EXPECT_EQ(out.status, "Doing");

  // bob query alice's tasks2
  data.tasklist_key = "tasklist2";
  data.task_key = "task2";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task2");
  EXPECT_EQ(out.content, "4156 Iteration-4");
  EXPECT_EQ(out.startDate, "12/31/2022");
  EXPECT_EQ(out.endDate, "01/29/2023");
  EXPECT_EQ(out.priority, NORMAL);
  EXPECT_EQ(out.status, "Done");

  // bob query alice's tasks3, which does not exist
  data.tasklist_key = "tasklist2";
  data.task_key = "task3";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");

  // change to sam
  data.user_key = name2Email["sam"];
  data.other_user_key = name2Email["alice"];

  // sam query alice's tasks1
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task1");
  EXPECT_EQ(out.content, "4156 Iteration-3");
  EXPECT_EQ(out.startDate, "11/30/2022");
  EXPECT_EQ(out.endDate, "12/29/2022");
  EXPECT_EQ(out.priority, URGENT);
  EXPECT_EQ(out.status, "Doing");

  // sam query alice's tasks2
  data.tasklist_key = "tasklist2";
  data.task_key = "task2";
  out = TaskContent();
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task2");
  EXPECT_EQ(out.content, "4156 Iteration-4");
  EXPECT_EQ(out.startDate, "12/31/2022");
  EXPECT_EQ(out.endDate, "01/29/2023");
  EXPECT_EQ(out.priority, NORMAL);
  EXPECT_EQ(out.status, "Done");
};

TEST_F(IntgTest, TasksCreateOwned) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // create tasks
  TaskContent task0("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                    VERY_URGENT, "To Do");
  TaskContent task1("task1", "4156 Iteration-3", "11/30/2022", "12/29/2022",
                    URGENT, "Doing");
  TaskContent task2("task2", "4156 Iteration-4", "12/31/2022", "01/29/2023",
                    NORMAL, "Done");
  TaskContent task3;

  // add tasks to tasklists
  std::string outTaskName;
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(tasksWorker->Create(data, task0, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0");

  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->Create(data, task1, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task1");

  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task2");

  // add multiple tasks to tasklist
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task2(1)");

  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task2(2)");
  outTaskName = "";

  // request is empty
  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Create(data, task3, outTaskName), ERR_RFIELD);
  EXPECT_EQ(outTaskName, "");

  // error format
  task2.status = "2/3 Done";
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), ERR_FORMAT);
  EXPECT_EQ(outTaskName, "");
  task2.status = "Done";
};

TEST_F(IntgTest, TasksCreateOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // create tasks
  TaskContent task0("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                    VERY_URGENT, "To Do");
  TaskContent task1("task1", "4156 Iteration-3", "11/30/2022", "12/29/2022",
                    URGENT, "Doing");
  TaskContent task2("task2", "4156 Iteration-4", "12/31/2022", "01/29/2023",
                    NORMAL, "Done");

  // change user to bob
  data.user_key = name2Email["bob"];
  data.other_user_key = name2Email["alice"];

  // bob tries to create task in alice's tasklist0
  data.tasklist_key = "tasklist0";
  std::string outTaskName;
  EXPECT_EQ(tasksWorker->Create(data, task0, outTaskName), ERR_ACCESS);
  EXPECT_EQ(outTaskName, "");

  // bob tries to create task in alice's tasklist1, but he has read-only
  // permission
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->Create(data, task1, outTaskName), ERR_ACCESS);
  EXPECT_EQ(outTaskName, "");

  // bob tries to create task in alice's tasklist2
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task2");

  // change user to sam
  data.user_key = name2Email["sam"];
  data.other_user_key = name2Email["alice"];

  // sam tries to create task in alice's tasklist1, and he has read-write
  // permission
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->Create(data, task1, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task1");
};

TEST_F(IntgTest, TasksDeleteOwned) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // Setup tasks for alice's tasklists
  // tasklist0 with task0, tasklist1 with task1, tasklist2 with task2
  EXPECT_TRUE(SetUpTasksForAlice());

  // delete tasks, should be successful
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  data.tasklist_key = "tasklist2";
  data.task_key = "task2";
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // delete tasks again, but still success since they are already deleted
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // tasklist does not exist
  data.tasklist_key = "unknown_tasklist";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_NO_NODE);

  // requeset is empty
  data.tasklist_key = "";
  data.task_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_RFIELD);
};

TEST_F(IntgTest, TasksDeleteOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // Setup tasks for alice's tasklists
  // tasklist0 with task0, tasklist1 with task1, tasklist2 with task2
  EXPECT_TRUE(SetUpTasksForAlice());

  // change user to bob
  data.user_key = name2Email["bob"];
  data.other_user_key = name2Email["alice"];

  // bob tries to delete task in alice's tasklist0, but he has no permission
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_ACCESS);

  // bob tries to delete task in alice's tasklist1, but he has read-only
  // permission
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_ACCESS);

  // bob tries to delete task in alice's tasklist2, which is public
  data.tasklist_key = "tasklist2";
  data.task_key = "task2";
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // change user to sam
  data.user_key = name2Email["sam"];
  data.other_user_key = name2Email["alice"];

  // sam tries to delete task in alice's tasklist1, he has read-write permission
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);
};

TEST_F(IntgTest, TasksReviseOwned) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // create tasks
  TaskContent task0("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                    VERY_URGENT, "To Do");
  TaskContent task1("task1", "4156 Iteration-3", "11/30/2022", "12/29/2022",
                    URGENT, "Doing");
  TaskContent task2("task2", "4156 Iteration-4", "12/31/2022", "01/29/2023",
                    NORMAL, "Done");
  TaskContent in("", "new task0 description", "", "", NORMAL, "");

  // add tasks to tasklists
  std::string outTaskName;
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(tasksWorker->Create(data, task0, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0");

  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->Create(data, task1, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task1");

  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task2");

  // alice tries to revise task in tasklist0
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // alice tries to revise task in tasklist1
  in = TaskContent("", "", "12/31/2022", "01/29/2023", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // alice tries to revise task_name in tasklist1, which is not allowed
  in = TaskContent("task3", "", "", "", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_KEY);

  // errro format
  in = TaskContent("", "", "12/32/2022", "01/29/2023", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_FORMAT);

  // input is empty
  in = TaskContent();
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_RFIELD);

  // request is empty
  in = TaskContent("", "", "", "", NULL_PRIORITY, "To Do");
  data.tasklist_key = "tasklist1";
  data.task_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_RFIELD);

  // ERR_NO_NODE
  data.tasklist_key = "tasklist1";
  data.task_key = "task3";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_NO_NODE);
};

TEST_F(IntgTest, TasksReviseOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // Setup tasks for alice's tasklists
  // tasklist0 with task0, tasklist1 with task1, tasklist2 with task2
  EXPECT_TRUE(SetUpTasksForAlice());

  // change user to bob
  data.user_key = name2Email["bob"];
  data.other_user_key = name2Email["alice"];

  // bob tries to revise task in tasklist0, which is not allowed
  TaskContent in("", "", "11/30/2022", "12/29/2022", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_ACCESS);

  // bob tries to revise task in tasklist1, but he has read-only permission
  in = TaskContent("", "", "12/30/2022", "01/29/2023", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_ACCESS);

  // bob tries to revise task in tasklist2, which is public
  in = TaskContent("", "", "01/30/2023", "02/25/2023", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist2";
  data.task_key = "task2";
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // change user to sam
  data.user_key = name2Email["sam"];
  data.other_user_key = name2Email["alice"];

  // sam tries to revise task in tasklist1, he has read-write permission
  in = TaskContent("", "", "12/30/2022", "01/29/2023", NULL_PRIORITY, "");
  data.tasklist_key = "tasklist1";
  data.task_key = "task1";
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);
};

TEST_F(IntgTest, TasksGetAllTasksNameOwned) {
  // setup environment
  // crreate user first
  EXPECT_TRUE(SetUpAlice());
  RequestData data(name2Email["alice"], "", "", "");

  // create tasks
  TaskContent task0("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                    VERY_URGENT, "To Do");
  TaskContent task1("task1", "4156 Iteration-3", "11/30/2022", "12/29/2022",
                    URGENT, "Doing");
  TaskContent task2("task2", "4156 Iteration-4", "12/31/2022", "01/29/2023",
                    NORMAL, "Done");
  TaskContent in("", "new task0 description", "", "", NORMAL, "");

  // add tasks to tasklists
  std::string outTaskName;
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(tasksWorker->Create(data, task0, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0");

  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->Create(data, task1, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task1");

  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->Create(data, task2, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task2");

  // get all tasks name owned by alice in tasklist0
  std::vector<std::string> outTaskNames;
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 1);
  EXPECT_EQ(outTaskNames[0], "task0");

  // get all tasks name owned by alice in tasklist1
  outTaskNames.clear();
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 1);
  EXPECT_EQ(outTaskNames[0], "task1");

  // get all tasks name owned by alice in tasklist2
  outTaskNames.clear();
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 1);
  EXPECT_EQ(outTaskNames[0], "task2");

  // get all tasks name owned by alice in tasklist3
  outTaskNames.clear();
  data.tasklist_key = "tasklist3";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 0);

  // non-exist tasklist
  outTaskNames.clear();
  data.tasklist_key = "unknown_tasklist";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), ERR_NO_NODE);
  EXPECT_EQ(outTaskNames.size(), 0);

  // request data is empty
  outTaskNames.clear();
  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), ERR_RFIELD);
  EXPECT_EQ(outTaskNames.size(), 0);
};

TEST_F(IntgTest, TasksGetAllTasksNameOthers) {
  // setup environment
  EXPECT_TRUE(SetUpAlice());
  EXPECT_TRUE(SetUpBob());
  EXPECT_TRUE(SetUpSam());
  RequestData data(name2Email["alice"], "", "", "");

  // alice shares her tasklist1 with bob, with read-only permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "bob", false));

  // alice shares her tasklist1 with sam, with read-write permission
  EXPECT_TRUE(ShareTaskList("alice", "tasklist1", "sam", true));

  // Setup tasks for alice's tasklists
  // tasklist0 with task0, tasklist1 with task1, tasklist2 with task2
  EXPECT_TRUE(SetUpTasksForAlice());

  // change user to bob
  data.user_key = name2Email["bob"];
  data.other_user_key = name2Email["alice"];

  // get all tasks name owned by alice in tasklist0, bob has no permission
  std::vector<std::string> outTaskNames;
  data.tasklist_key = "tasklist0";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), ERR_ACCESS);
  EXPECT_EQ(outTaskNames.size(), 0);

  // get all tasks name owned by alice in tasklist1, bob has read-only
  // permission
  outTaskNames.clear();
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 1);
  EXPECT_EQ(outTaskNames[0], "task1");

  // get all tasks name owned by alice in tasklist2, which is public
  outTaskNames.clear();
  data.tasklist_key = "tasklist2";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 1);
  EXPECT_EQ(outTaskNames[0], "task2");

  // change user to sam
  data.user_key = name2Email["sam"];
  data.other_user_key = name2Email["alice"];

  // get all tasks name owned by alice in tasklist1, sam has read-write
  // permission
  outTaskNames.clear();
  data.tasklist_key = "tasklist1";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, outTaskNames), SUCCESS);
  EXPECT_EQ(outTaskNames.size(), 1);
  EXPECT_EQ(outTaskNames[0], "task1");
};

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
