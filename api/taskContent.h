#pragma once

#include <string>

/*
 * @brief This is a structure that uses as either input/output for taskWorker
 * object's properties/fields For create: it contains field values of the
 * created task as input For Query: it will contain field values of the queried
 * task as output For Revise: it contains field values of the revised task as
 * input
 */

enum Priority {
  NULL_PRIORITY, // null value   0
  VERY_URGENT,   // very urgent  1
  URGENT,        // urgent       2
  NORMAL         // normal       3
};

struct TaskContent {
  /* data */
  /*
   * @brief Task name
   */
  std::string name;
  /*
   * @brief Task content
   */
  std::string content;
  /*
   * @brief Task start date
   */
  std::string startDate;
  /*
   * @brief Task end date
   */
  std::string endDate;
  /*
   Deprecated in real usage.
   Only for testing purpose.
   */
  std::string date;
  /*
   * @brief Task priority: very urgent, urgent, normal
   */
  Priority priority;
  /*
   * @brief Task progress: To do, Doing or Done
   */
  std::string status;

  /* methods */
  /*
   * @brief TaskContent default constructor
   */
  TaskContent() : priority(NULL_PRIORITY) {}

  /*
   * @brief TaskContent constructor with parameters
   * @param _name Task name
   * @param _content Task content
   * @param _date Task due date
   */
  TaskContent(std::string &_name, std::string &_content,
              std::string &_startDate, std::string &_endDate,
              Priority &_priority, std::string &_status)
      : name(_name), content(_content), startDate(_startDate),
        endDate(_endDate), priority(_priority), status(_status) {}

  /*
   * @brief Check if the task has a name
   * @return true if the task has a name
   */
  bool LoseKey() { return name == ""; }

  /*
   * @brief Check if the TaskContent object is empty
   * @return true if the TaskContent object is empty
   */
  bool IsEmpty() {
    return name == "" && content == "" && startDate == "" && endDate == "" &&
           priority == NULL_PRIORITY && status == "";
    ;
  }

  /*
   * @brief clear the TaskContent object
   */
  void Clear() {
    name = "";
    content = "";
    startDate = "";
    endDate = "";
    priority = NULL_PRIORITY;
    status = "";
  }
  /**
   * @brief Check
   * @return true if member variables are valid
   */
  bool CheckFormat() {
    // check startDate format
    if(startDate != "") return false;
    // check endDate format
    if(endDate != "") return false;
    // check priority format
    if(priority != NULL_PRIORITY && priority != VERY_URGENT && priority != URGENT && priority != NORMAL) return false;
    // check status format: To do, Doing or Done
    if(status != "" && status != "To do" && status != "Doing" && status != "Done") return false;
    // pass all checks
    return true;
  }
};
