#include <api/tasklistContent.h>
#include <db/DB.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <tasklists/tasklistsWorker.h>

class MockedDB : public DB {
public:
  MOCK_METHOD(returnCode, createTaskListNode,
              (const std::string &user_pkey,
               (const std::map<std::string, std::string> &)task_list_info),
              (override));
  MOCK_METHOD(returnCode, getTaskListNode,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               (std::map<std::string, std::string> &)task_list_info),
              (override));
  MOCK_METHOD(returnCode, deleteTaskListNode,
              (const std::string &user_pkey, const std::string &task_list_pkey),
              (override));
  MOCK_METHOD(returnCode, reviseTaskListNode,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               (const std::map<std::string, std::string> &)task_list_info),
              (override));
  MOCK_METHOD(returnCode, getAllTaskListNodes,
              (const std::string &user_pkey,
               std::vector<std::string> &outNames),
              (override));
  MOCK_METHOD(returnCode, addAccess,
              (const std::string &src_user_pkey,
               const std::string &dst_user_pkey,
               const std::string &task_list_pkey, const bool read_write),
              (override));
  MOCK_METHOD(returnCode, checkAccess,
              (const std::string &src_user_pkey,
               const std::string &dst_user_pkey,
               const std::string &task_list_pkey, bool &read_write),
              (override));
  MOCK_METHOD(returnCode, removeAccess,
              (const std::string &src_user_pkey,
               const std::string &dst_user_pkey,
               const std::string &task_list_pkey),
              (override));
  MOCK_METHOD(
      returnCode, allAccess,
      (const std::string &dst_user_pkey,
       (std::map<std::pair<std::string, std::string>, bool> &)list_accesses),
      (override));
  MOCK_METHOD(returnCode, allGrant,
              (const std::string &src_user_pkey,
               const std::string &task_list_pkey,
               (std::map<std::string, bool> &)list_grants),
              (override));
  MOCK_METHOD(returnCode, getAllPublic,
              ((std::vector<std::pair<std::string, std::string>> &)user_list),
              (override));

  MockedDB() : DB("testhost") {}
};

class MockedUsers : public Users {
public:
  MOCK_METHOD(bool, DuplicatedEmail, (const UserInfo &), (override));

  MockedUsers(std::shared_ptr<DB> _db) : Users(_db) {}
};

class TaskListTest : public ::testing::Test {
protected:
  void SetUp() override {
    mockedDB = std::make_shared<MockedDB>();
    mockedUsers = std::make_shared<MockedUsers>(mockedDB);
    tasklistsWorker = std::make_shared<TaskListsWorker>(mockedDB, mockedUsers);
  }

  void TearDown() override {
    // pass
  }

  std::shared_ptr<MockedDB> mockedDB;
  std::shared_ptr<MockedUsers> mockedUsers;
  std::shared_ptr<TaskListsWorker> tasklistsWorker;
  RequestData data;
  TasklistContent in;
  TasklistContent out;
};

using namespace ::testing;

TEST_F(TaskListTest, QueryOwned) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  out = TasklistContent();
  std::map<std::string, std::string> task_list_info;
  std::map<std::string, std::string> new_task_list_info = {
      {"name", "tasklist0"},
      {"content", "this is tasklist #0"},
      {"visibility", "private"}};

  // normal get, should be successful
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "this is tasklist #0");
  EXPECT_EQ(out.visibility, "private");

  // request tasklist_key is empty
  data.user_key = "user0";
  data.tasklist_key = "";
  out = TasklistContent();
  EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");

  // request user_key is empty
  data.user_key = "";
  data.tasklist_key = "";
  out = TasklistContent();
  EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");
}

TEST_F(TaskListTest, QueryAccess) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  data.other_user_key = "anotherUser0";
  out = TasklistContent();
  std::map<std::string, std::string> task_list_info;
  std::map<std::string, std::string> new_task_list_info = {
      {"name", "tasklist0"},
      {"content", "this is tasklist #0"},
      {"visibility", "shared"}};
  bool permission = false;

  // normal get with access, and read-write permission, should be successful
  out = TasklistContent();
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB, getTaskListNode(data.other_user_key, data.tasklist_key,
                                         task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "this is tasklist #0");
  EXPECT_EQ(out.visibility, "shared");

  // normal get with access, and read-only permission, should be successful
  out = TasklistContent();
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(false), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB, getTaskListNode(data.other_user_key, data.tasklist_key,
                                         task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "this is tasklist #0");
  EXPECT_EQ(out.visibility, "shared");

  // have no access
  out = TasklistContent();
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(ERR_ACCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_ACCESS);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.visibility, "");
}

TEST_F(TaskListTest, Create) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  in = TasklistContent("tasklist0", "this is tasklist #0", "private");

  std::map<std::string, std::string> task_list_info;
  task_list_info["name"] = in.name;
  task_list_info["content"] = in.content;
  task_list_info["visibility"] = in.visibility;
  std::string outName;

  // normal create, should be successful
  EXPECT_CALL(*mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  // request tasklist_key is empty, should be successful since task name is
  // provided in tasklistContent
  data.user_key = "user0";
  data.tasklist_key = "";
  outName = "";
  EXPECT_CALL(*mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  // request user_key is empty
  data.user_key = "";
  data.tasklist_key = "";
  outName = "";
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_RFIELD);
  EXPECT_EQ(outName, "");

  // loss tasklist name (key)
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  in.name = "";
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_KEY);
  EXPECT_EQ(outName, "");
  in.name = "tasklist0";

  // multiple calls to createTaskListNode if previous name is duplicated
  EXPECT_CALL(*mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_list_info["name"] = "tasklist0(1)";
  EXPECT_CALL(*mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_list_info["name"] = "tasklist0(2)";
  EXPECT_CALL(*mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0(2)");

  // if unknown error occurs
  in.name = "tasklist1";
  task_list_info["name"] = "tasklist1";
  EXPECT_CALL(*mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(ERR_UNKNOWN));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_UNKNOWN);
  EXPECT_EQ(outName, "");

  // visibility incorrect format
  in = TasklistContent("tasklist1", "this is tasklist #1", "wrong visibility");
  outName = "";
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_FORMAT);
  EXPECT_EQ(outName, "");
}

TEST_F(TaskListTest, Delete) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";

  // normal delete, should be successful
  EXPECT_CALL(*mockedDB, deleteTaskListNode(data.user_key, data.tasklist_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Delete(data), SUCCESS);

  // request tasklist_key is empty
  data.user_key = "user0";
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->Delete(data), ERR_RFIELD);

  // request user_key is empty
  data.user_key = "";
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->Delete(data), ERR_RFIELD);
}

TEST_F(TaskListTest, ReviseOwned) {
  // setup input
  data = RequestData("user0", "tasklist0", "", "");
  in = TasklistContent("", "this is tasklist #1", "");

  std::map<std::string, std::string> task_list_info;
  task_list_info["content"] = in.content;

  // normal revise, should be successful
  EXPECT_CALL(*mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                            task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), SUCCESS);

  // revise name, should be failed
  in.name = "tasklist_new_name";
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_KEY);
  in.name = "";

  // visibility incorrect format
  in.visibility = "wrongVis";
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_FORMAT);
  in.visibility = "";

  // request tasklist_key empty
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_RFIELD);

  // request user_key empty
  data.user_key = "";
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_RFIELD);

  // fields are empty
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  in = TasklistContent();
  task_list_info.clear();
  EXPECT_CALL(*mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                            task_list_info))
      .WillOnce(Return(ERR_RFIELD));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_RFIELD);

  // tasklist not exist
  data.user_key = "user0";
  data.tasklist_key = "not_exist_tasklist";
  EXPECT_CALL(*mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                            task_list_info))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_NO_NODE);
}

TEST_F(TaskListTest, ReviseAccess) {
  // setup input
  data = RequestData("user0", "tasklist0", "", "anotherUser0");
  in = TasklistContent("", "this is tasklist #1", "");

  std::map<std::string, std::string> task_list_info;
  task_list_info["content"] = in.content;

  bool permission = false;

  // normal revise with access, read-write permission, and does not try to
  // revise visibility, should be successful
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB, reviseTaskListNode(data.other_user_key,
                                            data.tasklist_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), SUCCESS);

  // revise with access, read-write permission, and try to revise visibility,
  // should not be successful
  in = TasklistContent("", "this is tasklist #1", "public");
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_REVISE);

  // revise with access but read-only permission, should be unsuccessful
  in = TasklistContent("", "this is tasklist #1", "");
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(false), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_ACCESS);
}

TEST_F(TaskListTest, GetAllTasklist) {
  // setup input
  data.user_key = "user0";
  std::vector<std::string> outNames;
  std::vector<std::string> newOutNames = {"tasklist0", "tasklist1", "tasklist2",
                                          "tasklist3"};

  // normal getAllTasklist, should be successful
  EXPECT_CALL(*mockedDB, getAllTaskListNodes(data.user_key, outNames))
      .WillOnce(DoAll(SetArgReferee<1>(newOutNames), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllTasklist(data, outNames), SUCCESS);
  EXPECT_EQ(outNames.size(), 4);
  EXPECT_EQ(outNames[3], "tasklist3");

  // request user_key empty
  data.user_key = "";
  EXPECT_EQ(tasklistsWorker->GetAllTasklist(data, outNames), ERR_RFIELD);
}

TEST_F(TaskListTest, GetAllAccessTaskList) {
  // setup input
  data.user_key = "user";
  std::vector<shareInfo> outList;
  std::map<std::pair<std::string, std::string>, bool> list_accesses;
  std::map<std::pair<std::string, std::string>, bool> new_list_accesses;
  for (int i = 0; i < 20; i++) {
    new_list_accesses[std::make_pair("user" + std::to_string(i),
                                     "tasklist" + std::to_string(i))] = false;
  }

  // normal getAllAccessTaskList call, should be successful
  EXPECT_CALL(*mockedDB, allAccess(data.user_key, list_accesses))
      .WillOnce(DoAll(SetArgReferee<1>(new_list_accesses), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllAccessTaskList(data, outList), SUCCESS);
  EXPECT_EQ(outList.size(), 20);

  int ind = 0;
  for (auto &it : new_list_accesses) {
    EXPECT_EQ(outList[ind].user_name, it.first.first);
    EXPECT_EQ(outList[ind].task_list_name, it.first.second);
    EXPECT_EQ(outList[ind].permission, false);
    ind++;
  }

  // request user_key empty
  data.user_key = "";
  outList.clear();
  EXPECT_EQ(tasklistsWorker->GetAllAccessTaskList(data, outList), ERR_RFIELD);
  EXPECT_EQ(outList.size(), 0);
}

TEST_F(TaskListTest, GetVisibility) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  std::string vis;
  std::map<std::string, std::string> task_list_info;
  std::map<std::string, std::string> new_task_list_info;
  task_list_info["visibility"] = "";

  // get visibility = "public"
  new_task_list_info["visibility"] = "public";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "public");

  // get visibility = "private"
  new_task_list_info["visibility"] = "private";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "private");

  // get visibility = "shared"
  new_task_list_info["visibility"] = "shared";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetVisibility(data, vis), SUCCESS);
  EXPECT_EQ(vis, "shared");

  // request user_key is empty
  data.user_key = "";
  data.tasklist_key = "";
  vis = "";
  EXPECT_EQ(tasklistsWorker->GetVisibility(data, vis), ERR_RFIELD);
  EXPECT_EQ(vis, "");

  // request user_key is empty
  data.user_key = "user0";
  data.tasklist_key = "unknown_tasklist";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasklistsWorker->GetVisibility(data, vis), ERR_NO_NODE);
  EXPECT_EQ(vis, "");
}

TEST_F(TaskListTest, GetAllGrantTaskList) {
  // setup input
  data.user_key = "user";
  data.tasklist_key = "tasklist";
  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"];
  std::map<std::string, std::string> new_task_list_info;
  new_task_list_info["visibility"] = "shared";
  std::map<std::string, bool> list_grants;
  std::map<std::string, bool> new_list_grants;
  for (int i = 0; i < 20; i++) {
    new_list_grants["user" + std::to_string(i)] = false;
  }
  std::vector<shareInfo> outList;
  bool isPublic = false;

  // normal GetAllGrantTaskList call, should be successful
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB,
              allGrant(data.user_key, data.tasklist_key, list_grants))
      .WillOnce(DoAll(SetArgReferee<2>(new_list_grants), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            SUCCESS);
  EXPECT_EQ(outList.size(), 20);
  int ind = 0;
  for (auto &it : new_list_grants) {
    EXPECT_EQ(outList[ind].user_name, it.first);
    EXPECT_EQ(outList[ind].task_list_name, "");
    EXPECT_EQ(outList[ind].permission, false);
    ind++;
  }
  EXPECT_EQ(isPublic, false);
  new_list_grants.clear();

  // call with public tasklist
  outList.clear();
  new_task_list_info["visibility"] = "public";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            SUCCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, true);

  // call with private tasklist
  outList.clear();
  new_task_list_info["visibility"] = "private";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_ACCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);

  // call when getTasklistNode failed
  outList.clear();
  new_task_list_info["visibility"] = "shared";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_NO_NODE);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);

  // reqeust has empty value
  outList.clear();
  data.user_key = "";
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_RFIELD);
  EXPECT_EQ(outList.size(), 0);
}

TEST_F(TaskListTest, ReviseGrantTaskList) {
  // setup input
  data.user_key = "user";
  data.tasklist_key = "tasklist";
  std::vector<shareInfo> in_list;
  for (int i = 0; i < 20; i++) {
    shareInfo info;
    info.user_name = "user" + std::to_string(i);
    info.permission = false;
    in_list.push_back(info);
  }
  std::string errUser;

  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"];
  std::map<std::string, std::string> new_task_list_info;
  new_task_list_info["visibility"] = "shared";

  // normal call, should be successful
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  for (int i = 0; i < in_list.size(); i++) {
    EXPECT_CALL(*mockedDB, addAccess(data.user_key, in_list[i].user_name,
                                     data.tasklist_key, in_list[i].permission))
        .WillOnce(Return(SUCCESS));
  }
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            SUCCESS);
  EXPECT_EQ(errUser, "");

  // failed on third user (no such user)
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  for (int i = 0; i < 3; i++) {
    EXPECT_CALL(*mockedDB, addAccess(data.user_key, in_list[i].user_name,
                                     data.tasklist_key, in_list[i].permission))
        .WillOnce(Return(SUCCESS));
  }

  EXPECT_CALL(*mockedDB, addAccess(data.user_key, in_list[3].user_name,
                                   data.tasklist_key, in_list[3].permission))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_NO_NODE);
  EXPECT_EQ(errUser, in_list[3].user_name);
  errUser = "";

  // call with empty tasklist key
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_RFIELD);
  EXPECT_EQ(errUser, "");
  data.tasklist_key = "tasklist";

  // failed, because tasklist is not "share"
  new_task_list_info["visibility"] = "public";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_ACCESS);
  EXPECT_EQ(errUser, "");
  new_task_list_info["visibility"] = "shared";

  // failed, because (one of the in_list) 's user_name is empty
  in_list[3].user_name = "";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  for (int i = 0; i < 3; i++) {
    EXPECT_CALL(*mockedDB, addAccess(data.user_key, in_list[i].user_name,
                                     data.tasklist_key, in_list[i].permission))
        .WillOnce(Return(SUCCESS));
  }
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_RFIELD);
}

TEST_F(TaskListTest, RemoveGrantTaskList) {
  // setup input
  data.user_key = "user";
  data.tasklist_key = "tasklist";
  data.other_user_key = "other_user";

  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"];
  std::map<std::string, std::string> new_task_list_info;
  new_task_list_info["visibility"] = "shared";

  // normal call, should be successful
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_CALL(*mockedUsers, DuplicatedEmail(UserInfo(data.other_user_key)))
      .WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, removeAccess(data.user_key, data.other_user_key,
                                      data.tasklist_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), SUCCESS);

  // no tasklist key
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), ERR_RFIELD);
  data.tasklist_key = "tasklist";

  // cannot remove access to a tasklist that is "public"
  new_task_list_info["visibility"] = "public";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), ERR_ACCESS);

  // cannot remove access to a tasklist that is "private"
  new_task_list_info["visibility"] = "private";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), ERR_ACCESS);

  // the other user does not exist
  new_task_list_info["visibility"] = "shared";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_CALL(*mockedUsers, DuplicatedEmail(UserInfo(data.other_user_key)))
      .WillOnce(Return(false));
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), ERR_NO_NODE);
}

TEST_F(TaskListTest, GetAllPublicTaskList) {
  // setup input
  std::vector<std::pair<std::string, std::string>> out_list;
  std::vector<std::pair<std::string, std::string>> new_out_list;
  for (int i = 0; i < 20; i++) {
    new_out_list.push_back(std::make_pair("user" + std::to_string(i),
                                          "tasklist" + std::to_string(i)));
  }

  // normal call, should be successful
  EXPECT_CALL(*mockedDB, getAllPublic(out_list))
      .WillOnce(DoAll(SetArgReferee<0>(new_out_list), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllPublicTaskList(out_list), SUCCESS);
  EXPECT_EQ(out_list, new_out_list);
}

TEST_F(TaskListTest, Exists) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  out = TasklistContent();
  std::map<std::string, std::string> task_list_info;
  std::map<std::string, std::string> new_task_list_info = {
      {"name", "tasklist0"},
      {"content", "this is tasklist #0"},
      {"visibility", "private"}};

  // normal get, should be successful
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_TRUE(tasklistsWorker->Exists(data));

  // no tasklist key
  data.tasklist_key = "unknown_tasklist";
  EXPECT_CALL(*mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_FALSE(tasklistsWorker->Exists(data));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
