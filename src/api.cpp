#include <api.h>
#include <json/include/nlohmann/json.hpp>

#define DefineHttpHandler(name)\
    void API::name(const httplib::Request& req, httplib::Response& res)

#define AddHttpHandler(server, path, method, func)\
    server->method(path, \
    [this](const httplib::Request& req, httplib::Response& res){this->func(req, res);})

API::API(std::shared_ptr<Users> _users, std::shared_ptr<httplib::Server> _svr):
    users(_users), svr(_svr) {
    if (!users) {
        users = std::make_shared<Users>();
    }
    if (!svr) {
        svr = std::make_shared<httplib::Server>();
    }
}

API::~API() {
    Stop();
}

DefineHttpHandler(UsersRegister) {
    std::string user_name;
    std::string user_passwd;
    std::string user_email;
    nlohmann::json result;

    try {
        const auto json_body = nlohmann::json::parse(req.body);
        user_name = json_body.at("name");
        user_passwd = json_body.at("passwd");
        user_email = json_body.at("email");

        // print for testing
        std::cout << "user_name=" << user_name << std::endl;
        std::cout << "user_password=" << user_passwd << std::endl;
    } catch (std::exception& e) {
        res.set_content("Request body format error.", "text/plain");
        return;
    }

    // check if user email is duplicated
    if (users->DuplicatedEmail(user_email)) {
        result["msg"] = "failed";
        res.set_content(result.dump(), "text/plain");
        return;
    }

    // create user
    if (users->Create(user_name, user_email, user_passwd)) {
        result["msg"] = "success";
    } else {
        result["msg"] = "failed";
    }
    res.set_content(result.dump(), "text/plain");
}

DefineHttpHandler(UsersLogin) {
    std::string user_passwd;
    std::string user_email;
    nlohmann::json result;

    try {
        const auto json_body = nlohmann::json::parse(req.body);
        user_passwd = json_body.at("passwd");
        user_email = json_body.at("email");
    } catch (std::exception& e) {
        res.set_content("Request body format error.", "text/plain");
        return;
    }

    if (users->Validate({}, user_email, user_passwd)) {
        result["msg"] = "success";
        result["token"] = "some token";
    } else {
        result["msg"] = "failed";
    }

    res.set_content(result.dump(), "text/plain");
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
    AddHttpHandler(svr, "/v1/users/register", Post, UsersRegister);
    AddHttpHandler(svr, "/v1/users/login", Post, UsersLogin);
    AddHttpHandler(svr, "/v1/users/logout", Post, UsersLogout);
    AddHttpHandler(svr, "/v1/task_lists/", Get, TaskLists);
    AddHttpHandler(svr, "/v1/task_lists/create", Post, TaskListsCreate);

    svr->Get("/stop", [this](const httplib::Request& req, httplib::Response& res){this->Stop();});
    svr->listen(host, port);
}

void API::Stop() {
    if (svr && svr->is_running()) {
        svr->stop();
    }
}
