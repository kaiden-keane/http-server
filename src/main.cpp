#include "server.hpp"
#include <iostream>

Response all_func(const Request& request) {
    std::cout << "response" << std::endl;
    Response response;
    return response;
}

int main (int argc, char *argv[]) {
    Server *server = new Server(8080);
    server->run();
    return 0;
}
