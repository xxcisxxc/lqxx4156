#pragma once

#include <string>

struct TaskContent {
    std::string name;  // key: name
    std::string content; // content
    std::string date; // date created

    TaskContent() {}

    TaskContent(std::string& _name, std::string& _content, std::string& _date): name(_name), content(_content), date(_date) {}

    bool LoseKey() {
        return name == "";
    }

    bool IsEmpty() {
        return name == "" && content == "" && date == "";
    }
};
