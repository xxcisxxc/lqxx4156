#pragma once

#include <errno.h>
#include <map>
#include <string>
#include <vector>
// third party library
#include "neo4j-client.h"

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
 * @brief This class connect and interact with neo4j DB.
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
   * @brief neo4j return code of duplicated node
   *
   */
  const std::string error_code_of_dup =
      "Neo.ClientError.Schema.ConstraintValidationFailed";
  /* method */
  /**
   * @brief Create the connection to neo4j DB.
   *
   * @return neo4j_connection_t* The connection to neo4j DB.
   */
  neo4j_connection_t *connectDB();
  /**
   * @brief Close the connection to neo4j DB.
   *
   */
  void closeDB(neo4j_connection_t *connection);
  /**
   * @brief Execute a query.
   *
   * @param query
   * @return neo4j_result_stream_t *: a pointer to a list of results
   */
  neo4j_result_stream_t *executeQuery(const std::string &query,
                                      neo4j_connection_t *connection);
  /**
   * @brief Get Neo4j Client Error Message
   *
   */
  std::string get_Neo4jC_error();
  /**
   * @brief Ensure to create database constraints
   *
   */
  void ensureConstraints();

public:
  /**
   * @brief Construct a new DB object: connect to neo4j DB.
   *
   * @param host The host address of neo4j DB.
   * @param default_connection The connection pool depth
   */
  DB(std::string host, int default_connection = 10);
  /**
   * @brief Destroy the DB object: disconnect from neo4j DB.
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
  virtual returnCode
  createUserNode(const std::map<std::string, std::string> &user_info);
  /**
   * @brief Create a task list node.
   *
   * @param [in] user_pkey primary key of the user node
   * @param [in] task_list_info key: field name, value: field value
   * @return returnCode error message
   */
  virtual returnCode
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
  virtual returnCode
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
  virtual returnCode
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
  virtual returnCode
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
  virtual returnCode
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
  virtual returnCode deleteUserNode(const std::string &user_pkey);
  /**
   * @brief Delete a task list node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @return returnCode error message
   */
  virtual returnCode deleteTaskListNode(const std::string &user_pkey,
                                const std::string &task_list_pkey);
  /**
   * @brief Delete a task node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in] task_pkey task primary key
   * @return returnCode error message
   */
  virtual returnCode deleteTaskNode(const std::string &user_pkey,
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
                         std::map<std::string, std::string> &user_info);
  /**
   * @brief Get a task list node.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [in, out] task_list_info key: field name to request, value: field
   * value to be filled
   * @return returnCode error message
   */
  virtual returnCode
  getTaskListNode(const std::string &user_pkey,
                  const std::string &task_list_pkey,
                  std::map<std::string, std::string> &task_list_info);
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
  virtual returnCode getTaskNode(const std::string &user_pkey,
                         const std::string &task_list_pkey,
                         const std::string &task_pkey,
                         std::map<std::string, std::string> &task_info);
  /**
   * @brief Get all user nodes.
   *
   * @param [out] user_info array of user pkeys
   */
  virtual returnCode getAllUserNodes(std::vector<std::string> &user_info);
  /**
   * @brief Get all task list nodes.
   *
   * @param [in] user_pkey user primary key
   * @param [out] task_list_info array of task list pkeys
   */
  virtual returnCode getAllTaskListNodes(const std::string &user_pkey,
                                 std::vector<std::string> &task_list_info);
  /**
   * @brief Get all task nodes.
   *
   * @param [in] user_pkey user primary key
   * @param [in] task_list_pkey task list primary key
   * @param [out] task_info array of task pkeys
   */
  virtual returnCode getAllTaskNodes(const std::string &user_pkey,
                             const std::string &task_list_pkey,
                             std::vector<std::string> &task_info);
};
