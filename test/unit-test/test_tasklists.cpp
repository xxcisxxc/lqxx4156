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

class TaskListTest : public ::testing::Test {
protected:
  void SetUp() override {
    tasklistsWorker = std::make_shared<TaskListsWorker>(mockedDB);
  }

  void TearDown() override {
    // pass
  }

  MockedDB mockedDB;
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
      {"date", "10/15/2022"}};

  // normal get, should be successful
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "this is tasklist #0");
  EXPECT_EQ(out.date, "10/15/2022");

  // request tasklist_key is empty
  data.user_key = "user0";
  data.tasklist_key = "";
  out = TasklistContent();
  EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.date, "");

  // request user_key is empty
  data.user_key = "";
  data.tasklist_key = "";
  out = TasklistContent();
  EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.date, "");
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
      {"date", "10/15/2022"},
      {"visibility", "shared"}};
  bool permission = false;

  // normal get with access, and read-write permission, should be successful
  out = TasklistContent();
  EXPECT_CALL(mockedDB, checkAccess(data.other_user_key, data.user_key,
                                    data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(mockedDB, getTaskListNode(data.other_user_key, data.tasklist_key,
                                        task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "this is tasklist #0");
  EXPECT_EQ(out.date, "10/15/2022");
  EXPECT_EQ(out.visibility, "shared");

  // normal get with access, and read-only permission, should be successful
  out = TasklistContent();
  EXPECT_CALL(mockedDB, checkAccess(data.other_user_key, data.user_key,
                                    data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(false), Return(SUCCESS)));
  EXPECT_CALL(mockedDB, getTaskListNode(data.other_user_key, data.tasklist_key,
                                        task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "tasklist0");
  EXPECT_EQ(out.content, "this is tasklist #0");
  EXPECT_EQ(out.date, "10/15/2022");
  EXPECT_EQ(out.visibility, "shared");

  // have no access
  out = TasklistContent();
  EXPECT_CALL(mockedDB, checkAccess(data.other_user_key, data.user_key,
                                    data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(ERR_ACCESS)));
  EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_ACCESS);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.date, "");
  EXPECT_EQ(out.visibility, "");
}

TEST_F(TaskListTest, Create) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  std::string name = "tasklist0";
  std::string content = "this is tasklist #0";
  std::string date = "10/15/2022";
  std::string vis = "private";
  in = TasklistContent(name, content, date, vis);

  std::map<std::string, std::string> task_list_info;
  task_list_info["name"] = name;
  task_list_info["content"] = content;
  task_list_info["date"] = date;
  task_list_info["visibility"] = vis;
  std::string outName;

  // normal create, should be successful
  EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  // request tasklist_key is empty, should be successful since task name is
  // provided in tasklistContent
  data.user_key = "user0";
  data.tasklist_key = "";
  outName = "";
  EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0");

  // request user_key is empty
  data.user_key = "";
  data.tasklist_key = "";
  outName = "";
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_RFIELD);
  EXPECT_EQ(outName, "");

  // multiple calls to createTaskListNode if previous name is duplicated
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  outName = "";
  EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_list_info["name"] = "tasklist0(1)";
  EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_list_info["name"] = "tasklist0(2)";
  EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
  EXPECT_EQ(outName, "tasklist0(2)");

  // visibility incorrect format
  std::string wrongVis = "wrong";
  in = TasklistContent(name, content, date, wrongVis);
  outName = "";
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_FORMAT);
  EXPECT_EQ(outName, "");

  // date incorrect format
  std::string wrongDate = "02/29/2022";
  in = TasklistContent(name, content, wrongDate, vis);
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_FORMAT);
  EXPECT_EQ(outName, "");

  std::string wrongDate2 = "13/20/2022";
  in = TasklistContent(name, content, wrongDate, vis);
  EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_FORMAT);
  EXPECT_EQ(outName, "");
}

TEST_F(TaskListTest, Delete) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";

  // normal delete, should be successful
  EXPECT_CALL(mockedDB, deleteTaskListNode(data.user_key, data.tasklist_key))
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
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";

  std::string newName = "tasklist1";
  std::string newContent = "this is tasklist #1";
  std::string newDate = "12/10/2022";
  std::string newVis = "private";
  std::string blankName = "";
  in = TasklistContent(blankName, newContent, newDate, newVis);

  std::map<std::string, std::string> task_list_info;
  task_list_info["content"] = newContent;
  task_list_info["date"] = newDate;
  task_list_info["visibility"] = newVis;

  // normal revise, should be successful
  EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                           task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), SUCCESS);

  // revise name, should be failed
  in.name = newName;
  task_list_info["name"] = newName;
  EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                           task_list_info))
      .WillOnce(Return(ERR_KEY));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_KEY);
  in.name = "";
  task_list_info["name"] = "";

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
  EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                           task_list_info))
      .WillOnce(Return(ERR_RFIELD));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_RFIELD);

  // tasklist not exist
  data.user_key = "user0";
  data.tasklist_key = "not_exist_tasklist";
  EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key,
                                           task_list_info))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_NO_NODE);

  // visibility incorrect format
  std::string wrongVis = "wrong";
  in = TasklistContent(newName, newContent, newDate, wrongVis);
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_FORMAT);

  // date incorrect format
  std::string wrongDate = "02/29/2022";
  in = TasklistContent(newName, newContent, wrongDate, newVis);
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_FORMAT);

  std::string wrongDate2 = "13/20/2022";
  in = TasklistContent(newName, newContent, wrongDate, newVis);
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_FORMAT);
}

TEST_F(TaskListTest, ReviseAccess) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  data.other_user_key = "anotherUser0";

  std::string newName = "tasklist1";
  std::string newContent = "this is tasklist #1";
  std::string newDate = "12/10/2022";
  std::string newVis = "";
  std::string blankName = "";
  in = TasklistContent(blankName, newContent, newDate, newVis);

  std::map<std::string, std::string> task_list_info;
  task_list_info["content"] = newContent;
  task_list_info["date"] = newDate;

  bool permission = false;

  // normal revise with access, read-write permission, and does not try to
  // revise visibility, should be successful
  EXPECT_CALL(mockedDB, checkAccess(data.other_user_key, data.user_key,
                                    data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(mockedDB, reviseTaskListNode(data.other_user_key,
                                           data.tasklist_key, task_list_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), SUCCESS);

  // revise with access, read-write permission, and try to revise visibility,
  // should not be successful
  newVis = "public";
  in = TasklistContent(newName, newContent, newDate, newVis);
  EXPECT_CALL(mockedDB, checkAccess(data.other_user_key, data.user_key,
                                    data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_REVISE);
  EXPECT_EQ(task_list_info.count("visibility"), 0);

  // revise with access and read-only permission, should be unsuccessful
  newVis = "";
  in = TasklistContent(newName, newContent, newDate, newVis);
  EXPECT_CALL(mockedDB, checkAccess(data.other_user_key, data.user_key,
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
  EXPECT_CALL(mockedDB, getAllTaskListNodes(data.user_key, outNames))
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
  EXPECT_CALL(mockedDB, allAccess(data.user_key, list_accesses))
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
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_CALL(mockedDB, allGrant(data.user_key, data.tasklist_key, list_grants))
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

  // call with public tasklist
  outList.clear();
  new_task_list_info["visibility"] = "public";
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            SUCCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, true);

  // call with private tasklist
  outList.clear();
  new_task_list_info["visibility"] = "private";
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_ACCESS);
  EXPECT_EQ(outList.size(), 0);
  EXPECT_EQ(isPublic, false);

  // call when getTasklistNode failed
  outList.clear();
  new_task_list_info["visibility"] = "shared";
  new_list_grants.clear();
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_CALL(mockedDB, allGrant(data.user_key, data.tasklist_key, list_grants))
      .WillOnce(DoAll(SetArgReferee<2>(new_list_grants), Return(ERR_ACCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllGrantTaskList(data, outList, isPublic),
            ERR_ACCESS);
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
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  for (int i = 0; i < in_list.size(); i++) {
    EXPECT_CALL(mockedDB, addAccess(data.user_key, in_list[i].user_name,
                                    data.tasklist_key, in_list[i].permission))
        .WillOnce(Return(SUCCESS));
  }
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            SUCCESS);
  EXPECT_EQ(errUser, "");

  // failed on third user
  task_list_info["visibility"] = "";
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  for (int i = 0; i < 3; i++) {
    EXPECT_CALL(mockedDB, addAccess(data.user_key, in_list[i].user_name,
                                    data.tasklist_key, in_list[i].permission))
        .WillOnce(Return(SUCCESS));
  }
  // errors are all similar logic
  EXPECT_CALL(mockedDB, addAccess(data.user_key, in_list[3].user_name,
                                  data.tasklist_key, in_list[3].permission))
      .WillOnce(Return(ERR_ACCESS));
  EXPECT_EQ(tasklistsWorker->ReviseGrantTaskList(data, in_list, errUser),
            ERR_ACCESS);
  EXPECT_EQ(errUser, in_list[3].user_name);
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
  EXPECT_CALL(mockedDB,
              getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info), Return(SUCCESS)));
  EXPECT_CALL(mockedDB, removeAccess(data.user_key, data.other_user_key,
                                     data.tasklist_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), SUCCESS);

  // no tasklist key
  data.tasklist_key = "";
  EXPECT_EQ(tasklistsWorker->RemoveGrantTaskList(data), ERR_RFIELD);
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
  EXPECT_CALL(mockedDB, getAllPublic(out_list))
      .WillOnce(DoAll(SetArgReferee<0>(new_out_list), Return(SUCCESS)));
  EXPECT_EQ(tasklistsWorker->GetAllPublicTaskList(out_list), SUCCESS);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
