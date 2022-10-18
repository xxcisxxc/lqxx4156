#include "DB.h"

DB::DB(std::string host) {}

DB::~DB() {}

returnCode
DB::createUserNode(const std::map<std::string, std::string> &user_info) {
  return SUCCESS;
}

returnCode DB::createTaskListNode(
    const std::string &user_pkey,
    const std::map<std::string, std::string> &task_list_info) {
  return SUCCESS;
}

returnCode
DB::createTaskNode(const std::string &user_pkey,
                   const std::string &task_list_pkey,
                   const std::map<std::string, std::string> &task_info) {
  return SUCCESS;
}

returnCode
DB::reviseUserNode(const std::string &user_pkey,
                   const std::map<std::string, std::string> &user_info) {
  return SUCCESS;
}

returnCode DB::reviseTaskListNode(
    const std::string &user_pkey, const std::string &task_list_pkey,
    const std::map<std::string, std::string> &task_list_info) {
  return SUCCESS;
}

returnCode
DB::reviseTaskNode(const std::string &user_pkey,
                   const std::string &task_list_pkey,
                   const std::string &task_pkey,
                   const std::map<std::string, std::string> &task_info) {
  return SUCCESS;
}

returnCode DB::deleteUserNode(const std::string &user_pkey) { return SUCCESS; }

returnCode DB::deleteTaskListNode(const std::string &user_pkey,
                                  const std::string &task_list_pkey) {
  return SUCCESS;
}

returnCode DB::deleteTaskNode(const std::string &user_pkey,
                              const std::string &task_list_pkey,
                              const std::string &task_pkey) {
  return SUCCESS;
}

returnCode
DB::getUserNode(const std::string &user_pkey,
                std::map<std::string, std::string> &user_info) {
  return SUCCESS;
}

returnCode
DB::getTaskListNode(const std::string &user_pkey,
                    const std::string &task_list_pkey,
                    std::map<std::string, std::string> &task_list_info) {
  return SUCCESS;
}

returnCode
DB::getTaskNode(const std::string &user_pkey, const std::string &task_list_pkey,
                const std::string &task_pkey,
                std::map<std::string, std::string> &task_info) {
  return SUCCESS;
}

void DB::connectDB() {
  // Connect to the database
}

void DB::disconnectDB() {
  // Disconnect to the database
}

std::string DB::executeQuery(std::string &query) {
  // Execute the query
  return "";
}