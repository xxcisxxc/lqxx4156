#pragma once

#include <string>
#include <vector>
#include "api/taskContent.h"

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