#pragma once

#include "common/utils.h"
#include <iostream>
#include <sstream>
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
  template <typename Name, typename Content, typename StartDate,
            typename EndDate, typename PriorityType, typename Status>
  TaskContent(Name &&_name, Content &&_content, StartDate &&_startDate,
              EndDate &&_endDate, PriorityType &&_priority, Status &&_status)
      : name(std::forward<Name>(_name)),
        content(std::forward<Content>(_content)),
        startDate(std::forward<StartDate>(_startDate)),
        endDate(std::forward<EndDate>(_endDate)),
        priority(std::forward<PriorityType>(_priority)),
        status(std::forward<Status>(_status)) {}

  /*
   * @brief Check if the task has a name
   * @return true if the task has a name
   */
  bool MissingKey() { return name == ""; }

  /*
   * @brief Check if the TaskContent object is empty
   * @return true if the TaskContent object is empty
   */
  bool IsEmpty() {
    return name == "" && content == "" && startDate == "" && endDate == "" &&
           priority == NULL_PRIORITY && status == "";
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
   * @brief get time_t object from date string
   * @return none
   */
  void GetTimeObject(const std::string &date, time_t &time) {
    int d, m, y;
    char delimiter1;
    char delimiter2;
    std::istringstream iss(date);
    if (iss >> m >> delimiter1 >> d >> delimiter2 >> y) {
      struct tm t = {0};
      t.tm_mday = d;
      t.tm_mon = m - 1;
      t.tm_year = y - 1900;

      // make time using time struct tm
      time = mktime(&t);
    }
  }
  /**
   * @brief Compare two time strings
   * @return true if the first time string is earlier than the second one
   */
  bool CompareTime(const std::string &startDate, const std::string &endDate) {
    // get time_t object from date string
    time_t startTime, endTime;
    GetTimeObject(startDate, startTime);
    GetTimeObject(endDate, endTime);

    // compare start date and end date
    double diff = difftime(startTime, endTime);
    return diff <= 0;
  }

  /**
   * @brief Check if the task is valid
   * @return true if member variables are valid
   */
  bool IsValid() {
    // check startDate format
    if (!startDate.empty() && !Common::IsDate(startDate))
      return false;
    // check endDate format
    if (!endDate.empty() && !Common::IsDate(endDate))
      return false;
    // check whether startDate and endDate exist at the same time
    if (startDate.empty() && !endDate.empty())
      return false;
    else if (!startDate.empty() && endDate.empty())
      return false;
    // check startDate <= endDate
    if (!startDate.empty() && !endDate.empty() &&
        !CompareTime(startDate, endDate))
      return false;
    // check priority format
    if (priority != NULL_PRIORITY && priority != VERY_URGENT &&
        priority != URGENT && priority != NORMAL)
      return false;
    // check status format: To Do, Doing or Done
    if (!status.empty() && status != "To Do" && status != "Doing" &&
        status != "Done")
      return false;
    // pass all checks
    return true;
  }
};
