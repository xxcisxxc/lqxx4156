#include <api.h>

int main(void) {
    API api;
    api.Run("0.0.0.0", 3001);
    return 0;
}