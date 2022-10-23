#include <api/api.h>

int main(void) {
    std::cout << "running" << std::endl;
    API api;
    api.Run("0.0.0.0", 3001);
    return 0;
}