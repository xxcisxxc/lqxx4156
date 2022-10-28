/**
 * @file api.h
 * @author Shichen Xu
 * @brief Definition of API class, which runs the main backend service for lqxx.
 * @date 2022-10
 *
 * The API class runs a top level http service for lqxx project.
 * It receives and responds to the http requests from clients,
 * handles authentication issues and basic request format checking,
 * and rely on classes Users, TaskLists, Tasks, to handle more complex business
 * logic.
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <httplib.h>
#include <memory>
#include <tasklists/tasklistsWorker.h>
#include <tasks/tasksWorker.h>
#include <users/users.h>

/* Declare a function that would be called to handle an http request of a
   certain route. The function name should be corresponding to the http
   interface name */
#define API_DECLARE_HTTP_HANDLER(name)                                         \
  virtual void name(const httplib::Request &, httplib::Response &) noexcept

class API {
public:
  /**
   * @brief Construct a new API object.
   * A secret token key will also be created upon calling this constructor.
   *
   * @param _users A shared pointer of Users object.
   * @param _svr A shared pointer of httplib::Server object.
   */
  API(std::shared_ptr<Users> _users = nullptr,
      std::shared_ptr<TaskListsWorker> _tasklists_worker = nullptr,
      std::shared_ptr<TasksWorker> _tasks_worker = nullptr,
      std::shared_ptr<DB> _db = nullptr,
      std::shared_ptr<httplib::Server> _svr = nullptr);

  /**
   * @brief Destroy the API object
   */
  ~API();

  /**
   * @brief Listen to the given host and port and run the http service, will be
   * blocked.
   *
   * @param host Host to run.
   * @param port Port to listen. If the port is already occupied, program will
   * be terminated.
   */
  virtual void Run(const std::string &host, uint32_t port);

  virtual void Stop();

protected:
  API_DECLARE_HTTP_HANDLER(UsersRegister);

  API_DECLARE_HTTP_HANDLER(UsersLogin);

  API_DECLARE_HTTP_HANDLER(UsersLogout);

  API_DECLARE_HTTP_HANDLER(TaskListsAll);

  API_DECLARE_HTTP_HANDLER(TaskListsGet);

  API_DECLARE_HTTP_HANDLER(TaskListsUpdate);

  API_DECLARE_HTTP_HANDLER(TaskListsDelete);

  API_DECLARE_HTTP_HANDLER(TaskListsCreate);

  API_DECLARE_HTTP_HANDLER(TasksAll);

  API_DECLARE_HTTP_HANDLER(TasksGet);

  API_DECLARE_HTTP_HANDLER(TasksUpdate);

  API_DECLARE_HTTP_HANDLER(TasksDelete);

  API_DECLARE_HTTP_HANDLER(TasksCreate);

  API_DECLARE_HTTP_HANDLER(Health);

private:
  std::shared_ptr<Users> users;
  std::shared_ptr<TaskListsWorker> tasklists_worker;
  std::shared_ptr<TasksWorker> tasks_worker;
  std::shared_ptr<DB> db;
  std::shared_ptr<httplib::Server> svr;

  const std::string token_secret_key;
};
