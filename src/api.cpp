#include <api.h>
#include <json.hpp>

#define DefineHttpHandler(name)\
    void API::name(const httplib::Request& req, httplib::Response& res)

#define AddHttpHandler(server, path, method, func)\
    server->method(path, \
    [this](const httplib::Request& req, httplib::Response& res){this->func(req, res);})

DefineHttpHandler(UsersRegister) {
    nlohmann::json result;

    try {
        nlohmann::json json_body(req.body);
        std::string user_name = json_body.at("name");
        std::string user_passwd = json_body.at("passwd");

        std::cout << "user_name=" << user_name << std::endl;
        std::cout << "user_password=" << user_passwd << std::endl;
        // Do something

        result["msg"] = "success";
        res.set_content(result.dump(), "text/plain");
    } catch (std::exception& e) {
        res.set_content("Request body format error.", "text/plain");
    }

    std::cout << "Request body: " << req.body;
}

DefineHttpHandler(UsersLogin) {
    std::cout << "Request body: " << req.body;
}

DefineHttpHandler(UsersLogout) {
    std::cout << "Request body: " << req.body;
}

DefineHttpHandler(TaskLists) {
    std::cout << "Request body: " << req.body;
}

DefineHttpHandler(TaskListsCreate) {
    std::cout << "Request body: " << req.body;
}

DefineHttpHandler(Tasks) {
    std::cout << "Request body: " << req.body;
}

DefineHttpHandler(TasksCreate) {
    std::cout << "Request body: " << req.body;
}

void API::Run(const std::string& host, uint32_t port) {
    svr = std::make_shared<httplib::Server>();

    AddHttpHandler(svr, "/v1/users/register", Post, UsersRegister);
    AddHttpHandler(svr, "/v1/users/login", Post, UsersLogin);
    AddHttpHandler(svr, "/v1/users/logout", Post, UsersLogout);
    AddHttpHandler(svr, "/v1/task_lists/", Get, TaskLists);
    AddHttpHandler(svr, "/v1/task_lists/create", Post, TaskListsCreate);

    svr->listen(host, port);
}
