#pragma once

#include <memory>
#include <httplib.h>

#define DeclareHttpHandler(name)\
    virtual void name(const httplib::Request&, httplib::Response&)

class API {
public:
    virtual void Run(const std::string& host, uint32_t port);

protected:
    DeclareHttpHandler(UsersRegister);

    DeclareHttpHandler(UsersLogin);

    DeclareHttpHandler(UsersLogout);

    DeclareHttpHandler(TaskLists);

    DeclareHttpHandler(TaskListsCreate);

    DeclareHttpHandler(Tasks);

    DeclareHttpHandler(TasksCreate);

private:
    std::shared_ptr<httplib::Server> svr;
};
