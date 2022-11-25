#include "db/DB.h"
#include <api/api.h>
#include <common/utils.h>
#include <memory>
#include <string>

int main(void) {
  std::string api_host = Common::GetEnv<std::string>("api_host");
  uint32_t api_port = Common::GetEnv<uint32_t>("api_port");

  if (api_host.empty()) {
    api_host = "0.0.0.0";
  }
  if (!api_port) {
    api_port = 3001;
  }

  const std::string db_host = "neo4j://neo4j:hello4156@localhost:7687";
  auto db_instance = std::make_shared<DB>(db_host);

  Api api(nullptr, nullptr, nullptr, db_instance, nullptr);
  api.Run(api_host, api_port);
  return 0;
}
