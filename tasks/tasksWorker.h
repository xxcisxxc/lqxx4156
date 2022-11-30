#pragma once

#include "api/requestData.h"
#include "api/taskContent.h"
#include "common/utils.h"
#include "db/DB.h"
#include "tasklists/tasklistsWorker.h"
#include <map>
#include <string>

class TaskListsWorker; // forward definition

/*
 * @brief This class is a worker class for tasks.
 *
 */
class TasksWorker {
protected:
  /* data */
  /**
   * @brief DB pointer
   *
   */
  std::shared_ptr<DB> db;
  /**
   * @brief TaskListsWorker pointer
   *
   */
  std::shared_ptr<TaskListsWorker> taskListsWorker;

public:
  /* method */
  /**
   * @brief Construct a new Tasks Worker object
   *
   * @param _db
   * @param _taskListWorker
   */
  TasksWorker(std::shared_ptr<DB> _db = nullptr,
              std::shared_ptr<TaskListsWorker> _taskListWorker = nullptr);
  /**
   * @brief Destroy the Tasks Worker object
   *
   */
  virtual ~TasksWorker();
  /**
   * @brief Construct a new Tasks Worker object
   *
   * @param taskContent
   * @param task_info
   */
  virtual void TaskStruct2Map(const TaskContent &taskContent,
                              std::map<std::string, std::string> &task_info);
  /**
   * @brief Convert map to TaskContent object
   *
   * @param task_info
   * @param taskContent
   */
  virtual void
  Map2TaskStruct(const std::map<std::string, std::string> &task_info,
                 TaskContent &taskContent);
  /**
   * @brief Query the task and return the task content in out.
   *
   * @param data
   * @param out
   * @return returnCode
   */
  virtual returnCode Query(const RequestData &data, TaskContent &out);
  /**
   * @brief Create a Task object and return the task name in outTaskName.
   *
   * @param data
   * @param in
   * @param outTaskName
   * @return returnCode
   */
  virtual returnCode Create(const RequestData &data, TaskContent &in,
                            std::string &outTaskName);
  /**
   * @brief Delete the task.
   *
   * @param data
   * @return returnCode
   */
  virtual returnCode Delete(const RequestData &data);

  /**
   * @brief Update the task with the TaskContent object in.
   *
   * @param data
   * @param in
   * @return returnCode
   */
  virtual returnCode Revise(const RequestData &data, TaskContent &in);

  /**
   * @brief Get all tasks in the tasklist and return the task name list in
   * outTaskNameList.
   *
   * @param data
   * @param out
   * @return returnCode
   */
  virtual returnCode GetAllTasksName(const RequestData &data,
                                     std::vector<std::string> &outTaskNameList);
};