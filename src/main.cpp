#include "server.hpp"

int main (int argc, char *argv[]) {
    Server server{8080, "127.0.0.1"};
    server.run();
    return 0;
}
