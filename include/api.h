#pragma once

#include <memory>
#include <httplib.h>
#include <users.h>

#define DeclareHttpHandler(name)\
    virtual void name(const httplib::Request&, httplib::Response&)

class API {
public:
    API(std::shared_ptr<Users> _users = nullptr, std::shared_ptr<httplib::Server> _svr = nullptr);

    ~API();

    virtual void Run(const std::string& host, uint32_t port);

    virtual void Stop();

protected:
    DeclareHttpHandler(UsersRegister);

    DeclareHttpHandler(UsersLogin);

    DeclareHttpHandler(UsersLogout);

    DeclareHttpHandler(TaskLists);

    DeclareHttpHandler(TaskListsCreate);

    DeclareHttpHandler(Tasks);

    DeclareHttpHandler(TasksCreate);

private:
    std::shared_ptr<Users> users;
    // to be defined
    // std::shared_ptr<TaskLists> tasklists;
    // std::shared_ptr<Tasks> tasks;
    std::shared_ptr<httplib::Server> svr;
};
