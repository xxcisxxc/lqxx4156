#include <tasks/tasksWorker.h>
#include <tasklists/tasklistsWorker.h>
#include <db/DB.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

class MockedDB : public DB {
public:
    MOCK_METHOD(returnCode,
                getTaskNode,
                (const std::string &user_pkey, 
                 const std::string &task_list_pkey,
                 const std::string &task_pkey,
                 (std::map<std::string, std::string>) &task_info),
                (override));
    MOCK_METHOD(returnCode, 
                createTaskNode, 
                (const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 (const std::map<std::string, std::string>) &task_info), 
                (override));
    MOCK_METHOD(returnCode,
                deleteTaskNode,
                (const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 const std::string &task_pkey),
                (override));
    MOCK_METHOD(returnCode,
                reviseTaskNode,
                (const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 const std::string &task_pkey,
                 (const std::map<std::string, std::string>) &task_info),
                (override));
    
    MockedDB() : DB("testhost") {}
};

class MockedTaskLists : public TaskListsWorker {
public:
    MOCK_METHOD(returnCode,
                Query,
                (const RequestData& data, 
                TasklistContent& out),
                (override));
    MOCK_METHOD(returnCode,
                Create,
                (const RequestData& data,
                TasklistContent& in,
                std::string& outName),
                (override));
    MOCK_METHOD(returnCode,
                Delete,
                (const RequestData& data),
                (override));
    MOCK_METHOD(returnCode,
                Revise,
                (const RequestData& data,
                TasklistContent& in),
                (override));
    MOCK_METHOD(bool,
                Exists,
                (const RequestData& data),
                (override));

    MockedTaskLists(DB& _db_instance) : TaskListsWorker(_db_instance) {}
};

class TasksWorkerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockedDB = new MockedDB();
        mockedTaskLists = new MockedTaskLists(*mockedDB);
        tasksWorker = std::make_shared<TasksWorker>(mockedDB, mockedTaskLists);
    }

    void TearDown() override {
        delete(mockedDB);
        delete(mockedTaskLists);
    }

    MockedDB* mockedDB;
    MockedTaskLists* mockedTaskLists;
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
    task.date = "test_date";
    std::map<std::string, std::string> mp;

    tasksWorker->TaskStruct2Map(task, mp);
    EXPECT_EQ(mp.at("name"), "test_name");
    EXPECT_EQ(mp.at("content"), "test_content");
    EXPECT_EQ(mp.at("date"), "test_date");   

    task.name = "";
    mp.clear();
    tasksWorker->TaskStruct2Map(task, mp);
    EXPECT_EQ(mp.at("name"), "");
    EXPECT_EQ(mp.at("content"), "test_content");
    EXPECT_EQ(mp.at("date"), "test_date");   

    task.content = "";
    mp.clear();
    tasksWorker->TaskStruct2Map(task, mp);
    EXPECT_EQ(mp.at("name"), "");
    EXPECT_EQ(mp.at("content"), "");
    EXPECT_EQ(mp.at("date"), "test_date");   

    task.date = "";
    mp.clear();
    tasksWorker->TaskStruct2Map(task, mp);
    EXPECT_EQ(mp.at("name"), "");
    EXPECT_EQ(mp.at("content"), "");
    EXPECT_EQ(mp.at("date"), "");   
}

TEST_F(TasksWorkerTest, Map2TaskStruct) {
    std::map<std::string, std::string> mp;
    mp["name"] = "test_name";
    mp["content"] = "test_content";
    mp["date"] = "test_date";
    TaskContent task;

    tasksWorker->Map2TaskStruct(mp, task);
    EXPECT_EQ(task.name, "test_name");
    EXPECT_EQ(task.content, "test_content");
    EXPECT_EQ(task.date, "test_date");   

    mp["name"]= "";
    task.Clear();
    tasksWorker->Map2TaskStruct(mp, task);
    EXPECT_EQ(task.name, "");
    EXPECT_EQ(task.content, "test_content");
    EXPECT_EQ(task.date, "test_date");   

    mp["content"]= "";
    task.Clear();
    tasksWorker->Map2TaskStruct(mp, task);
    EXPECT_EQ(task.name, "");
    EXPECT_EQ(task.content, "");
    EXPECT_EQ(task.date, "test_date");    

    mp["date"]= "";
    task.Clear();
    tasksWorker->Map2TaskStruct(mp, task);
    EXPECT_EQ(task.name, "");
    EXPECT_EQ(task.content, "");
    EXPECT_EQ(task.date, "");   

    // no error is just ok
    mp.erase("date");
    testing::internal::CaptureStdout();
    tasksWorker->Map2TaskStruct(mp, task);

    mp.erase("content");
    tasksWorker->Map2TaskStruct(mp, task);

    mp.erase("name");
    tasksWorker->Map2TaskStruct(mp, task);

}

// Create Function
TEST_F(TasksWorkerTest, Query) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    data.task_key = "task0";
    std::map<std::string, std::string> task_info;
    std::map<std::string, std::string> new_task_info;
    new_task_info["name"] = "task0";
    new_task_info["content"] = "4156 Iteration-1";
    new_task_info["date"] = "10/24/2022";
    
    // should be successful
    EXPECT_CALL(*mockedDB, getTaskNode(data.user_key, data.tasklist_key, data.task_key, task_info))
        .WillOnce(DoAll(SetArgReferee<3>(new_task_info), Return(SUCCESS)));
    EXPECT_EQ(tasksWorker->Query(data, out), SUCCESS);
    EXPECT_EQ(out.name, "task0");
    EXPECT_EQ(out.content, "4156 Iteration-1");
    EXPECT_EQ(out.date, "10/24/2022");

    // request is empty
    out = TaskContent();
    data.user_key = "";
    EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");
    data.user_key = "user0";

    data.tasklist_key = "";
    EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");
    data.tasklist_key = "tasklist0";

    data.task_key = "";
    EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");
    data.task_key = "task0";

    // key error
    EXPECT_CALL(*mockedDB, getTaskNode(data.user_key, data.tasklist_key, data.task_key, task_info))
        .WillOnce(Return(ERR_KEY));
    EXPECT_EQ(tasksWorker->Query(data, out), ERR_KEY);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");

    // task or tasklist itself does not exist
    EXPECT_CALL(*mockedDB, getTaskNode(data.user_key, data.tasklist_key, data.task_key, task_info))
        .WillOnce(Return(ERR_NO_NODE));
    EXPECT_EQ(tasksWorker->Query(data, out), ERR_NO_NODE);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");
}

TEST_F(TasksWorkerTest, Create) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    
    std::string name = "task0";
    std::string content = "4156 Iteration-1";
    std::string date = "10/24/2022";
    in = TaskContent(name, content, date);

    std::map<std::string, std::string> task_info;
    task_info["name"] = in.name;
    task_info["content"] = in.content;
    task_info["date"] = in.date;

    std::string outTaskName;

    // should be successful
    EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
    EXPECT_CALL(*mockedDB, createTaskNode(data.user_key,
                                          data.tasklist_key,
                                          task_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
    EXPECT_EQ(outTaskName, "task0");
    outTaskName = "";

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
    EXPECT_CALL(*mockedDB, createTaskNode(data.user_key, 
                                          data.tasklist_key,
                                          task_info))
        .WillOnce(Return(ERR_DUP_NODE));
    task_info["name"] = "task0(1)";
    EXPECT_CALL(*mockedDB, createTaskNode(data.user_key, 
                                         data.tasklist_key,
                                         task_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasksWorker->Create(data, in, outTaskName), SUCCESS);
    EXPECT_EQ(outTaskName, "task0(1)");
    outTaskName = "";

    // multiple calls again
    EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
    task_info["name"] = "task0";
    EXPECT_CALL(*mockedDB, createTaskNode(data.user_key, 
                                         data.tasklist_key,
                                         task_info))
        .WillOnce(Return(ERR_DUP_NODE)); 
    task_info["name"] = "task0(1)";
    EXPECT_CALL(*mockedDB, createTaskNode(data.user_key, 
                                         data.tasklist_key,
                                         task_info))
        .WillOnce(Return(ERR_DUP_NODE));
    task_info["name"] = "task0(2)";
    EXPECT_CALL(*mockedDB, createTaskNode(data.user_key, 
                                         data.tasklist_key,
                                         task_info))
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
    EXPECT_CALL(*mockedDB, deleteTaskNode(data.user_key, data.tasklist_key, data.task_key))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasksWorker->Delete(data), SUCCESS);

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
    EXPECT_CALL(*mockedDB, deleteTaskNode(data.user_key, data.tasklist_key, data.task_key))
        .WillOnce(Return(ERR_NO_NODE));
    EXPECT_EQ(tasksWorker->Delete(data), ERR_NO_NODE);
}

TEST_F(TasksWorkerTest, Revise) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    data.task_key = "task0";

    std::string name = "task0";
    std::string content = "4156 Iteration-1";
    std::string date = "10/24/2022";
    in = TaskContent(name, content, date);

    std::map<std::string, std::string> task_info;
    task_info["name"] = in.name;
    task_info["content"] = in.content;
    task_info["date"] = in.date;

    // should be successful
    EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(true));
    EXPECT_CALL(*mockedDB, reviseTaskNode(data.user_key, 
                                         data.tasklist_key, 
                                         data.task_key,
                                         task_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasksWorker->Revise(data, in), SUCCESS);

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

    // input is empty
    in = TaskContent();
    EXPECT_EQ(tasksWorker->Revise(data, in), ERR_KEY);
    in = TaskContent(name, content, date);

    // tasklist does not exist
    data.tasklist_key = "not_exist_tasklist";
    EXPECT_CALL(*mockedTaskLists, Exists(data)).WillOnce(Return(false));
    EXPECT_EQ(tasksWorker->Revise(data, in), ERR_NO_NODE);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
