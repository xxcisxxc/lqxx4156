#pragma once

#include <map>
#include <string>

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

  /* method */
  /**
   * @brief Connect to Neo4j DB.
   *
   */
  void connectDB();
  /**
   * @brief Disconnect from Neo4j DB.
   *
   */
  void disconnectDB();
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
  DB(std::string host);
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
  virtual returnCode getUserNode(const std::string &user_pkey,
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
};

/*
unit test:
- each method (mock database?)
*/