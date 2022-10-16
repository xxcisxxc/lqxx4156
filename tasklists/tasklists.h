#pragma once

#include <string>
#include <db/DB.h>

// TODO: pass data as a Data structure
struct RequestData {
    std::string user_key;
    std::string tasklist_key;
    std::string task_key;
    std::string request_field_name;
    std::string request_field_value;
};

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
};