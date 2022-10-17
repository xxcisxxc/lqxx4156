#include <map>
#include <iostream>
#include "tasklists.h"

TaskLists :: ~TaskLists() {
    
}

std::string TaskLists :: NextName(const std::string& tasklist_name, int suffix) {
    if (suffix == 0) return tasklist_name;
    return tasklist_name + "(" + std::to_string(suffix) + ")";
}


returnCode TaskLists :: Query(const RequestData& data, std::string& out) {

    std::map<std::string, std::string> task_list_info;
    task_list_info[data.request_field_name] = "";
    returnCode ret = db_instance.getTaskListNode(data.user_key,
                                data.tasklist_key,
                                task_list_info);

    out = task_list_info[data.request_field_name];
    return ret;
}

returnCode TaskLists :: Create(const RequestData& data) {
    // check if requested task_list_name can be created

    std::map<std::string, std::string> task_list_info;
    std::string queryString = data.tasklist_key;
    int suffix = 0;
    returnCode ret;
    do {
        task_list_info.clear();
        queryString = NextName(data.tasklist_key, suffix++);
        task_list_info[queryString] = "";
        ret = db_instance.createTaskListNode(data.user_key,
                                             task_list_info);
    } while (ret == ERR_DUP_NODE);

    return ret;
}

returnCode TaskLists :: Delete(const RequestData& data) {
    returnCode ret = db_instance.deleteTaskListNode(data.user_key,
                                                    data.tasklist_key);
    return ret;            
}

returnCode TaskLists :: Revise(const RequestData& data) {
    std::map<std::string, std::string> task_list_info;
    task_list_info[data.request_field_name] = data.request_field_value;
    returnCode ret = db_instance.reviseTaskListNode(data.user_key,
                                                    data.tasklist_key,
                                                    task_list_info);
    return ret;
}

bool TaskLists :: Exists(const RequestData& data) {
    std::string out;
    returnCode ret = Query(data, out);
    if(ret == ERR_KEY || ret == ERR_NO_NODE) {
        return false;
    }
    return true;
}