#pragma once

#include <string>

/*
 * @brief This is a structure that specifies which object is what we try to
 * access ie: the object parsed from API url For example: /users/1/tasks/2
 */

struct RequestData {
  /* data */
  /*
   * @brief User id
   */
  std::string user_key;
  /*
   * @brief Task list id
   */
  std::string tasklist_key;
  /*
   * @brief Task id
   */
  std::string task_key;
  /**
   * @brief other user's id, we need to try to access tasklist from the
   * specified user if it contains value
   *
   */
  std::string other_user_key;

  /* methods */
  /*
   * @brief RequestData default constructor
   */
  RequestData() {}
  /*
   * @brief RequestData constructor
   *
   * @param _user_key user id
   * @param _tasklist_key task list id
   * @param _task_key task id
   */
  template <typename UserKey, typename TaskListKey, typename TaskKey,
            typename OtherUserKey>
  RequestData(UserKey &&_user_key, TaskListKey &&_tasklist_key,
              TaskKey &&_task_key, OtherUserKey &&_other_user_key)
      : user_key(std::forward<UserKey>(_user_key)),
        tasklist_key(std::forward<TaskListKey>(_tasklist_key)),
        task_key(std::forward<TaskKey>(_task_key)),
        other_user_key(std::forward<OtherUserKey>(_other_user_key)) {}
  /*
   * @brief RequestData decomposed constructor
   */
  virtual ~RequestData() {}
  /*
   * @brief RequestData copy constructor
   */
  RequestData(const RequestData &data) {
    user_key = data.user_key;
    tasklist_key = data.tasklist_key;
    task_key = data.task_key;
    other_user_key = data.other_user_key;
  }
  /*
   * @brief operator == overload
   * @param data RequestData object to compare
   */
  bool operator==(const RequestData &other) const {
    return this->user_key == other.user_key &&
           this->tasklist_key == other.tasklist_key &&
           this->task_key == other.task_key &&
           this->other_user_key == other.other_user_key;
  }
  /*
   * @brief Check if the User id is empty, const method
   * @return true if the User id is empty
   */
  bool RequestUserIsEmpty() const { return user_key == ""; }

  /*
   * @brief For TasklistWorker in request
   * @return true if the Task list id is empty
   */
  bool RequestTaskListIsEmpty() const {
    return user_key == "" || tasklist_key == "";
  }

  /*
   * @brief For TaskWorker in request
   * @return true if the Task id is empty
   */
  bool RequestIsEmpty() const {
    return user_key == "" || tasklist_key == "" || task_key == "";
  }
};