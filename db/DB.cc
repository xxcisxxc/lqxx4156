#include "DB.h"
#include "common/errorCode.h"

DB::DB(std::string host) {
  this->host_ = host; // hardcode

  // For unit test
  if (host == "testhost") {
    return;
  }

  // Called before neo4j client driver is used.
  neo4j_client_init();
  // Test the connection.
  // Copied from https://neo4j-client.net/
  {
    neo4j_connection_t *connection = connectDB();
    if (connection == NULL) {
      throw std::runtime_error(get_Neo4jC_error());
    }

    neo4j_result_stream_t *results =
        neo4j_run(connection, "RETURN 'hello world'", neo4j_null);
    if (!results) {
      closeDB(connection);
      throw std::runtime_error(get_Neo4jC_error());
    }

    neo4j_result_t *result = neo4j_fetch_next(results);
    if (result == NULL) {
      neo4j_close_results(results);
      closeDB(connection);
      throw std::runtime_error(get_Neo4jC_error());
    }

    neo4j_close_results(results);
    closeDB(connection);
  }

  ensureConstraints();
}

DB::~DB() {
  // closeDB();
  neo4j_client_cleanup();
}

returnCode
DB::createUserNode(const std::map<std::string, std::string> &user_info) {
  // Check Primary Key - user_pkey
  if (user_info.find("email") == user_info.end()) {
    return ERR_KEY;
  }
  // Check Password existence
  if (user_info.find("passwd") == user_info.end()) {
    return ERR_RFIELD;
  }

  neo4j_connection_t *connection = connectDB();

  // Create node
  std::string query = "CREATE (n:User {";
  for (auto it = user_info.begin(); it != user_info.end(); it++) {
    query += it->first + ": '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += "})";
  neo4j_result_stream_t *results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    if (error_code_of_dup == neo4j_error_code(results)) {
      neo4j_close_results(results);
      closeDB(connection);
      return ERR_DUP_NODE;
    } else {
      neo4j_close_results(results);
      closeDB(connection);
      return ERR_UNKNOWN;
    }
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::createTaskListNode(
    const std::string &user_pkey,
    const std::map<std::string, std::string> &task_list_info) {
  // Check Primary Key - task_list_pkey exists
  if (task_list_info.find("name") == task_list_info.end()) {
    return ERR_KEY;
  }

  neo4j_connection_t *connection = connectDB();

  // Check Foreign Key - user_pkey exsits
  std::string query =
      "MATCH (n:User) WHERE n.email = '" + user_pkey + "' RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  if (neo4j_fetch_next(results) == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Create node TaskList
  std::map<std::string, std::string> revised_info = task_list_info;
  revised_info["name"] = revised_info["name"];
  revised_info["user"] = user_pkey;
  if (task_list_info.find("visibility") == task_list_info.end()) {
    revised_info["visibility"] = "private";
  }
  query = "CREATE (n:TaskList {";
  for (auto it = revised_info.begin(); it != revised_info.end(); it++) {
    query += it->first + ": '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += "})";
  results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    if (error_code_of_dup == neo4j_error_code(results)) {
      neo4j_close_results(results);
      closeDB(connection);
      return ERR_DUP_NODE;
    } else {
      neo4j_close_results(results);
      closeDB(connection);
      return ERR_UNKNOWN;
    }
  }

  // Create relationship between User and TaskList
  std::string user_node = "(a:User {email: '" + user_pkey + "'})";
  std::string list_node = "(b:TaskList {name: '" + revised_info["name"] +
                          "', user: '" + revised_info["user"] + "'})";
  query = "MATCH " + user_node + ", " + list_node + " MERGE (a)-[r:Owns]->(b)";
  results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::createTaskNode(const std::string &user_pkey,
                   const std::string &task_list_pkey,
                   const std::map<std::string, std::string> &task_info) {
  // Check Primary Key - task_pkey exists
  if (task_info.find("name") == task_info.end()) {
    return ERR_KEY;
  }

  neo4j_connection_t *connection = connectDB();

  // Check Foreign Key - user_pkey exsits
  std::string query =
      "MATCH (n:User) WHERE n.email = '" + user_pkey + "' RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  if (neo4j_fetch_next(results) == NULL) {
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check Foreign Key - task_list_pkey exsits
  query = "MATCH (n:TaskList) WHERE n.name = '" + task_list_pkey +
          "' AND n.user = '" + user_pkey + "' RETURN n";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  if (neo4j_fetch_next(results) == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  std::map<std::string, std::string> revised_info = task_info;
  revised_info["name"] = revised_info["name"];
  revised_info["list"] = task_list_pkey;
  revised_info["user"] = user_pkey;
  // Create node Task
  query = "CREATE (n:Task {";
  for (auto it = revised_info.begin(); it != revised_info.end(); it++) {
    query += it->first + ": '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += "})";
  results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    if (error_code_of_dup == neo4j_error_code(results)) {
      neo4j_close_results(results);
      closeDB(connection);
      return ERR_DUP_NODE;
    } else {
      neo4j_close_results(results);
      closeDB(connection);
      return ERR_UNKNOWN;
    }
  }

  // Create relationship between TaskList and Task
  std::string list_node = "(a:TaskList {name: '" + task_list_pkey +
                          "', user: '" + user_pkey + "'})";
  std::string task_node = "(b:Task {name: '" + revised_info["name"] +
                          "', list: '" + task_list_pkey + "', user: '" +
                          user_pkey + "'})";
  query =
      "MATCH " + list_node + ", " + task_node + " MERGE (a)-[r:Contains]->(b)";
  results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::reviseUserNode(const std::string &user_pkey,
                   const std::map<std::string, std::string> &user_info) {
  // Check Primary Key unmodified - user_pkey
  if (user_info.find("email") != user_info.end()) {
    return ERR_KEY;
  }
  // Check info not empty
  if (user_info.empty()) {
    return ERR_RFIELD;
  }

  neo4j_connection_t *connection = connectDB();

  // Modify node User
  std::string query = "MATCH (n:User {email: '" + user_pkey + "'}) SET ";
  for (auto it = user_info.begin(); it != user_info.end(); it++) {
    query += "n." + it->first + " = '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += " RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  if (neo4j_fetch_next(results) == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::reviseTaskListNode(
    const std::string &user_pkey, const std::string &task_list_pkey,
    const std::map<std::string, std::string> &task_list_info) {
  // Check Primary Key unmodified - task_list_pkey
  if (task_list_info.find("name") != task_list_info.end()) {
    return ERR_KEY;
  }
  // Check info not empty
  if (task_list_info.empty()) {
    return ERR_RFIELD;
  }

  neo4j_connection_t *connection = connectDB();

  // Modify node TaskList
  std::string query = "MATCH (n:TaskList {name: '" + task_list_pkey +
                      "', user: '" + user_pkey + "'}) SET ";
  for (auto it = task_list_info.begin(); it != task_list_info.end(); it++) {
    query += "n." + it->first + " = '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += " RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  if (neo4j_fetch_next(results) == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::reviseTaskNode(const std::string &user_pkey,
                   const std::string &task_list_pkey,
                   const std::string &task_pkey,
                   const std::map<std::string, std::string> &task_info) {
  // Check Primary Key unmodified - task_pkey
  if (task_info.find("name") != task_info.end()) {
    return ERR_KEY;
  }
  // Check info not empty
  if (task_info.empty()) {
    return ERR_RFIELD;
  }

  neo4j_connection_t *connection = connectDB();

  // Modify node Task
  std::string query = "MATCH (n:Task {name: '" + task_pkey + "', list: '" +
                      task_list_pkey + "', user: '" + user_pkey + "'}) SET ";
  for (auto it = task_info.begin(); it != task_info.end(); it++) {
    query += "n." + it->first + " = '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += " RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);

  // Check result
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  if (neo4j_fetch_next(results) == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteUserNode(const std::string &user_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Delete node User
  std::string query =
      "MATCH (a:User {email: '" + user_pkey +
      "'})-[r:Owns]->(b:TaskList)-[s:Contains]->(c:Task) DETACH DELETE s, c";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  query = "MATCH (a:User {email: '" + user_pkey +
          "'})-[r:Owns]->(b:TaskList) DETACH DELETE r, b";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  query = "MATCH (a:User {email: '" + user_pkey + "'}) DETACH DELETE a";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteTaskListNode(const std::string &user_pkey,
                                  const std::string &task_list_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Delete node TaskList
  std::string query = "MATCH (a:TaskList {name: '" + task_list_pkey +
                      "', user: '" + user_pkey +
                      "'})-[r:Contains]->(b:Task) DETACH DELETE r, b";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  query = "MATCH (a:TaskList {name: '" + task_list_pkey + "', user: '" +
          user_pkey + "'}) DETACH DELETE a";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteTaskNode(const std::string &user_pkey,
                              const std::string &task_list_pkey,
                              const std::string &task_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Delete node Task
  std::string query = "MATCH (a:Task {name: '" + task_pkey + "', list: '" +
                      task_list_pkey + "', user: '" + user_pkey +
                      "'}) DETACH DELETE a";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getUserNode(const std::string &user_pkey,
                           std::map<std::string, std::string> &user_info) {
  neo4j_connection_t *connection = connectDB();

  // Get node User
  std::string query = "MATCH (n:User {email: '" + user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Extract node info
  neo4j_value_t node = neo4j_result_field(result, 0);
  neo4j_value_t value = neo4j_node_properties(node);
  char buf[1024];
  // Empty: return all fields / Not empty: return specified fields
  if (user_info.empty()) {
    for (int i = 0; i < neo4j_map_size(value); i++) {
      const neo4j_map_entry_t *kv = neo4j_map_getentry(value, i);
      neo4j_value_t key = kv->key;
      neo4j_value_t val = kv->value;
      neo4j_tostring(key, buf, 1024);
      std::string key_str(buf);
      key_str.pop_back();
      key_str.erase(0, 1);
      neo4j_tostring(val, buf, 1024);
      std::string val_str(buf);
      val_str.pop_back();
      val_str.erase(0, 1);
      user_info[key_str] = val_str;
    }
  } else {
    for (auto it = user_info.begin(); it != user_info.end(); it++) {
      neo4j_value_t field_value = neo4j_map_get(value, it->first.c_str());
      if (neo4j_is_null(field_value)) {
        it->second = "";
      } else {
        neo4j_tostring(field_value, buf, sizeof(buf));
        std::string value_str(buf);
        value_str.pop_back();
        value_str.erase(0, 1);
        it->second = value_str;
      }
    }
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::getTaskListNode(const std::string &user_pkey,
                    const std::string &task_list_pkey,
                    std::map<std::string, std::string> &task_list_info) {
  neo4j_connection_t *connection = connectDB();

  // Get node TaskList
  std::string query = "MATCH (n:TaskList {name: '" + task_list_pkey +
                      "', user: '" + user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Extract node info
  neo4j_value_t node = neo4j_result_field(result, 0);
  neo4j_value_t value = neo4j_node_properties(node);
  char buf[1024];
  // Empty: return all fields / Not empty: return specified fields
  if (task_list_info.empty()) {
    for (int i = 0; i < neo4j_map_size(value); i++) {
      const neo4j_map_entry_t *kv = neo4j_map_getentry(value, i);
      neo4j_value_t key = kv->key;
      neo4j_value_t val = kv->value;
      neo4j_tostring(key, buf, 1024);
      std::string key_str(buf);
      key_str.pop_back();
      key_str.erase(0, 1);
      neo4j_tostring(val, buf, 1024);
      std::string val_str(buf);
      val_str.pop_back();
      val_str.erase(0, 1);
      task_list_info[key_str] = val_str;
    }
  } else {
    for (auto it = task_list_info.begin(); it != task_list_info.end(); it++) {
      neo4j_value_t field_value = neo4j_map_get(value, it->first.c_str());
      if (neo4j_is_null(field_value)) {
        it->second = "";
      } else {
        neo4j_tostring(field_value, buf, sizeof(buf));
        std::string value_str(buf);
        value_str.pop_back();
        value_str.erase(0, 1);
        it->second = value_str;
      }
    }
  }
  // Delete user field
  task_list_info.erase("user");

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getTaskNode(const std::string &user_pkey,
                           const std::string &task_list_pkey,
                           const std::string &task_pkey,
                           std::map<std::string, std::string> &task_info) {
  neo4j_connection_t *connection = connectDB();

  // Get node Task
  std::string query = "MATCH (n:Task {name: '" + task_pkey + "', list: '" +
                      task_list_pkey + "', user: '" + user_pkey +
                      "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Extract node info
  neo4j_value_t node = neo4j_result_field(result, 0);
  neo4j_value_t value = neo4j_node_properties(node);
  char buf[1024];
  // Empty: return all fields / Not empty: return specified fields
  if (task_info.empty()) {
    for (int i = 0; i < neo4j_map_size(value); i++) {
      const neo4j_map_entry_t *kv = neo4j_map_getentry(value, i);
      neo4j_value_t key = kv->key;
      neo4j_value_t val = kv->value;
      neo4j_tostring(key, buf, 1024);
      std::string key_str(buf);
      key_str.pop_back();
      key_str.erase(0, 1);
      neo4j_tostring(val, buf, 1024);
      std::string val_str(buf);
      val_str.pop_back();
      val_str.erase(0, 1);
      task_info[key_str] = val_str;
    }
  } else {
    for (auto it = task_info.begin(); it != task_info.end(); it++) {
      neo4j_value_t field_value = neo4j_map_get(value, it->first.c_str());
      if (neo4j_is_null(field_value)) {
        it->second = "";
      } else {
        neo4j_tostring(field_value, buf, sizeof(buf));
        std::string value_str(buf);
        value_str.pop_back();
        value_str.erase(0, 1);
        it->second = value_str;
      }
    }
  }
  // Delete user and list field
  task_info.erase("user");
  task_info.erase("list");

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getAllUserNodes(std::vector<std::string> &user_info) {
  neo4j_connection_t *connection = connectDB();

  // Clear vector
  user_info.clear();

  // Get all nodes User
  std::string query = "MATCH (n:User) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Extract returned info
  neo4j_result_t *result;
  while ((result = neo4j_fetch_next(results)) != NULL) {
    neo4j_value_t node = neo4j_result_field(result, 0);
    neo4j_value_t value = neo4j_node_properties(node);
    char buf[1024];
    neo4j_value_t field_value = neo4j_map_get(value, "email");
    neo4j_tostring(field_value, buf, sizeof(buf));
    std::string value_str(buf);
    value_str.pop_back();
    value_str.erase(0, 1);
    user_info.push_back(value_str);
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getAllTaskListNodes(const std::string &user_pkey,
                                   std::vector<std::string> &task_list_info) {
  neo4j_connection_t *connection = connectDB();

  // Clear vector
  task_list_info.clear();

  // Check User node exists
  std::string query = "MATCH (n:User {email: '" + user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Get all nodes TaskList
  query = "MATCH (n:User {email: '" + user_pkey + "'})-[:Owns]->(m) RETURN m";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Extract returned info
  while ((result = neo4j_fetch_next(results)) != NULL) {
    neo4j_value_t node = neo4j_result_field(result, 0);
    neo4j_value_t value = neo4j_node_properties(node);
    char buf[1024];
    neo4j_value_t field_value = neo4j_map_get(value, "name");
    neo4j_tostring(field_value, buf, sizeof(buf));
    std::string value_str(buf);
    value_str.pop_back();
    value_str.erase(0, 1);
    task_list_info.push_back(value_str);
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getAllTaskNodes(const std::string &user_pkey,
                               const std::string &task_list_pkey,
                               std::vector<std::string> &task_info) {
  neo4j_connection_t *connection = connectDB();

  // Clear vector
  task_info.clear();

  // Check User node exists
  std::string query = "MATCH (n:User {email: '" + user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList node exists
  query = "MATCH (n:TaskList {name: '" + task_list_pkey + "', user: '" +
          user_pkey + "'}) RETURN n";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Get all nodes Task
  query = "MATCH (n:TaskList {name: '" + task_list_pkey + "', user: '" +
          user_pkey + "'})-[:Contains]->(m) RETURN m";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Extract returned info
  while ((result = neo4j_fetch_next(results)) != NULL) {
    neo4j_value_t node = neo4j_result_field(result, 0);
    neo4j_value_t value = neo4j_node_properties(node);
    char buf[1024];
    neo4j_value_t field_value = neo4j_map_get(value, "name");
    neo4j_tostring(field_value, buf, sizeof(buf));
    std::string value_str(buf);
    value_str.pop_back();
    value_str.erase(0, 1);
    task_info.push_back(value_str);
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::addAccess(const std::string &src_user_pkey,
                         const std::string &dst_user_pkey,
                         const std::string &task_list_pkey,
                         const bool read_write) {
  neo4j_connection_t *connection = connectDB();

  // Check User node exists - src
  std::string query =
      "MATCH (n:User {email: '" + src_user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check User node exists - dst
  query = "MATCH (n:User {email: '" + dst_user_pkey + "'}) RETURN n";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList node exists
  query = "MATCH (n:TaskList {name: '" + task_list_pkey + "', user: '" +
          src_user_pkey + "'}) RETURN n.visibility";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList visibility
  neo4j_value_t value = neo4j_result_field(result, 0);
  char buf[16];
  neo4j_tostring(value, buf, sizeof(buf));
  std::string value_str(buf);
  value_str.pop_back();
  value_str.erase(0, 1);
  if (value_str == "private") {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_ACCESS;
  }

  // Create or Modify access relationship
  query = "MATCH (n:User {email: '" + dst_user_pkey +
          "'}), (m:TaskList {name: '" + task_list_pkey + "', user: '" +
          src_user_pkey + "'}) MERGE (n)-[r:Access]->(m) SET r.read_write = " +
          std::to_string(read_write) + " RETURN r";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::checkAccess(const std::string &src_user_pkey,
                           const std::string &dst_user_pkey,
                           const std::string &task_list_pkey,
                           bool &read_write) {
  if (src_user_pkey == dst_user_pkey) {
    read_write = true;
    return SUCCESS;
  }

  neo4j_connection_t *connection = connectDB();

  // Check User node exists - src
  std::string query =
      "MATCH (n:User {email: '" + src_user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check User node exists - dst
  query = "MATCH (n:User {email: '" + dst_user_pkey + "'}) RETURN n";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList node exists
  query = "MATCH (n:TaskList {name: '" + task_list_pkey + "', user: '" +
          src_user_pkey + "'}) RETURN n.visibility";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList visibility
  neo4j_value_t value = neo4j_result_field(result, 0);
  char buf[16];
  neo4j_tostring(value, buf, sizeof(buf));
  std::string value_str(buf);
  value_str.pop_back();
  value_str.erase(0, 1);
  if (value_str == "private") {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_ACCESS;
  } else if (value_str == "public") {
    read_write = true;
    neo4j_close_results(results);
    closeDB(connection);
    return SUCCESS;
  }

  // Check access relationship
  query = "MATCH (n:User {email: '" + dst_user_pkey +
          "'})-[r:Access]->(m:TaskList {name: '" + task_list_pkey +
          "', user: '" + src_user_pkey + "'}) RETURN r.read_write";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_ACCESS;
  }
  value = neo4j_result_field(result, 0);
  neo4j_tostring(value, buf, sizeof(buf));
  read_write = bool(buf[0] - '0');

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::removeAccess(const std::string &src_user_pkey,
                            const std::string &dst_user_pkey,
                            const std::string &task_list_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Remove access relationship
  std::string query = "MATCH (n:User {email: '" + dst_user_pkey +
                      "'})-[r:Access]->(m:TaskList {name: '" + task_list_pkey +
                      "', user: '" + src_user_pkey + "'}) DELETE r";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::allAccess(
    const std::string &dst_user_pkey,
    std::map<std::pair<std::string, std::string>, bool> &list_accesses) {
  neo4j_connection_t *connection = connectDB();

  // clear map
  list_accesses.clear();

  // Check User node exists - dst
  std::string query =
      "MATCH (n:User {email: '" + dst_user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }

  // Get all TaskList nodes
  query = "MATCH (n:User {email: '" + dst_user_pkey +
          "'})-[r:Access]->(m:TaskList) RETURN m.user, m.name, m.visibility, "
          "r.read_write";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  while ((result = neo4j_fetch_next(results)) != NULL) {
    // Check TaskList visibility
    neo4j_value_t value = neo4j_result_field(result, 2);
    char buf[1024];
    neo4j_tostring(value, buf, sizeof(buf));
    std::string value_str(buf);
    value_str.pop_back();
    value_str.erase(0, 1);
    if (value_str == "private") {
      continue;
    }
    // Check access relationship
    value = neo4j_result_field(result, 1);
    neo4j_tostring(value, buf, sizeof(buf));
    std::string task_list_pkey(buf);
    task_list_pkey.pop_back();
    task_list_pkey.erase(0, 1);
    value = neo4j_result_field(result, 0);
    neo4j_tostring(value, buf, sizeof(buf));
    std::string user_pkey(buf);
    user_pkey.pop_back();
    user_pkey.erase(0, 1);
    value = neo4j_result_field(result, 3);
    neo4j_tostring(value, buf, sizeof(buf));
    bool read_write = (value_str == "public") ? true : bool(buf[0] - '0');
    list_accesses[{user_pkey, task_list_pkey}] = read_write;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::allGrant(const std::string &src_user_pkey,
                        const std::string &task_list_pkey,
                        std::map<std::string, bool> &list_grants) {
  neo4j_connection_t *connection = connectDB();

  // clear map
  list_grants.clear();

  // Check User node exists - src
  std::string query =
      "MATCH (n:User {email: '" + src_user_pkey + "'}) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList node exists
  query = "MATCH (n:TaskList {name: '" + task_list_pkey + "', user: '" +
          src_user_pkey + "'}) RETURN n.visibility";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  result = neo4j_fetch_next(results);
  if (result == NULL) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_NO_NODE;
  }
  // Check TaskList visibility
  neo4j_value_t value = neo4j_result_field(result, 0);
  char buf[1024];
  neo4j_tostring(value, buf, sizeof(buf));
  std::string value_str(buf);
  value_str.pop_back();
  value_str.erase(0, 1);
  if (value_str != "shared") {
    neo4j_close_results(results);
    closeDB(connection);
    return SUCCESS;
  }

  // Get all grants
  query = "MATCH (n:User)-[r:Access]->(m:TaskList {name: '" + task_list_pkey +
          "', user: '" + src_user_pkey + "'}) RETURN n.email, r.read_write";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  while ((result = neo4j_fetch_next(results)) != NULL) {
    // Check access relationship
    neo4j_value_t value = neo4j_result_field(result, 0);
    char buf[1024];
    neo4j_tostring(value, buf, sizeof(buf));
    std::string user_pkey(buf);
    user_pkey.pop_back();
    user_pkey.erase(0, 1);
    value = neo4j_result_field(result, 1);
    neo4j_tostring(value, buf, sizeof(buf));
    bool read_write = bool(buf[0] - '0');
    list_grants[user_pkey] = read_write;
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::getAllPublic(std::vector<std::pair<std::string, std::string>> &user_list) {
  neo4j_connection_t *connection = connectDB();

  // clear vector
  user_list.clear();

  // Get all public TaskList nodes
  std::string query =
      "MATCH (n:TaskList) WHERE n.visibility = 'public' RETURN n.user, n.name";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_result_t *result;
  while ((result = neo4j_fetch_next(results)) != NULL) {
    // Get TaskList info
    neo4j_value_t value = neo4j_result_field(result, 0);
    char buf[1024];
    neo4j_tostring(value, buf, sizeof(buf));
    std::string user_pkey(buf);
    user_pkey.pop_back();
    user_pkey.erase(0, 1);
    value = neo4j_result_field(result, 1);
    neo4j_tostring(value, buf, sizeof(buf));
    std::string task_list_pkey(buf);
    task_list_pkey.pop_back();
    task_list_pkey.erase(0, 1);
    user_list.push_back({user_pkey, task_list_pkey});
  }

  // Success
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteEverything(void) {
  neo4j_connection_t *connection = connectDB();
  std::string query = "MATCH (n) DETACH DELETE n";

  neo4j_result_stream_t *results = executeQuery(query, connection);

  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }

  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

neo4j_connection_t *DB::connectDB() {
  neo4j_connection_t *connection =
      neo4j_connect(host_.c_str(), NULL, NEO4J_INSECURE);
  if (!connection) {
    throw std::runtime_error("Connection failed");
  }
  return connection;
}

void DB::closeDB(neo4j_connection_t *connection) { neo4j_close(connection); }

neo4j_result_stream_t *DB::executeQuery(const std::string &query,
                                        neo4j_connection_t *connection) {
  // Execute the query
  neo4j_result_stream_t *results =
      neo4j_run(connection, query.c_str(), neo4j_null);
  return results;
}

void DB::ensureConstraints() {
  // Create constraints for User_pkey
  std::string query = "CREATE CONSTRAINT User_pkey IF NOT EXISTS FOR (n:User) "
                      "REQUIRE n.email IS UNIQUE";
  neo4j_connection_t *connection = connectDB();
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    closeDB(connection);
    throw std::runtime_error(get_Neo4jC_error());
  }
  // Create constraints for TaskList_pkey
  query = "CREATE CONSTRAINT TaskList_pkey IF NOT EXISTS FOR (n:TaskList) "
          "REQUIRE (n.name, n.user) IS UNIQUE";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    closeDB(connection);
    throw std::runtime_error(get_Neo4jC_error());
  }
  // Create constraints for Task_pkey
  query = "CREATE CONSTRAINT Task_pkey IF NOT EXISTS FOR (n:Task) "
          "REQUIRE (n.name, n.list, n.user) IS UNIQUE";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    closeDB(connection);
    throw std::runtime_error(get_Neo4jC_error());
  }
  closeDB(connection);
  neo4j_close_results(results);
}

std::string DB::get_Neo4jC_error() {
  return std::string(neo4j_strerror(errno, NULL, 0));
}