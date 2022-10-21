#pragma once

#include <string>
#include <vector>
#include "api/taskContent.h"

// This is a structure that uses as either input/output for tasklistWorker object's properties/fields
// For create: it contains field values of the created tasklist as input
// For Query: it will contain field values of the queried tasklist as output
// For Revise: it contains field values of the revised tasklist as input
struct TasklistContent {
    std::string name; // key: name
    std::string content; // descripton of the tasklist
    std::string date; // date created

    TasklistContent() {}

    TasklistContent(std::string& _name, std::string& _content, std::string& _date): name(_name), content(_content), date(_date) {}

    bool LoseKey() {
        return name == "";
    }

    bool IsEmpty() {
        return name == "" && content == "" && date == "";
    }
};