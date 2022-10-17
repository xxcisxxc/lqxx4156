#pragma once

#include <string>
#include <map>
#include <db/DB.h>
#include "taskContent.h"
#include "requestData.h"

class TaskLists;  // forward definition

class TasksWorker {
private:
    DB* db;
    
    TaskLists* taskList;

    std::string RenameTask(const std::string& tasklist_name, int suffix);

    void TaskStruct2Map(const TaskContent& taskContent, std::map<std::string, std::string>& task_info);

    void Map2TaskStruct(const std::map<std::string, std::string>& task_info, TaskContent& taskContent);
    
public:
    TasksWorker(DB* _db, TaskLists* _taskList);

    virtual ~TasksWorker();

    virtual returnCode Query(const RequestData& data, TaskContent& out);

    virtual returnCode Create(const RequestData& data, TaskContent& in, std::string& outTaskName);

    virtual returnCode Delete(const RequestData& data);

    virtual returnCode Revise(const RequestData& data, TaskContent& in);
};