#include "tasklistsWorker.h"
#include "common/utils.h"
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
  if (tasklistContent.visibility != "")
    task_list_info["visibility"] = tasklistContent.visibility;
}

void TaskListsWorker ::Map2Content(
    const std::map<std::string, std::string> &task_list_info,
    TasklistContent &tasklistContent) {
  if (task_list_info.count("name"))
    tasklistContent.name = task_list_info.at("name");

  if (task_list_info.count("content"))
    tasklistContent.content = task_list_info.at("content");

  if (task_list_info.count("date"))
    tasklistContent.date = task_list_info.at("date");

  if (task_list_info.count("visibility"))
    tasklistContent.visibility = task_list_info.at("visibility");
}

returnCode TaskListsWorker ::Query(const RequestData &data,
                                   TasklistContent &out) {
  // request has empty value
  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db_instance.checkAccess(data.other_user_key, data.user_key,
                                             data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
  }

  // can access
  std::map<std::string, std::string> task_list_info;

  // get all available fields
  returnCode ret = db_instance.getTaskListNode(
      data.other_user_key.empty() ? data.user_key : data.other_user_key,
      data.tasklist_key, task_list_info);

  if (ret != SUCCESS)
    return ret;
  Map2Content(task_list_info, out);
  return ret;
}

returnCode TaskListsWorker ::Create(const RequestData &data,
                                    TasklistContent &in,
                                    std::string &outTasklistName) {
  // request has empty value
  if (data.RequestUserIsEmpty())
    return ERR_RFIELD;

  if (!in.visibility.empty() && in.visibility != "public" &&
      in.visibility != "shared" && in.visibility != "private")
    return ERR_FORMAT;

  if (!in.date.empty() && !Common::IsDate(in.date))
    return ERR_FORMAT;

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
  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;

  returnCode ret =
      db_instance.deleteTaskListNode(data.user_key, data.tasklist_key);
  return ret;
}

returnCode TaskListsWorker ::Revise(const RequestData &data,
                                    TasklistContent &in) {
  // request has empty value
  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;

  if (!in.visibility.empty() && in.visibility != "public" &&
      in.visibility != "shared" && in.visibility != "private")
    return ERR_FORMAT;

  if (!in.date.empty() && !Common::IsDate(in.date))
    return ERR_FORMAT;

  if (!data.other_user_key.empty()) {
    bool permission = false;
    returnCode ret = db_instance.checkAccess(data.other_user_key, data.user_key,
                                             data.tasklist_key, permission);
    if (ret != SUCCESS)
      // no permission
      return ret;
    if (!permission) {
      // read permission cannot revise
      return ERR_ACCESS;
    }
  }

  // can access
  std::map<std::string, std::string> task_list_info;
  Content2Map(in, task_list_info);
  if (!data.other_user_key.empty() && task_list_info.count("visibility")) {
    // do not let user edit the visibility of another user's tasklist
    return ERR_REVISE;
  }
  returnCode ret = db_instance.reviseTaskListNode(
      data.other_user_key.empty() ? data.user_key : data.other_user_key,
      data.tasklist_key, task_list_info);
  return ret;
}

returnCode
TaskListsWorker ::GetAllTasklist(const RequestData &data,
                                 std::vector<std::string> &outNames) {
  // request has empty value
  if (data.RequestUserIsEmpty())
    return ERR_RFIELD;

  returnCode ret = db_instance.getAllTaskListNodes(data.user_key, outNames);
  return ret;
}

returnCode
TaskListsWorker ::GetAllAccessTaskList(const RequestData &data,
                                       std::vector<shareInfo> &out_list) {
  if (data.RequestUserIsEmpty())
    return ERR_RFIELD;

  std::map<std::pair<std::string, std::string>, bool> list_accesses;
  returnCode ret = db_instance.allAccess(data.user_key, list_accesses);
  if (ret != SUCCESS)
    return ret;

  for (auto it : list_accesses) {
    shareInfo sh;
    sh.user_name = it.first.first;
    sh.task_list_name = it.first.second;
    sh.permission = it.second;
    out_list.push_back(sh);
  }

  return ret;
}

returnCode TaskListsWorker ::GetVisibility(const RequestData &data,
                                           std::string &visibility) {
  // request has empty value
  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;

  // get only visibility field
  std::map<std::string, std::string> task_list_info;
  task_list_info["visibility"];
  returnCode ret = db_instance.getTaskListNode(data.user_key, data.tasklist_key,
                                               task_list_info);

  if (ret != SUCCESS)
    return ret;

  if (task_list_info.count("visibility")) {
    visibility = task_list_info.at("visibility");
  }
  return ret;
}

returnCode TaskListsWorker ::GetAllGrantTaskList(
    const RequestData &data, std::vector<shareInfo> &out_list, bool &isPublic) {
  // request has empty value
  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;

  returnCode ret;
  // if the tasklist is public, don't call allGrant
  std::string visibility;
  ret = GetVisibility(data, visibility);

  if (ret != SUCCESS) {
    isPublic = false;
    return ret;
  } else if (visibility == "public") {
    isPublic = true;
    return SUCCESS;
  } else if (visibility == "private" || visibility == "") {
    isPublic = false;
    return ERR_ACCESS;
  } else {
    isPublic = false;
  }

  std::map<std::string, bool> list_grants;
  ret = db_instance.allGrant(data.user_key, data.tasklist_key, list_grants);
  if (ret != SUCCESS)
    return ret;

  for (auto it : list_grants) {
    shareInfo sh;
    sh.user_name = it.first;
    sh.permission = it.second;
    out_list.push_back(sh);
  }

  return ret;
}

returnCode
TaskListsWorker ::ReviseGrantTaskList(const RequestData &data,
                                      std::vector<shareInfo> &in_list,
                                      std::string &errUser) {

  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;

  returnCode ret;
  std::string visibility;

  // we can only grant permission to shared tasklist
  // also check whether the tasklist exists
  ret = GetVisibility(data, visibility);
  if (ret != SUCCESS || visibility != "shared")
    return ERR_ACCESS;

  for (int i = 0; i < in_list.size(); i++) {
    // 这里应该要验证 dst_user_key 是否存在

    // ERR_NO_NODE

    // add grant
    ret = db_instance.addAccess(data.user_key, in_list[i].user_name,
                                data.tasklist_key, in_list[i].permission);
    if (ret != SUCCESS) {
      errUser = in_list[i].user_name;
      return ret;
    }
  }

  return ret;
}

returnCode
TaskListsWorker ::RemoveGrantTaskList(const RequestData &data) {

  if (data.RequestTaskListIsEmpty())
    return ERR_RFIELD;
  
  returnCode ret;
  std::string visibility;

  // we can only grant permission to shared tasklist
  // also check whether the tasklist exists
  ret = GetVisibility(data, visibility);
  if (ret != SUCCESS || visibility != "shared")
    return ERR_ACCESS;
  
  ret = db_instance.removeAccess(data.user_key, data.other_user_key, data.tasklist_key);
  return ret;
}

returnCode TaskListsWorker ::GetAllPublicTaskList(
    std::vector<std::pair<std::string, std::string>> &out_list) {
  returnCode ret = db_instance.getAllPublic(out_list);
  return ret;
}

bool TaskListsWorker ::Exists(const RequestData &data) {
  TasklistContent out;
  returnCode ret = Query(data, out);
  if (ret == SUCCESS) {
    return true;
  }
  return false;
}