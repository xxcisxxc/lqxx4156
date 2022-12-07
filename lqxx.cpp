#include "tasklists/tasklistsWorker.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "api/api.h"
#include "common/utils.h"
#include "db/DB.h"
#include <memory>
#include <string>

int main(void) {
  std::string api_host = Common::GetEnv<std::string>("api_host");
  uint32_t api_port = Common::GetEnv<uint32_t>("api_port");

  if (api_host.empty()) {
    api_host = "0.0.0.0";
  }
  if (!api_port) {
    api_port = 443;
  }

  const std::string db_host = "neo4j://neo4j:hello4156@localhost:7687";
  auto db_instance = std::make_shared<DB>(db_host);
  auto svr =
      std::make_shared<httplib::SSLServer>("/root/cert.pem", "/root/key.pem");
  auto user = std::make_shared<Users>(db_instance);
  auto tasklists_worker = std::make_shared<TaskListsWorker>(db_instance, user);
  auto tasks_worker = std::make_shared<TasksWorker>(db_instance, tasklists_worker);
  

  Api api(user, tasklists_worker, tasks_worker);
  api.Run(api_host, api_port);
  return 0;
}
