#pragma once

#include <string>
#include <map>
#include <db/DB.h>
#include "api/taskContent.h"
#include "api/requestData.h"

class TaskListsWorker;  // forward definition

class TasksWorker {
private:
    DB* db;

    TaskListsWorker* taskListsWorker;

    virtual std::string Rename(const std::string& tasklist_name, int suffix);

public: 
    virtual void TaskStruct2Map(const TaskContent& taskContent, std::map<std::string, std::string>& task_info);

    virtual void Map2TaskStruct(const std::map<std::string, std::string>& task_info, TaskContent& taskContent);

    TasksWorker(DB* _db, TaskListsWorker* _taskListWorker);

    virtual ~TasksWorker();

    virtual returnCode Query(const RequestData& data, TaskContent& out);

    virtual returnCode Create(const RequestData& data, TaskContent& in, std::string& outTaskName);

    virtual returnCode Delete(const RequestData& data);

    virtual returnCode Revise(const RequestData& data, TaskContent& in);

    virtual returnCode GetAllTasksName(const RequestData& data, std::vector<std::string>& outTaskNameList);
};