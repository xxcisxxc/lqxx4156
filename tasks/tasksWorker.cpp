#include "tasksWorker.h"
#include "api/taskContent.h"
#include "tasklists/tasklistsWorker.h"
#include <iostream>

TasksWorker::TasksWorker(DB *_db, TaskListsWorker *_taskListsWorker)
    : db(_db), taskListsWorker(_taskListsWorker) {}

TasksWorker::~TasksWorker() {}

std::string TasksWorker::Rename(const std::string &name, int suffix) {
  if (suffix <= 0)
    return name;
  return name + "(" + std::to_string(suffix) + ")";
}

void TasksWorker::TaskStruct2Map(
    const TaskContent &taskContent,
    std::map<std::string, std::string> &task_info) {
  if (!taskContent.name.empty()) {
    task_info["name"] = taskContent.name;
  }
  if (!taskContent.content.empty()) {
    task_info["content"] = taskContent.content;
  }
  if (!taskContent.startDate.empty()) {
    task_info["startDate"] = taskContent.startDate;
  }
  if (!taskContent.endDate.empty()) {
    task_info["endDate"] = taskContent.endDate;
  }
  /* deprecated, only for test purpose */
  if (!taskContent.date.empty()) {
    task_info["date"] = taskContent.date;
  }
  if (taskContent.priority != NULL_PRIORITY) {
    task_info["priority"] = std::to_string((int)taskContent.priority);
  }
  if (!taskContent.status.empty()) {
    task_info["status"] = taskContent.status;
  }
}

void TasksWorker::Map2TaskStruct(
    const std::map<std::string, std::string> &task_info,
    TaskContent &taskContent) {
  if (task_info.count("name"))
    taskContent.name = task_info.at("name");

  if (task_info.count("content"))
    taskContent.content = task_info.at("content");

  if (task_info.count("startDate"))
    taskContent.startDate = task_info.at("startDate");

  if (task_info.count("endDate"))
    taskContent.endDate = task_info.at("endDate");

  /* deprecated, only for test purpose */
  if (task_info.count("date"))
    taskContent.date = task_info.at("date");

  if (task_info.count("priority"))
    taskContent.priority = (Priority)stoi(task_info.at("priority"));

  if (task_info.count("status"))
    taskContent.status = task_info.at("status");
}

returnCode TasksWorker::Query(const RequestData &data, TaskContent &out) {
  // request has empty value
  if (data.RequestIsEmpty())
    return ERR_KEY;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db->checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
  }

  // can access
  std::map<std::string, std::string> task_info;

  // get all available fields
  returnCode ret = db->getTaskNode(data.user_key, data.tasklist_key,
                                   data.task_key, task_info);

  // error in tasklist_key or task_key
  if (ret == ERR_KEY || ret == ERR_NO_NODE) {
    return ret;
  }

  // assign value to out object
  Map2TaskStruct(task_info, out);

  return ret;
}

returnCode TasksWorker::Create(const RequestData &data, TaskContent &in,
                               std::string &outTaskName) {
  // request has empty value
  if (data.RequestTaskListIsEmpty())
    return ERR_KEY;

  // check if in is valid
  if (!in.IsValid())
    return ERR_FORMAT;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db->checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
    if (!permission) {
      // read only permission cannot create
      return ERR_ACCESS;
    }
  }

  // can access
  // input value does not have a key
  if (in.LoseKey())
    return ERR_KEY;

  // tasklist itself does not exist
  if (!taskListsWorker->Exists(data)) {
    // maybe create taskList first
    return ERR_NO_NODE;
  }

  std::map<std::string, std::string> task_info;
  TaskStruct2Map(in, task_info);

  int suffix = 0;
  returnCode ret;
  std::string originTaskName = task_info["name"];
  do {
    // For Create, data.task_key can be "", so we should use task_info["name"]
    outTaskName = Rename(originTaskName, suffix++);
    task_info["name"] = outTaskName;
    ret = db->createTaskNode(data.user_key, data.tasklist_key, task_info);
  } while (ret == ERR_DUP_NODE);

  return ret;
}

returnCode TasksWorker::Delete(const RequestData &data) {
  // request has empty value
  if (data.RequestIsEmpty())
    return ERR_KEY;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db->checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
    if (!permission) {
      // read only permission cannot delete
      return ERR_ACCESS;
    }
  }

  // can access
  // tasklist itself does not exist
  if (!taskListsWorker->Exists(data)) {
    return ERR_NO_NODE;
  }

  returnCode ret =
      db->deleteTaskNode(data.user_key, data.tasklist_key, data.task_key);
  return ret;
}

returnCode TasksWorker::Revise(const RequestData &data, TaskContent &in) {
  // request has empty value
  if (data.RequestIsEmpty())
    return ERR_KEY;

  // check if in is valid
  if (!in.IsValid())
    return ERR_FORMAT;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db->checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
    if (!permission) {
      // read only permission cannot revise
      return ERR_ACCESS;
    }
  }
  // can access

  // if revise, then struct "in" is the value that we would like to revise
  // eg. revise name, then in->name = "revisedName", but in->content = ""
  // because we do not need to revise content
  if (in.IsEmpty())
    return ERR_KEY; // no such input

  // tasklist itself does not exist
  if (!taskListsWorker->Exists(data)) {
    return ERR_NO_NODE;
  }

  std::map<std::string, std::string> task_info;
  TaskStruct2Map(in, task_info);

  returnCode ret = db->reviseTaskNode(data.user_key, data.tasklist_key,
                                      data.task_key, task_info);
  return ret;
}

returnCode
TasksWorker::GetAllTasksName(const RequestData &data,
                             std::vector<std::string> &outTaskNameList) {
  // request has empty value
  if (data.RequestTaskListIsEmpty())
    return ERR_KEY;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db->checkAccess(data.other_user_key, data.user_key,
                                     data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
  }
  // can access

  // tasklist itself does not exist
  if (!taskListsWorker->Exists(data)) {
    return ERR_NO_NODE;
  }

  returnCode ret =
      db->getAllTaskNodes(data.user_key, data.tasklist_key, outTaskNameList);
  return ret;
}