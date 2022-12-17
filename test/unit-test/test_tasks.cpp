#include <db/DB.h>
#include <exception>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <tasklists/tasklistsWorker.h>
#include <tasks/tasksWorker.h>
#include <users/users.h>

class MockedDB : public DB {
public:
  MOCK_METHOD(returnCode, getTaskNode,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               const std::string &task_pkey,
               (std::map<std::string, std::string>)&task_info),
              (override));
  MOCK_METHOD(returnCode, createTaskNode,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               (const std::map<std::string, std::string>)&task_info),
              (override));
  MOCK_METHOD(returnCode, deleteTaskNode,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               const std::string &task_pkey),
              (override));
  MOCK_METHOD(returnCode, reviseTaskNode,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               const std::string &task_pkey,
               (const std::map<std::string, std::string>)&task_info),
              (override));
  MOCK_METHOD(returnCode, getAllTaskNodes,
              (const std::string &user_pkey, const std::string &task_list_pkey,
               std::vector<std::string> &task_info),
              (override));
  MOCK_METHOD(returnCode, checkAccess,
              (const std::string &src_user_pkey,
               const std::string &dst_user_pkey,
               const std::string &task_list_pkey, bool &read_write),
              (override));
  MockedDB() : DB("testhost") {}
};

class MockedTaskLists : public TaskListsWorker {
public:
  MOCK_METHOD(bool, Exists, (const RequestData &data), (override));

  MockedTaskLists(std::shared_ptr<DB> _db, std::shared_ptr<Users> _users)
      : TaskListsWorker(_db, _users) {}
};

class MockedUsers : public Users {
public:
  MockedUsers(std::shared_ptr<DB> _db) : Users(_db) {}
};

class TasksWorkerTest : public ::testing::Test {
protected:
  void SetUp() override {
    mockedDB = std::make_shared<MockedDB>();
    mockedUsers = std::make_shared<MockedUsers>(mockedDB);
    mockedTaskLists = std::make_shared<MockedTaskLists>(mockedDB, mockedUsers);
    tasksWorker = std::make_shared<TasksWorker>(mockedDB, mockedTaskLists);
  }

  void TearDown() override {}

  std::shared_ptr<MockedDB> mockedDB;
  std::shared_ptr<MockedUsers> mockedUsers;
  std::shared_ptr<MockedTaskLists> mockedTaskLists;
  std::shared_ptr<TasksWorker> tasksWorker;

  RequestData data;
  TaskContent in;
  TaskContent out;
};

using namespace ::testing;

// Query Function
TEST_F(TasksWorkerTest, Query) {
  // setup input
  data = RequestData("user0", "tasklist0", "task0", "");

  std::map<std::string, std::string> task_info;
  std::map<std::string, std::string> new_task_info;
  new_task_info["name"] = "task0";
  new_task_info["content"] = "4156 Iteration-2";
  new_task_info["startDate"] = "10/31/2022";
  new_task_info["endDate"] = "11/29/2022";
  new_task_info["priority"] = "1";
  new_task_info["status"] = "To Do";

  // should be successful
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, getTaskNode(data.user_key, data.tasklist_key,
                                     data.task_key, task_info))
      .WillOnce(DoAll(SetArgReferee<3>(new_task_info), Return(SUCCESS)));
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task0");
  EXPECT_EQ(out.content, "4156 Iteration-2");
  EXPECT_EQ(out.startDate, "10/31/2022");
  EXPECT_EQ(out.endDate, "11/29/2022");
  EXPECT_EQ(out.priority, VERY_URGENT);
  EXPECT_EQ(out.status, "To Do");

  // query others' tasks should be successful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(SUCCESS));
  EXPECT_CALL(*mockedDB, getTaskNode(data.other_user_key, data.tasklist_key,
                                     data.task_key, task_info))
      .WillOnce(DoAll(SetArgReferee<3>(new_task_info), Return(SUCCESS)));
  EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
  EXPECT_EQ(out.name, "task0");
  EXPECT_EQ(out.content, "4156 Iteration-2");
  EXPECT_EQ(out.startDate, "10/31/2022");
  EXPECT_EQ(out.endDate, "11/29/2022");
  EXPECT_EQ(out.priority, VERY_URGENT);
  EXPECT_EQ(out.status, "To Do");

  // query others' tasks failed
  out = TaskContent();
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  out = TaskContent();
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.tasklist_key = "tasklist0";

  data.task_key = "";
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_RFIELD);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.task_key = "task0";

  // key error
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, getTaskNode(data.user_key, data.tasklist_key,
                                     data.task_key, task_info))
      .WillOnce(Return(ERR_KEY));
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");

  // task or tasklist itself does not exist
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");

  // task itself does not exist
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, getTaskNode(data.user_key, data.tasklist_key,
                                     data.task_key, task_info))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_NO_NODE);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
}

TEST_F(TasksWorkerTest, Create) {
  // setup input
  data = RequestData("user0", "tasklist0", "", "");
  in = TaskContent("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                   VERY_URGENT, "To Do");

  std::map<std::string, std::string> task_info;
  task_info["name"] = in.name;
  task_info["content"] = in.content;
  task_info["startDate"] = in.startDate;
  task_info["endDate"] = in.endDate;
  task_info["priority"] = std::to_string(in.priority);
  task_info["status"] = in.status;

  std::string outTaskName;

  // should be successful
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0");
  outTaskName = "";

  // create others' tasks should be successful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.other_user_key, data.tasklist_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0");
  outTaskName = "";

  // create others' tasks failed (ERR_ACCESS)
  permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_ACCESS);
  EXPECT_EQ(outTaskName, "");
  outTaskName = "";

  // create others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_NO_NODE);
  EXPECT_EQ(outTaskName, "");
  outTaskName = "";
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_RFIELD);
  EXPECT_EQ(outTaskName, "");
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_RFIELD);
  EXPECT_EQ(outTaskName, "");
  data.tasklist_key = "tasklist0";
  // not use data.task_key

  // input does not have a key
  in.name = "";
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_KEY);
  EXPECT_EQ(outTaskName, "");
  in.name = "task0";

  // tasklist does not exist
  data.tasklist_key = "not_exist_tasklist";
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_NO_NODE);
  EXPECT_EQ(outTaskName, "");
  data.tasklist_key = "tasklist0";

  // multiple calls to createTaskNode if previous name is duplicated
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_info["name"] = "task0(1)";
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0(1)");
  outTaskName = "";

  // multiple calls again
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  task_info["name"] = "task0";
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_info["name"] = "task0(1)";
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(ERR_DUP_NODE));
  task_info["name"] = "task0(2)";
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(SUCCESS));

  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0(2)");
  outTaskName = "";

  // Error format for startDate
  in = TaskContent("task0", "4156 Iteration-2", "2018-01-01", "11/29/2022",
                   VERY_URGENT, "To Do");
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_FORMAT);
  EXPECT_EQ(outTaskName, "");

  // Error format for endDate
  in = TaskContent("task0", "4156 Iteration-2", "10/31/2022", "2018-01-02",
                   VERY_URGENT, "To Do");
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_FORMAT);
  EXPECT_EQ(outTaskName, "");

  // Error format for startDate > endDate
  in = TaskContent("task0", "4156 Iteration-2", "11/30/2022", "11/29/2022",
                   VERY_URGENT, "To Do");
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_FORMAT);
  EXPECT_EQ(outTaskName, "");

  // Error format for priority
  in = TaskContent("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                   (Priority)5, "To Do");
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_FORMAT);
  EXPECT_EQ(outTaskName, "");

  // Error format for status
  in = TaskContent("task0", "4156 Iteration-2", "10/31/2022", "11/29/2022",
                   VERY_URGENT, "2/3 Done");
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_FORMAT);
  EXPECT_EQ(outTaskName, "");
}

TEST_F(TasksWorkerTest, Delete) {
  // setup input
  data = RequestData("user0", "tasklist0", "task0", "");

  // should be successful
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              deleteTaskNode(data.user_key, data.tasklist_key, data.task_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // delete others' tasks should be successful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB, deleteTaskNode(data.other_user_key, data.tasklist_key,
                                        data.task_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // delete others' tasks failed (ERR_NO_NODE)
  permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Delete(data), ERR_NO_NODE);

  // delete others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Delete(data), ERR_ACCESS);
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_RFIELD);
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_RFIELD);
  data.tasklist_key = "tasklist0";

  data.task_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_RFIELD);
  data.task_key = "task0";

  // tasklist does not exist
  data.tasklist_key = "not_exist_tasklist";
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
  EXPECT_EQ(tasksWorker->Delete(data), ERR_NO_NODE);

  // task does not exist
  data.tasklist_key = "tasklist0";
  data.task_key = "not_exist_task";
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              deleteTaskNode(data.user_key, data.tasklist_key, data.task_key))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Delete(data), ERR_NO_NODE);
}

TEST_F(TasksWorkerTest, Revise) {
  // setup input
  data = RequestData("user0", "tasklist0", "task0", "");

  in = TaskContent();
  std::map<std::string, std::string> task_info;

  // should be successful
  in.content = "4156 Iteration-2";
  task_info["content"] = in.content;
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // should be successful
  in.startDate = "10/31/2022", in.endDate = "11/29/2022";
  ;
  task_info["startDate"] = in.startDate, task_info["endDate"] = in.endDate;
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // should be successful
  in.priority = VERY_URGENT;
  task_info["priority"] = std::to_string(in.priority);
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // should be successful
  in.status = "To Do";
  task_info["status"] = in.status;
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // revise others' tasks should be successful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.other_user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // revise others' tasks failed (ERR_NO_NODE)
  permission = false;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_NO_NODE);

  // revise others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_ACCESS);
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_RFIELD);
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_RFIELD);
  data.tasklist_key = "tasklist0";

  data.task_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_RFIELD);
  data.task_key = "task0";

  // tasklist does not exist
  data.tasklist_key = "not_exist_tasklist";
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_NO_NODE);
  data.tasklist_key = "tasklist0";

  // input is empty
  in = TaskContent();
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_RFIELD);

  // Error format for startDate
  in.startDate = "2018-01-01";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_FORMAT);
  in.startDate = "10/31/2022";

  // Error format for endDate
  in.endDate = "2018-01-02";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_FORMAT);
  in.endDate = "11/29/2022";

  // Error format for startDate > endDate
  in.startDate = "11/30/2022";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_FORMAT);
  in.startDate = "10/31/2022";

  // Error format for priority
  in.priority = (Priority)5;
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_FORMAT);
  in.priority = VERY_URGENT;

  // Error format for status
  in.status = "2/3 Done";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_FORMAT);
  in.status = "Done";
}

// Create Function
TEST_F(TasksWorkerTest, GetAllTasksName) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";

  std::vector<std::string> task_names;
  std::vector<std::string> new_task_names = {"task0", "task1", "task2"};

  // should be successful
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              getAllTaskNodes(data.user_key, data.tasklist_key, task_names))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_names), Return(SUCCESS)));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), SUCCESS);
  EXPECT_EQ(task_names[0], "task0");
  EXPECT_EQ(task_names[1], "task1");
  EXPECT_EQ(task_names[2], "task2");
  EXPECT_EQ(task_names.size(), 3);

  // get others' tasks should be successful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = false;
  task_names.clear();
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(DoAll(SetArgReferee<3>(true), Return(SUCCESS)));
  EXPECT_CALL(*mockedDB, getAllTaskNodes(data.other_user_key, data.tasklist_key,
                                         task_names))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_names), Return(SUCCESS)));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), SUCCESS);
  EXPECT_EQ(task_names[0], "task0");
  EXPECT_EQ(task_names[1], "task1");
  EXPECT_EQ(task_names[2], "task2");
  EXPECT_EQ(task_names.size(), 3);

  // get others' tasks failed (ERR_NO_NODE)
  permission = false;
  task_names.clear();
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_NO_NODE);
  EXPECT_EQ(task_names.size(), 0);
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // should be successful but return empty
  task_names.clear();
  new_task_names.clear();
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              getAllTaskNodes(data.user_key, data.tasklist_key, task_names))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_names), Return(SUCCESS)));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), SUCCESS);
  EXPECT_EQ(task_names.size(), 0);

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_RFIELD);
  EXPECT_EQ(task_names.size(), 0);
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_RFIELD);
  EXPECT_EQ(task_names.size(), 0);
  data.tasklist_key = "tasklist0";

  // tasklist does not exist
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_NO_NODE);
  EXPECT_EQ(task_names.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
