#pragma once

#include <string>

// This is a structure that specifies which object is what we try to access
// ie: the object parsed from API url
struct RequestData {
    std::string user_key;
    std::string tasklist_key;
    std::string task_key;

    RequestData() {}
    
    virtual ~RequestData() {}

    RequestData(const RequestData& data) {
        user_key = data.user_key;
        tasklist_key = data.tasklist_key;
        task_key = data.task_key;
    }

    bool operator == (const RequestData& other) const {
        return this->user_key == other.user_key &&
               this->tasklist_key == other.tasklist_key &&
               this->task_key == other.task_key;
    }

    // For user in request
    bool RequestUserIsEmpty() const {
        return user_key == "";
    }

    // For tasklist in request
    bool RequestTaskListIsEmpty() const {
        return user_key == "" || tasklist_key == "";
    }

    // For task in request
    bool RequestIsEmpty() const {
        return user_key == "" || tasklist_key == "" || task_key == "";
    }
};