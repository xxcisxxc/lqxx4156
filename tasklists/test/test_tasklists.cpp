#include <tasklists/tasklists.h>
#include <db/DB.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

class MockedDB : public DB {
public:
    MOCK_METHOD(returnCode, 
                createTaskListNode, 
                (const std::string &user_pkey, (const std::map<std::string, std::string> &) task_list_info), 
                (override));
    MOCK_METHOD(returnCode,
                getTaskListNode,
                (const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 (const std::map<std::string, std::string> &) task_list_info),
                (override));
    MOCK_METHOD(returnCode,
                deleteTaskListNode,
                (const std::string &user_pkey,
                 const std::string &task_list_pkey),
                 (override));
    MOCK_METHOD(returnCode,
                reviseTaskListNode,
                (const std::string &user_pkey,
                const std::string &task_list_pkey,
                (const std::map<std::string, std::string> &) task_list_info),
                (override));
    
    MockedDB() : DB("localhost") {}
};

class TaskListTest : public ::testing::Test {
protected:

    void SetUp() override {
        tasklists = std::make_shared<TaskLists>(mockedDB);
    }

    void TearDown() override {
        // pass
    }

    MockedDB mockedDB;
    std::shared_ptr<TaskLists> tasklists;
    RequestData data;
};

using namespace ::testing;

TEST_F(TaskListTest, Create) {
    // normal create
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    std::map<std::string, std::string> task_list_info;
    task_list_info[data.tasklist_key] = "";

    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklists->Create(data), SUCCESS);

    // multiple calls to createTaskListNode if previous name is duplicated
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(ERR_DUP_NODE));
    task_list_info.clear();
    task_list_info["tasklist0(1)"] = "";
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(ERR_DUP_NODE));
    task_list_info.clear();
    task_list_info["tasklist0(2)"] = "";
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklists->Create(data), SUCCESS);
}

TEST_F(TaskListTest, Query) {
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    data.request_field_name = "field0";
    std::map<std::string, std::string> task_list_info;
    task_list_info[data.request_field_name] = "";
    std::string out;
    auto fill = [&]() {
        task_list_info["field0"] = "value0";
        return SUCCESS;
    };

    EXPECT_CALL(mockedDB, getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Invoke(fill));
    EXPECT_EQ(tasklists->Query(data, out), SUCCESS);
    EXPECT_EQ(task_list_info["field0"], "value0");
}

TEST_F(TaskListTest, Delete) {
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    EXPECT_CALL(mockedDB, deleteTaskListNode(data.user_key, data.tasklist_key))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklists->Delete(data), SUCCESS);
    
    data.tasklist_key = "not_exist_task";
    EXPECT_CALL(mockedDB, deleteTaskListNode(data.user_key, data.tasklist_key))
        .WillOnce(Return(ERR_NO_NODE));
    EXPECT_EQ(tasklists->Delete(data), ERR_NO_NODE);

}

TEST_F(TaskListTest, Revise) {
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    data.request_field_name = "field0";
    data.request_field_value = "value0";
    std::map<std::string, std::string> task_list_info;
    task_list_info[data.request_field_name] = data.request_field_value;
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklists->Revise(data), SUCCESS);
    
    data.tasklist_key = "not_exist_task";
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(ERR_NO_NODE));
    EXPECT_EQ(tasklists->Revise(data), ERR_NO_NODE);

    data.tasklist_key = "value0";
    task_list_info.clear();
    data.request_field_name = "not_exist_field";
    task_list_info["not_exist_field"] = data.request_field_value;
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(ERR_KEY));
    EXPECT_EQ(tasklists->Revise(data), ERR_KEY);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

