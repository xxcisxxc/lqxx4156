#pragma once

#include "api/requestData.h"
#include "api/tasklistContent.h"
#include <db/DB.h>
#include <string>
#include <vector>

class TasksWorker;

class TaskListsWorker {
private:
  DB &db_instance;
  TasksWorker *tasksworker;
  std::string Rename(const std::string &tasklist_name, int prev_suffix);
  void Content2Map(const TasklistContent &tasklistContent,
                   std::map<std::string, std::string> &task_info);
  void Map2Content(const std::map<std::string, std::string> &task_info,
                   TasklistContent &tasklistContent);

public:
  TaskListsWorker(DB &_db_instance) : db_instance(_db_instance) {}

  virtual ~TaskListsWorker();

  virtual returnCode Query(const RequestData &data, TasklistContent &out);

  virtual returnCode Create(const RequestData &data, TasklistContent &in,
                            std::string &outTasklistName);

  virtual returnCode Delete(const RequestData &data);

  virtual returnCode Revise(const RequestData &data, TasklistContent &in);

  virtual returnCode GetAllTasklist(const RequestData &data,
                                    std::vector<std::string> &outNames);

  virtual bool Exists(const RequestData &data);
};