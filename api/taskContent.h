#pragma once

#include <string>

/*
 * @brief This is a structure that uses as either input/output for taskWorker
 * object's properties/fields For create: it contains field values of the
 * created task as input For Query: it will contain field values of the queried
 * task as output For Revise: it contains field values of the revised task as
 * input
 */

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
   * @brief Task due date
   */
  std::string date;

  /* methods */
  /*
   * @brief TaskContent default constructor
   */
  TaskContent() {}

  /*
   * @brief TaskContent constructor with parameters
   * @param _name Task name
   * @param _content Task content
   * @param _date Task due date
   */
  TaskContent(std::string &_name, std::string &_content, std::string &_date)
      : name(_name), content(_content), date(_date) {}

  /*
   * @brief Check if the task has a name
   * @return true if the task has a name
   */
  bool LoseKey() { return name == ""; }

  /*
   * @brief Check if the TaskContent object is empty
   * @return true if the TaskContent object is empty
   */
  bool IsEmpty() { return name == "" && content == "" && date == ""; }

  /*
   * @brief clear the TaskContent object
   */
  void Clear() {
    name = "";
    content = "";
    date = "";
  }
};
