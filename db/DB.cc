#include "DB.h"

DB::DB(std::string host) {
  this->host_ = host;

  // this is just for unit test
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
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::createTaskListNode(
    const std::string &user_pkey,
    const std::map<std::string, std::string> &task_list_info) {
  // Check Primary Key - task_list_pkey
  if (task_list_info.find("name") == task_list_info.end()) {
    return ERR_KEY;
  }

  neo4j_connection_t *connection = connectDB();

  // Check Foreign Key - user_pkey
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

  std::map<std::string, std::string> revised_info = task_list_info;
  revised_info["name"] = revised_info["name"];
  revised_info["user"] = user_pkey;
  // Create node
  query = "CREATE (n:TaskList {";
  for (auto it = revised_info.begin(); it != revised_info.end(); it++) {
    query += it->first + ": '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += "})";
  results = executeQuery(query, connection);
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
  // Create relationship
  std::string user_node = "(a:User {email: '" + user_pkey + "'})"
  std::string task_node = "(b:TaskList {name: '" + revised_info["name"] + "', user: '" + revised_info["user"] + "'})"
  query = "MATCH " + user_node + ", " + task_node + " MERGE (a)-[r:Owns]->(b)";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::createTaskNode(const std::string &user_pkey,
                   const std::string &task_list_pkey,
                   const std::map<std::string, std::string> &task_info) {
  // Check Primary Key - task_pkey
  if (task_info.find("name") == task_info.end()) {
    return ERR_KEY;
  }

  neo4j_connection_t *connection = connectDB();

  // Check Foreign Key - user_pkey
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
  // Check Foreign Key - task_list_pkey
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  query =
      "MATCH (n:TaskList) WHERE n.name = '" + revised_list_pkey + "' RETURN n";
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
  revised_info["name"] = revised_info["name"] + "#" + revised_list_pkey;
  // Create node
  query = "CREATE (n:Task {";
  for (auto it = revised_info.begin(); it != revised_info.end(); it++) {
    query += it->first + ": '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += "})";
  results = executeQuery(query, connection);
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
  // Create relationship
  query = "MATCH (a:TaskList {name: '" + revised_list_pkey +
          "'}), (b:Task {name: '" + revised_info["name"] +
          "'}) MERGE (a)-[r:Contains]->(b)";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
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

  // Modify node
  std::string query = "MATCH (n:User {email: '" + user_pkey + "'}) SET ";
  for (auto it = user_info.begin(); it != user_info.end(); it++) {
    query += "n." + it->first + " = '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += " RETURN n";
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

  // Modify node
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  std::string query =
      "MATCH (n:TaskList {name: '" + revised_list_pkey + "'}) SET ";
  for (auto it = task_list_info.begin(); it != task_list_info.end(); it++) {
    query += "n." + it->first + " = '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += " RETURN n";
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

  // Modify node
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  std::string revised_task_pkey = task_pkey + "#" + revised_list_pkey;
  std::string query = "MATCH (n:Task {name: '" + revised_task_pkey + "'}) SET ";
  for (auto it = task_info.begin(); it != task_info.end(); it++) {
    query += "n." + it->first + " = '" + it->second + "', ";
  }
  query.pop_back();
  query.pop_back();
  query += " RETURN n";
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
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteUserNode(const std::string &user_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Delete node
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
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteTaskListNode(const std::string &user_pkey,
                                  const std::string &task_list_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Delete node
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  std::string query = "MATCH (n:TaskList {name: '" + revised_list_pkey +
                      "'})-[r:Contains]->(m:Task) DETACH DELETE r, m";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  query = "MATCH (n:TaskList {name: '" + revised_list_pkey +
          "'}) DETACH DELETE "
          "n";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::deleteTaskNode(const std::string &user_pkey,
                              const std::string &task_list_pkey,
                              const std::string &task_pkey) {
  neo4j_connection_t *connection = connectDB();

  // Delete node
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  std::string revised_task_pkey = task_pkey + "#" + revised_list_pkey;
  std::string query =
      "MATCH (n:Task {name: '" + revised_task_pkey + "'}) DETACH DELETE n";
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

returnCode DB::getUserNode(const std::string &user_pkey,
                           std::map<std::string, std::string> &user_info) {
  neo4j_connection_t *connection = connectDB();

  // Get node
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
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode
DB::getTaskListNode(const std::string &user_pkey,
                    const std::string &task_list_pkey,
                    std::map<std::string, std::string> &task_list_info) {
  neo4j_connection_t *connection = connectDB();

  // Get node
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  std::string query =
      "MATCH (n:TaskList {name: '" + revised_list_pkey + "'}) RETURN n";
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
  if (task_list_info.find("name") != task_list_info.end()) {
    task_list_info["name"].erase(task_list_info["name"].begin() +
                                     task_list_info["name"].find("#"),
                                 task_list_info["name"].end());
  }
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getTaskNode(const std::string &user_pkey,
                           const std::string &task_list_pkey,
                           const std::string &task_pkey,
                           std::map<std::string, std::string> &task_info) {
  neo4j_connection_t *connection = connectDB();

  // Get node
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  std::string revised_task_pkey = task_pkey + "#" + revised_list_pkey;
  std::string query =
      "MATCH (n:Task {name: '" + revised_task_pkey + "'}) RETURN n";
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
  if (task_info.find("name") != task_info.end()) {
    task_info["name"].erase(task_info["name"].begin() +
                                task_info["name"].find("#"),
                            task_info["name"].end());
  }
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getAllUserNodes(std::vector<std::string> &user_info) {
  neo4j_connection_t *connection = connectDB();

  std::string query = "MATCH (n:User) RETURN n";
  neo4j_result_stream_t *results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
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
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getAllTaskListNodes(const std::string &user_pkey,
                                   std::vector<std::string> &task_list_info) {
  neo4j_connection_t *connection = connectDB();

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
  query = "MATCH (n:User {email: '" + user_pkey + "'})-[:Owns]->(m) RETURN m";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  while ((result = neo4j_fetch_next(results)) != NULL) {
    neo4j_value_t node = neo4j_result_field(result, 0);
    neo4j_value_t value = neo4j_node_properties(node);
    char buf[1024];
    neo4j_value_t field_value = neo4j_map_get(value, "name");
    neo4j_tostring(field_value, buf, sizeof(buf));
    std::string value_str(buf);
    value_str.pop_back();
    value_str.erase(0, 1);
    value_str.erase(value_str.begin() + value_str.find("#"), value_str.end());
    task_list_info.push_back(value_str);
  }
  neo4j_close_results(results);
  closeDB(connection);
  return SUCCESS;
}

returnCode DB::getAllTaskNodes(const std::string &user_pkey,
                               const std::string &task_list_pkey,
                               std::vector<std::string> &task_info) {
  neo4j_connection_t *connection = connectDB();

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
  std::string revised_list_pkey = task_list_pkey + "#" + user_pkey;
  query = "MATCH (n:TaskList {name: '" + revised_list_pkey + "'}) RETURN n";
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
  query = "MATCH (n:TaskList {name: '" + revised_list_pkey +
          "'})-[:Contains]->(m) RETURN m";
  results = executeQuery(query, connection);
  if (neo4j_check_failure(results)) {
    neo4j_close_results(results);
    closeDB(connection);
    return ERR_UNKNOWN;
  }
  while ((result = neo4j_fetch_next(results)) != NULL) {
    neo4j_value_t node = neo4j_result_field(result, 0);
    neo4j_value_t value = neo4j_node_properties(node);
    char buf[1024];
    neo4j_value_t field_value = neo4j_map_get(value, "name");
    neo4j_tostring(field_value, buf, sizeof(buf));
    std::string value_str(buf);
    value_str.pop_back();
    value_str.erase(0, 1);
    value_str.erase(value_str.begin() + value_str.find("#"), value_str.end());
    task_info.push_back(value_str);
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