#pragma once

#include "common/utils.h"
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
   * by default, it is private
   */
  std::string visibility;

  /* methods */

  /**
   * @brief Construct a new Tasklist Content object
   * by default, it is private
   */
  TasklistContent(): visibility("private") {}

  /**
   * @brief Construct a new Tasklist Content object
   *
   * @param _name value for tasklist name
   * @param _content value for tasklist content
   * @param _date value for tasklist date
   * @param _vis value for tasklist visibility
   */
  template <typename Name, typename Content, typename Date, typename Visibility>
  TasklistContent(Name &&_name, Content &&_content, Date &&_date,
                  Visibility &&_vis)
      : name(std::forward<Name>(_name)),
        content(std::forward<Content>(_content)),
        date(std::forward<Date>(_date)),
        visibility(std::forward<Visibility>(_vis)) {
          // by default, it is private
          if(visibility.empty()) {
            visibility = "false";
          }
      }

  /**
   * @brief check if the key -- name is missing
   *
   * @return true if name is missing
   */
  bool MissingKey() { return name == ""; }

  /**
   * @brief check if the content is empty
   *
   * @return true if content is empty
   */
  bool IsEmpty() { return name == "" && content == "" && date == ""; }

  /**
   * @brief check if tasklist object is valid
   *
   * @return true if it is valid
   */
  bool IsValid() { 
    if(!date.empty() && !Common::IsDate(date)) 
      return false;
    if (!visibility.empty() && visibility != "public" &&
      visibility != "shared" && visibility != "private")
      return false;
    // pass all checks
    return true;
  }
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
   * by default, it is read-only permission, which is "false"
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
   * @param _tasklist_name value for tasklist name
   * @param _permission value for tasklist permission
   */
  template <typename Name, typename TasklistName, typename Permission>
  shareInfo(Name &&_name, TasklistName &&_tasklist_name, Permission &&_permission)
      : user_name(std::forward<Name>(_name)),
        task_list_name(std::forward<TasklistName>(_tasklist_name)),
        permission(std::forward<Permission>(_permission)) {}
  /**
   * @brief check if the user_name is empty
   * @return true if it has user_name
   */
  bool MissingKey() { return user_name.empty(); }
};