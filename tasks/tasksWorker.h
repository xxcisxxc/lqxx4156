#pragma once

#include "api/requestData.h"
#include "api/taskContent.h"
#include <db/DB.h>
#include <map>
#include <string>

class TaskListsWorker; // forward definition

/*
  * @brief This class is a worker class for tasks.
  *
  */
class TasksWorker {
private:
  /* data */
  /**
   * @brief DB object
   *
   */
  DB *db;
  /**
   * @brief TaskListsWorker object
   *
   */
  TaskListsWorker *taskListsWorker;
  /* method */
  /**
   * @brief Rename the task name if it is duplicated.
   *
   * @param name
   * @param suffix
   * @return std::string
   */
  virtual std::string Rename(const std::string &tasklist_name, int suffix);

public:
  /**
   * @brief Construct a new Tasks Worker object
   *
   * @param _db
   * @param _taskListsWorker
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
   * @brief Construct a new Tasks Worker object
   * 
   * @param _db 
   * @param _taskListWorker 
   */
  TasksWorker(DB *_db, TaskListsWorker *_taskListWorker);
  /**
   * @brief Destroy the Tasks Worker object
   * 
   */
  virtual ~TasksWorker();
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
   * @brief Get all tasks in the tasklist and return the task name list in outTaskNameList.
   * 
   * @param data 
   * @param out 
   * @return returnCode 
   */
  virtual returnCode GetAllTasksName(const RequestData &data,
                                     std::vector<std::string> &outTaskNameList);
};