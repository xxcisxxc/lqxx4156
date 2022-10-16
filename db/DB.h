#pragma once

#include <map>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
// third party library
#include <mongoc/mongoc.h>

/**
 * @brief DB class return code
 * 
 */
enum returnCode {
  SUCCESS,
  ERR_UNKNOWN,  // Unknown error
  ERR_KEY,      // No primary key or revise primary key
  ERR_RFIELD,   // No required field (no password, etc.) or revise unmodified
                // field
  ERR_NO_NODE,  // No such node
  ERR_DUP_NODE, // Duplicate node
};

/**
 * @brief This class connect and interact with Neo4j DB.
 *
 */
class DB {
private:
  /* data */
  /**
   * @brief host address
   * 
   */
  std::string host_;
  /**
   * @brief database name
   * 
   */
  std::string database_;
  /* method */
  /**
   * @brief Test the connection to Neo4j DB.
   *
   */
  void connectDBTest();
  /**
   * @brief Execute a query.
   *
   * @param query
   * @return std::string
   */
  std::string executeQuery(std::string &query);

public:
  /**
   * @brief Construct a new DB object: connect to Neo4j DB.
   *
   * @param host
   */
  DB(std::string host, std::string dbname = "test");
  /**
   * @brief Destroy the DB object: disconnect from Neo4j DB.
   *
   */
  ~DB();

  /*
   * All parameters are passed by reference. Therefore, the caller should
   * allocate them even if it is a return value. (Except for the return code)
   */
  /**
   * @brief Create a user node.
   *
   * @param [in] user_info key: field name, value: field value
   * @return returnCode error message
   */
  returnCode
  createUserNode(const std::map<std::string, std::string> &user_info);
  /**
   * @brief Create a task list node.
   *
   * @param [in] user_pkey primary key of the user node
   * @param [in] task_list_info key: field name, value: field value
   * @return returnCode error message
   */
  returnCode
  createTaskListNode(const std::string &user_pkey,
                     const std::map<std::string, std::string> &task_list_info);
  /**
   * @brief Create a task node.
   *
   * @param [in] user_pkey primary key of the user node
   * @param [in] task_list_pkey primary key of the task list node
   * @param [in] task_info key: field name, value: field value
   * @return returnCode error message
   */
  returnCode
  createTaskNode(const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 const std::map<std::string, std::string> &task_info);
  /**
   * @brief Revise a user node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] user_info key: field name, value: field value
   * @return returnCode error message
   */
  returnCode
  reviseUserNode(const std::string &user_pkey,
                 const std::map<std::string, std::string> &user_info);
  /**
   * @brief Revise a task list node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in] task_list_info key: field name, value: field value
   * @return returnCode error message
   */
  returnCode
  reviseTaskListNode(const std::string &user_pkey,
                     const std::string &task_list_pkey,
                     const std::map<std::string, std::string> &task_list_info);
  /**
   * @brief Revise a task node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in] task_pkey task primary key
   * @param [in] task_info key: field name, value: field value
   * @return returnCode error message
   */
  returnCode
  reviseTaskNode(const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 const std::string &task_pkey,
                 const std::map<std::string, std::string> &task_info);
  /**
   * @brief Delete a user node.
   *
   * @param [in] user_pkey user primary key
   * @return returnCode error message
   */
  returnCode deleteUserNode(const std::string &user_pkey);
  /**
   * @brief Delete a task list node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @return returnCode error message
   */
  returnCode deleteTaskListNode(const std::string &user_pkey,
                                const std::string &task_list_pkey);
  /**
   * @brief Delete a task node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in] task_pkey task primary key
   * @return returnCode error message
   */
  returnCode deleteTaskNode(const std::string &user_pkey,
                            const std::string &task_list_pkey,
                            const std::string &task_pkey);
  /**
   * @brief Get a user node.
   *
   * @param [in] user_pkey user primary key
   * @param [in, out] user_info key: field name to request, value: field value
   * to be filled
   * @return returnCode error message
   */
  returnCode getUserNode(const std::string &user_pkey,
                         const std::map<std::string, std::string> &user_info);
  /**
   * @brief Get a task list node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in, out] task_list_info key: field name to request, value: field
   * value to be filled
   * @return returnCode error message
   */
  returnCode
  getTaskListNode(const std::string &user_pkey,
                  const std::string &task_list_pkey,
                  const std::map<std::string, std::string> &task_list_info);
  /**
   * @brief Get a task node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in] task_pkey task primary key
   * @param [in, out] task_info key: field name to request, value: field value
   * to be filled
   * @return returnCode error message
   */
  returnCode getTaskNode(const std::string &user_pkey,
                         const std::string &task_list_pkey,
                         const std::string &task_pkey,
                         const std::map<std::string, std::string> &task_info);
};
