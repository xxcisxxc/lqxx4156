#pragma once

#include "api/requestData.h"
#include "api/tasklistContent.h"
#include "common/errorCode.h"
#include "common/utils.h"
#include "db/DB.h"
#include "users/users.h"
#include <string>
#include <vector>

/**
 * @brief This class is a worker class for tasklists
 *
 */
class TaskListsWorker {
private:
  /* data */

  /**
   * @brief DB pointer
   *
   */
  std::shared_ptr<DB> db;

  /**
   * @brief user controller pointer
   *
   */
  std::shared_ptr<Users> users;

  /* methods */
  /**
   * @brief convert tasklist content struct to map
   *
   * @param [in] tasklistContent source struct
   * @param [out] task_info target map to be filled
   */
  void Content2Map(const TasklistContent &tasklistContent,
                   std::map<std::string, std::string> &task_info);

  /**
   * @brief convert map to content struct
   *
   * @param [in] task_info source map
   * @param [out] tasklistContent target struct to be filled
   */
  void Map2Content(const std::map<std::string, std::string> &task_info,
                   TasklistContent &tasklistContent);

public:
  /**
   * @brief Construct a new Task Lists Worker object
   *
   * @param _db_instance value for DB
   */
  TaskListsWorker(std::shared_ptr<DB> _db = nullptr,
                  std::shared_ptr<Users> _users = nullptr);

  /**
   * @brief Destroy the Task Lists Worker object
   *
   */
  virtual ~TaskListsWorker();

  /**
   * @brief Query the database to get a tasklist
   *
   * @param [in] data target tasklist that we'd want to query for
   * @param [out] out target tasklist's properties
   * @return returnCode
   */
  virtual returnCode Query(const RequestData &data, TasklistContent &out);

  /**
   * @brief Create a new tasklist in database
   *
   * @param [in] data target user that we'd want to create tasklist for
   * @param [in] in new tasklist's properties
   * @param [out] outTasklistName new tasklist name successfully created
   * @return returnCode
   */
  virtual returnCode Create(const RequestData &data, TasklistContent &in,
                            std::string &outTasklistName);

  /**
   * @brief Delete a tasklist from database
   *
   * @param [in] data target tasklist we'd want to delete
   * @return returnCode
   */
  virtual returnCode Delete(const RequestData &data);

  /**
   * @brief Revise a tasklist in database
   *
   * @param [in] data target tasklist we'd want to revise
   * @param [in] in target tasklist's new properties
   * @return returnCode
   */
  virtual returnCode Revise(const RequestData &data, TasklistContent &in);

  /**
   * @brief Get the All Tasklist names
   *
   * @param [in] data target user we'd want to get tasklist for
   * @param [out] outNames all tasklist names
   * @return returnCode
   */
  virtual returnCode GetAllTasklist(const RequestData &data,
                                    std::vector<std::string> &outNames);

  /**
   * @brief Get the all Tasklist info that are shared by others
   *
   * @param [in] data target user that we'd want to get all grant for
   * @param [out] out_list vector of shareInfo with information about the users,
   * tasklists and permissions that current user have access to
   * @return returnCode
   */
  virtual returnCode GetAllAccessTaskList(const RequestData &data,
                                          std::vector<shareInfo> &out_list);

  /**
   * @brief Get the all Tasklist info that are sharing to others
   *
   * @param [in] data target user and tasklist we'd want to get all grant for
   * @param [out] out_list vector of shareInfo with information about the users
   * who have access to the current user's tasklist
   * @param [out] isPublic is true when the requested tasklist is public to
   * everyone, and false otherwise
   * @return returnCode
   */
  virtual returnCode GetAllGrantTaskList(const RequestData &data,
                                         std::vector<shareInfo> &out_list,
                                         bool &isPublic);

  /**
   * @brief Create if not exists or Revise the share status for a tasklist. It
   * is guaranteed that all operations on users before errUser in the in_list
   * are successful
   *
   * @param [in] data target tasklist that we'd want to revise share status
   * @param [in] in_list list of shareInfo to add/revise for target tasklist
   * @param [out] errUser the user that causes the err
   * @return returnCode
   */
  virtual returnCode ReviseGrantTaskList(const RequestData &data,
                                         std::vector<shareInfo> &in_list,
                                         std::string &errUser);

  /**
   * @brief Delete the share for a tasklist. It is guaranteed that
   * all deletions on users before errUser in the in_list are successful
   *
   * @param [in] data target tasklists that we'd want to delete share status
   * @return returnCode
   */
  virtual returnCode RemoveGrantTaskList(const RequestData &data);

  // for compilation before api revise
  virtual returnCode RemoveGrantTaskList(const RequestData &data,
                                         std::vector<std::string> &in_list,
                                         std::string &errUser) {
    return SUCCESS;
  }
  /**
   * @brief Get all public tasklists
   *
   * @param [out] out_list a list of (user, task_list) pair that are public
   * @return returnCode
   */
  virtual returnCode GetAllPublicTaskList(
      std::vector<std::pair<std::string, std::string>> &out_list);

  /**
   * @brief Check if a tasklist exists
   *
   * @param [in] data target tasklist we'd want to check
   * @return true if the tasklist exists
   */
  virtual bool Exists(const RequestData &data);

  /**
   * @brief Get a tasklist's visibility
   *
   * @param [in] data target tasklist we'd want to check
   * @return true if the tasklist exists
   */
  virtual returnCode GetVisibility(const RequestData &data,
                                   std::string &visibility);
};