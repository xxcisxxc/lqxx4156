#include <db/DB.h>
#include <exception>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <tasklists/tasklistsWorker.h>
#include <tasks/tasksWorker.h>

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
              (const std::string &src_user_pkey, const std::string &dst_user_pkey,
               const std::string &task_list_pkey, bool &read_write),
              (override));
  MockedDB() : DB("testhost") {}
};

class MockedTaskLists : public TaskListsWorker {
public:
  MOCK_METHOD(bool, Exists, (const RequestData &data), (override));

  MockedTaskLists(DB &_db_instance) : TaskListsWorker(_db_instance) {}
};

class TasksWorkerTest : public ::testing::Test {
protected:
  void SetUp() override {
    mockedDB = new MockedDB();
    mockedTaskLists = new MockedTaskLists(*mockedDB);
    tasksWorker = std::make_shared<TasksWorker>(mockedDB, mockedTaskLists);
  }

  void TearDown() override {
    delete (mockedDB);
    delete (mockedTaskLists);
  }

  MockedDB *mockedDB;
  MockedTaskLists *mockedTaskLists;
  std::shared_ptr<TasksWorker> tasksWorker;

  RequestData data;
  TaskContent in;
  TaskContent out;
};

using namespace ::testing;

TEST_F(TasksWorkerTest, TaskStruct2Map) {
  TaskContent task;
  task.name = "test_name";
  task.content = "test_content";
  task.startDate = "test_startDate";
  task.endDate = "test_endDate";
  task.priority = VERY_URGENT;
  task.status = "test_status";
  std::map<std::string, std::string> mp;

  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_EQ(mp.at("name"), "test_name");
  EXPECT_EQ(mp.at("content"), "test_content");
  EXPECT_EQ(mp.at("startDate"), "test_startDate");
  EXPECT_EQ(mp.at("endDate"), "test_endDate");
  EXPECT_EQ(mp.at("priority"), "1");
  EXPECT_EQ(mp.at("status"), "test_status");

  task.name = "";
  mp.clear();
  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_THROW(mp.at("name"), std::exception);
  EXPECT_EQ(mp.at("content"), "test_content");
  EXPECT_EQ(mp.at("startDate"), "test_startDate");
  EXPECT_EQ(mp.at("endDate"), "test_endDate");
  EXPECT_EQ(mp.at("priority"), "1");
  EXPECT_EQ(mp.at("status"), "test_status");

  task.content = "";
  mp.clear();
  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_THROW(mp.at("name"), std::exception);
  EXPECT_THROW(mp.at("content"), std::exception);
  EXPECT_EQ(mp.at("startDate"), "test_startDate");
  EXPECT_EQ(mp.at("endDate"), "test_endDate");
  EXPECT_EQ(mp.at("priority"), "1");
  EXPECT_EQ(mp.at("status"), "test_status");

  task.startDate = "";
  mp.clear();
  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_THROW(mp.at("name"), std::exception);
  EXPECT_THROW(mp.at("content"), std::exception);
  EXPECT_THROW(mp.at("startDate"), std::exception);
  EXPECT_EQ(mp.at("endDate"), "test_endDate");
  EXPECT_EQ(mp.at("priority"), "1");
  EXPECT_EQ(mp.at("status"), "test_status");

  task.endDate = "";
  mp.clear();
  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_THROW(mp.at("name"), std::exception);
  EXPECT_THROW(mp.at("content"), std::exception);
  EXPECT_THROW(mp.at("startDate"), std::exception);
  EXPECT_THROW(mp.at("endDate"), std::exception);
  EXPECT_EQ(mp.at("priority"), "1");
  EXPECT_EQ(mp.at("status"), "test_status");

  task.priority = NULL_PRIORITY;
  mp.clear();
  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_THROW(mp.at("name"), std::exception);
  EXPECT_THROW(mp.at("content"), std::exception);
  EXPECT_THROW(mp.at("startDate"), std::exception);
  EXPECT_THROW(mp.at("endDate"), std::exception);
  EXPECT_THROW(mp.at("priority"), std::exception);
  EXPECT_EQ(mp.at("status"), "test_status");

  task.status = "";
  mp.clear();
  tasksWorker->TaskStruct2Map(task, mp);
  EXPECT_THROW(mp.at("name"), std::exception);
  EXPECT_THROW(mp.at("content"), std::exception);
  EXPECT_THROW(mp.at("startDate"), std::exception);
  EXPECT_THROW(mp.at("endDate"), std::exception);
  EXPECT_THROW(mp.at("priority"), std::exception);
  EXPECT_THROW(mp.at("status"), std::exception);
}

TEST_F(TasksWorkerTest, Map2TaskStruct) {
  std::map<std::string, std::string> mp;
  mp["name"] = "test_name";
  mp["content"] = "test_content";
  mp["startDate"] = "test_startDate";
  mp["endDate"] = "test_endDate";
  mp["priority"] = "1";
  mp["status"] = "test_status";
  TaskContent task;

  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "test_name");
  EXPECT_EQ(task.content, "test_content");
  EXPECT_EQ(task.startDate, "test_startDate");
  EXPECT_EQ(task.endDate, "test_endDate");
  EXPECT_EQ(task.priority, VERY_URGENT);
  EXPECT_EQ(task.status, "test_status");

  mp["name"] = "";
  task.Clear();
  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "");
  EXPECT_EQ(task.content, "test_content");
  EXPECT_EQ(task.startDate, "test_startDate");
  EXPECT_EQ(task.endDate, "test_endDate");
  EXPECT_EQ(task.priority, VERY_URGENT);
  EXPECT_EQ(task.status, "test_status");

  mp["content"] = "";
  task.Clear();
  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "");
  EXPECT_EQ(task.content, "");
  EXPECT_EQ(task.startDate, "test_startDate");
  EXPECT_EQ(task.endDate, "test_endDate");
  EXPECT_EQ(task.priority, VERY_URGENT);
  EXPECT_EQ(task.status, "test_status");

  mp["startDate"] = "";
  task.Clear();
  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "");
  EXPECT_EQ(task.content, "");
  EXPECT_EQ(task.startDate, "");
  EXPECT_EQ(task.endDate, "test_endDate");
  EXPECT_EQ(task.priority, VERY_URGENT);
  EXPECT_EQ(task.status, "test_status");

  mp["endDate"] = "";
  task.Clear();
  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "");
  EXPECT_EQ(task.content, "");
  EXPECT_EQ(task.startDate, "");
  EXPECT_EQ(task.endDate, "");
  EXPECT_EQ(task.priority, VERY_URGENT);
  EXPECT_EQ(task.status, "test_status");

  mp["priority"] = "0";
  task.Clear();
  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "");
  EXPECT_EQ(task.content, "");
  EXPECT_EQ(task.startDate, "");
  EXPECT_EQ(task.endDate, "");
  EXPECT_EQ(task.priority, NULL_PRIORITY);
  EXPECT_EQ(task.status, "test_status");

  mp["status"] = "";
  task.Clear();
  tasksWorker->Map2TaskStruct(mp, task);
  EXPECT_EQ(task.name, "");
  EXPECT_EQ(task.content, "");
  EXPECT_EQ(task.startDate, "");
  EXPECT_EQ(task.endDate, "");
  EXPECT_EQ(task.priority, NULL_PRIORITY);
  EXPECT_EQ(task.status, "");

  // no error is just ok
  mp.erase("status");
  tasksWorker->Map2TaskStruct(mp, task);

  mp.erase("priority");
  tasksWorker->Map2TaskStruct(mp, task);

  mp.erase("endDate");
  tasksWorker->Map2TaskStruct(mp, task);

  mp.erase("startDate");
  tasksWorker->Map2TaskStruct(mp, task);

  mp.erase("content");
  tasksWorker->Map2TaskStruct(mp, task);

  mp.erase("name");
  tasksWorker->Map2TaskStruct(mp, task);
}

// Query Function
TEST_F(TasksWorkerTest, Query) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";
  std::map<std::string, std::string> task_info;
  std::map<std::string, std::string> new_task_info;
  new_task_info["name"] = "task0";
  new_task_info["content"] = "4156 Iteration-2";
  new_task_info["startDate"] = "10/31/2022";
  new_task_info["endDate"] = "11/29/2022";
  new_task_info["priority"] = "1";
  new_task_info["status"] = "To Do";

  // should be successful
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
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(SUCCESS));
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
  
  // query others' tasks failed
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
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
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.tasklist_key = "tasklist0";

  data.task_key = "";
  EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
  EXPECT_EQ(out.name, "");
  EXPECT_EQ(out.content, "");
  EXPECT_EQ(out.startDate, "");
  EXPECT_EQ(out.endDate, "");
  EXPECT_EQ(out.priority, NULL_PRIORITY);
  EXPECT_EQ(out.status, "");
  data.task_key = "task0";

  // key error
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
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";

  std::string name = "task0";
  std::string content = "4156 Iteration-2";
  std::string startDate = "10/31/2022";
  std::string endDate = "11/29/2022";
  Priority priority = VERY_URGENT;
  std::string status = "To Do";
  in = TaskContent(name, content, startDate, endDate, priority, status);

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

  // create others' tasks should be sucdessful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = true;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(DoAll(SetArgReferee<3>(permission), Return(SUCCESS)));
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              createTaskNode(data.user_key, data.tasklist_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
  EXPECT_EQ(outTaskName, "task0");
  outTaskName = "";

  // create others' tasks failed (ERR_NO_NODE)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_ACCESS);
  EXPECT_EQ(outTaskName, "");
  outTaskName = "";

  // create others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_NO_NODE);
  EXPECT_EQ(outTaskName, "");
  outTaskName = "";
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_KEY);
  EXPECT_EQ(outTaskName, "");
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), ERR_KEY);
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
}

TEST_F(TasksWorkerTest, Delete) {
  // setup input
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";

  // should be successful
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              deleteTaskNode(data.user_key, data.tasklist_key, data.task_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // delete others' tasks should be successful
  data.other_user_key = "user1";
  data.tasklist_key = "tasklist1";
  bool permission = true;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(DoAll(SetArgReferee<3>(permission), Return(SUCCESS)));
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              deleteTaskNode(data.user_key, data.tasklist_key, data.task_key))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

  // delete others' tasks failed (ERR_NO_NODE)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Delete(data), ERR_NO_NODE);

  // delete others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Delete(data), ERR_ACCESS);
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_KEY);
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_KEY);
  data.tasklist_key = "tasklist0";

  data.task_key = "";
  EXPECT_EQ(tasksWorker->Delete(data), ERR_KEY);
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
  data.user_key = "user0";
  data.tasklist_key = "tasklist0";
  data.task_key = "task0";

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
  in.startDate = "10/31/2022";
  task_info["startDate"] = in.startDate;
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // should be successful
  in.endDate = "11/29/2022";
  task_info["endDate"] = in.endDate;
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
  bool permission = true;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(DoAll(SetArgReferee<3>(permission), Return(SUCCESS)));
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, data.tasklist_key,
                                        data.task_key, task_info))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

  // revise others' tasks failed (ERR_NO_NODE)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_NO_NODE);

  // revise others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_ACCESS);
  data.other_user_key = "";
  data.tasklist_key = "tasklist0";

  // request is empty
  data.user_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_KEY);
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_KEY);
  data.tasklist_key = "tasklist0";

  data.task_key = "";
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_KEY);
  data.task_key = "task0";

  // tasklist does not exist
  data.tasklist_key = "not_exist_tasklist";
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_NO_NODE);
  data.tasklist_key = "tasklist0";

  // input is empty
  in = TaskContent();
  EXPECT_EQ(tasksWorker->Revise(data, in), ERR_KEY);
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
  bool permission = true;
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(DoAll(SetArgReferee<3>(permission), Return(SUCCESS)));
  EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
  EXPECT_CALL(*mockedDB,
              getAllTaskNodes(data.user_key, data.tasklist_key, task_names))
      .WillOnce(DoAll(SetArgReferee<2>(new_task_names), Return(SUCCESS)));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), SUCCESS);
  EXPECT_EQ(task_names[0], "task0");
  EXPECT_EQ(task_names[1], "task1");
  EXPECT_EQ(task_names[2], "task2");
  EXPECT_EQ(task_names.size(), 3);

  // get others' tasks failed (ERR_NO_NODE)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(ERR_NO_NODE));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_NO_NODE);
  EXPECT_EQ(task_names.size(), 0);

  // get others' tasks failed (permission denied)
  EXPECT_CALL(*mockedDB, checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, false))
      .WillOnce(Return(SUCCESS));
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_ACCESS);
  EXPECT_EQ(task_names.size(), 0);

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
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_KEY);
  EXPECT_EQ(task_names.size(), 0);
  data.user_key = "user0";

  data.tasklist_key = "";
  EXPECT_EQ(tasksWorker->GetAllTasksName(data, task_names), ERR_KEY);
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
