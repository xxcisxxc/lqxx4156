#include <common/utils.h>
#include <api/api.h>
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

    API api;
    api.Run(api_host, api_port);
    return 0;
}
