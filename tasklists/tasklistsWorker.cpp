#include "tasklistsWorker.h"
#include <iostream>
#include <map>

TaskListsWorker ::~TaskListsWorker() {}

std::string TaskListsWorker ::Rename(const std::string &tasklist_name,
                                     int suffix) {
  if (suffix == 0)
    return tasklist_name;
  return tasklist_name + "(" + std::to_string(suffix) + ")";
}

void TaskListsWorker ::Content2Map(
    const TasklistContent &tasklistContent,
    std::map<std::string, std::string> &task_list_info) {
  if (tasklistContent.name != "")
    task_list_info["name"] = tasklistContent.name;
  if (tasklistContent.content != "")
    task_list_info["content"] = tasklistContent.content;
  if (tasklistContent.date != "")
    task_list_info["date"] = tasklistContent.date;
}

void TaskListsWorker ::Map2Content(
    const std::map<std::string, std::string> &task_list_info,
    TasklistContent &tasklistContent) {
  if (!task_list_info.count("name"))
    std::cout << "lose name" << std::endl;
  else
    tasklistContent.name = task_list_info.at("name");

  if (!task_list_info.count("content"))
    std::cout << "lose content" << std::endl;
  else
    tasklistContent.content = task_list_info.at("content");

  if (!task_list_info.count("date"))
    std::cout << "lose date" << std::endl;
  else
    tasklistContent.date = task_list_info.at("date");
}

returnCode TaskListsWorker ::Query(const RequestData &data,
                                   TasklistContent &out) {
  // request has empty value
  // TODO: may need a returnCode representing empty request
  if (data.RequestTaskListIsEmpty())
    return ERR_KEY;

  std::map<std::string, std::string> task_list_info;

  // get all available fields
  returnCode ret = db_instance.getTaskListNode(data.user_key, data.tasklist_key,
                                               task_list_info);

  if (ret != SUCCESS)
    return ret;
  Map2Content(task_list_info, out);
  return ret;
}

returnCode TaskListsWorker ::Create(const RequestData &data,
                                    TasklistContent &in,
                                    std::string &outTasklistName) {
  // request has empty value
  // TODO: may need a returnCode representing empty request
  if (data.RequestUserIsEmpty())
    return ERR_KEY;

  std::map<std::string, std::string> task_list_info;
  Content2Map(in, task_list_info);

  int suffix = 0;
  returnCode ret;
  std::string originTasklistName = task_list_info["name"];
  do {
    outTasklistName = Rename(originTasklistName, suffix++);
    task_list_info["name"] = outTasklistName;
    ret = db_instance.createTaskListNode(data.user_key, task_list_info);
  } while (ret == ERR_DUP_NODE);

  return ret;
}

returnCode TaskListsWorker ::Delete(const RequestData &data) {
  // request has empty value
  // TODO: may need a returnCode representing empty request
  if (data.RequestTaskListIsEmpty())
    return ERR_KEY;

  returnCode ret =
      db_instance.deleteTaskListNode(data.user_key, data.tasklist_key);
  return ret;
}

returnCode TaskListsWorker ::Revise(const RequestData &data,
                                    TasklistContent &in) {
  // request has empty value
  // TODO: may need a returnCode representing empty request
  if (data.RequestTaskListIsEmpty())
    return ERR_KEY;

  std::map<std::string, std::string> task_list_info;
  Content2Map(in, task_list_info);
<<<<<<< HEAD

  returnCode ret = db_instance.reviseTaskListNode(
      data.user_key, data.tasklist_key, task_list_info);
  return ret;
}

returnCode
TaskListsWorker ::GetAllTasklist(const RequestData &data,
                                 std::vector<std::string> &outNames) {
  // request has empty value
  if (data.RequestUserIsEmpty())
    return ERR_KEY;

  returnCode ret = db_instance.getAllTaskListNodes(data.user_key, outNames);
=======

  returnCode ret = db_instance.reviseTaskListNode(
      data.user_key, data.tasklist_key, task_list_info);
>>>>>>> 1f9a479b9ed4b4f7693307b4f2ba981ab1dcfe7d
  return ret;
}

bool TaskListsWorker ::Exists(const RequestData &data) {
  TasklistContent out;
  returnCode ret = Query(data, out);
  if (ret == ERR_KEY || ret == ERR_NO_NODE) {
    return false;
  }
  return true;
}