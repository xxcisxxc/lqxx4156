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

  /**
   * @brief visibility of the tasklist: true for public, false for private
   * 
   */
  std::string visibility;

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
  TasklistContent(std::string &_name, std::string &_content, std::string &_date, std::string &_vis)
      : name(_name), content(_content), date(_date), visibility(_vis) {}

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

/**
 * @brief This is a structure that uses as either input/output for
 * tasklistWorker object's properties/fields. It is used to deal with sharing 
 * of tasklists between users. 
 * 
 */
struct shareInfo {
  /**
   * @brief user name
   */
  std::string user_name;

  /**
   * @brief tasklist name on share
   */
  std::string task_list_name;

  /**
   * @brief permission of tasklist
   */
  bool permission;

  /**
   * @brief Construct a new shareInfo object
   *
   */
  shareInfo() {}

  /**
   * @brief Construct a new shareInfo object
   *
   * @param _name value for tasklist name
   * @param _content value for tasklist content
   * @param _date value for tasklist date
   */
  shareInfo(std::string& _user_name, std::string& _task_list_name, bool _permission)
    : user_name(_user_name), task_list_name(_task_list_name), permission(_permission) {}
};