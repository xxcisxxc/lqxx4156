#pragma once

#include "api/taskContent.h"
#include <string>
#include <vector>

/**
 * @brief This is a structure that uses as either input/output for
 * tasklistWorker object's properties/fields. For create: it contains field
 * values of the created tasklist as input. For Query: it will contain field
 * values of the queried tasklist as output. For Revise: it contains field
 * values of the revised tasklist as input
 */
struct TasklistContent {
  /* data */

  /**
   * @brief key: name
   *
   */
  std::string name;

  /**
   * @brief tasklist content
   *
   */
  std::string content;

  /**
   * @brief date created
   *
   */
  std::string date;

  /* methods */

  /**
   * @brief Construct a new Tasklist Content object
   *
   */
  TasklistContent() {}

  /**
   * @brief Construct a new Tasklist Content object
   *
   * @param _name value for tasklist name
   * @param _content value for tasklist content
   * @param _date value for tasklist date
   */
  TasklistContent(std::string &_name, std::string &_content, std::string &_date)
      : name(_name), content(_content), date(_date) {}

  /**
   * @brief check if the key -- name is missing
   *
   * @return true if name is missing
   */
  bool LoseKey() { return name == ""; }

  /**
   * @brief check if the content is empty
   *
   * @return true if content is empty
   */
  bool IsEmpty() { return name == "" && content == "" && date == ""; }
};