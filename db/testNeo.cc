#include "DB.h"
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <thread>

const std::string host = "neo4j://neo4j:hello4156@localhost:7687";

void create_thread(int id, DB *db) {
  std::string user_pkey = "test" + std::to_string(id) + "@test.com";
  std::map<std::string, std::string> user_info;

  // Create the user node
  user_info["email"] = user_pkey;
  user_info["passwd"] = "test";
  user_info["test"] = std::to_string(id);
  db->createUserNode(user_info);
}

void delete_thread(int id, DB *db) {
  std::string user_pkey = "test" + std::to_string(id) + "@test.com";

  // Delete the user node
  auto ret = db->deleteUserNode(user_pkey);
}

int main(int argc, char *argv[]) {
  DB *db = new DB(host);
  const int thread_num = 20;
  std::thread t[thread_num];

  // Create 10 threads
  for (int i = 0; i < thread_num; i++) {
    t[i] = std::thread(create_thread, i, db);
  }
  // Wait for all threads to finish
  for (int i = 0; i < thread_num; i++) {
    t[i].join();
  }
  // Delete 10 threads
  for (int i = 0; i < thread_num; i++) {
    t[i] = std::thread(delete_thread, i, db);
  }
  // Wait for all threads to finish
  for (int i = 0; i < thread_num; i++) {
    t[i].join();
  }
  delete db;
  return 0;
}