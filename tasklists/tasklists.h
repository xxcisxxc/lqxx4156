#pragma once

#include <string>
#include <db/DB.h>
#include "../tasks/requestData.h"

class TasksWorker;

class TaskLists {
private:
    DB& db_instance;
    std::string NextName(const std::string& tasklist_name, int prev_suffix);
public:
    
    TaskLists(DB& _db_instance): db_instance(_db_instance) {}

    virtual ~TaskLists();

    virtual returnCode Query(const RequestData& data, std::string& out);

    virtual returnCode Create(const RequestData& data);

    virtual returnCode Delete(const RequestData& data);

    virtual returnCode Revise(const RequestData& data);

    virtual bool Exists(const RequestData& data);
};