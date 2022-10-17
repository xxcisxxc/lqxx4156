#pragma once

#include <string>

// This is a structure that uses as either input/output for taskWorker object's properties/fields
// For create: it contains field values of the created task as input
// For Query: it will contain field values of the queried task as output
// For Revise: it contains field values of the revised task as input
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
