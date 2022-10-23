#pragma once

#include "api/requestData.h"
#include "api/tasklistContent.h"
#include <db/DB.h>
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
   * @brief a instance of DB
   *
   */
  DB &db_instance;

  /* methods */

  /**
   * @brief rename the tasklist if the given name existed
   *
   * @param [in] tasklist_name given name of tasklists
   * @param [in] suffix suffix to be added
   * @return string of revised name
   */
  std::string Rename(const std::string &tasklist_name, int suffix);

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
  TaskListsWorker(DB &_db_instance) : db_instance(_db_instance) {}

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
   * @brief Check if a tasklist exists
   *
   * @param [in] data target tasklist we'd want to check
   * @return true if the tasklist exists
   */
  virtual bool Exists(const RequestData &data);
};