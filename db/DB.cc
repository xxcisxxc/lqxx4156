#include "DB.h"
#include <iostream>

DB::DB(std::string host, std::string dbname) {
  this->host_ = host;
  this->database_ = dbname;

  mongoc_init();
  connectDBTest();
}

DB::~DB() { mongoc_cleanup(); }

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
                const std::map<std::string, std::string> &user_info) {
  return SUCCESS;
}

returnCode
DB::getTaskListNode(const std::string &user_pkey,
                    const std::string &task_list_pkey,
                    const std::map<std::string, std::string> &task_list_info) {
  return SUCCESS;
}

returnCode
DB::getTaskNode(const std::string &user_pkey, const std::string &task_list_pkey,
                const std::string &task_pkey,
                const std::map<std::string, std::string> &task_info) {
  return SUCCESS;
}

void DB::connectDBTest() {
  bson_error_t error;

  mongoc_uri_t *uri = mongoc_uri_new_with_error(host_.c_str(), &error);
  if (!uri) {
    throw std::runtime_error("Connection failed: " +
                             std::string(error.message));
  }
  mongoc_client_t *client = mongoc_client_new_from_uri(uri);
  if (!client) {
    throw std::runtime_error("Connection failed: Unkown error");
  }
  bson_t *b = BCON_NEW("ping", BCON_INT32(1));
  if (!mongoc_client_command_simple(client, database_.c_str(), b, NULL, NULL,
                                    &error)) {
    throw std::runtime_error("Connection failed: " +
                             std::string(error.message));
  }

  bson_destroy(b);
  mongoc_uri_destroy(uri);
  mongoc_client_destroy(client);
}

std::string DB::executeQuery(std::string &query) {
  // Execute the query
  return "";
}