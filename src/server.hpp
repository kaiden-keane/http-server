#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <optional>

#define REQUEST_BUFF_SIZE 2048


enum ParserState {
    METHOD,
    PATH,
    VERSION,
    HEADER_NAME,
    HEADER_VALUE,
    LINE_END,
    BODY,
};

inline int mod_subtract(int a, int b, int modulus);

struct Request {
    std::string body;
    std::string method;
    std::string path;
    std::string http_version;
    std::unordered_map<std::string, std::string> headers;
};

struct Response {
    unsigned int status;
    std::string header;
    std::string body;
    std::string to_string();
};


class RequestHandler {
    private:
        Request request; // request that will be built by this
        char request_buff[REQUEST_BUFF_SIZE];
        ParserState state{METHOD};
        unsigned int state_start_idx;
        unsigned int line_start_idx;
        std::string current_header_key;
        std::string current_header_val;
        bool finished_request{false};
        void update_state(const char buff[REQUEST_BUFF_SIZE], ssize_t read_amnt);
        bool is_request_finished();
    public:
    // RequestHandler();
    std::optional<Request> handle_fd(int client_fd);
};


using RouteHandler = std::function<Response(const Request&)>;


class Server {
    private:
    struct sockaddr_in serv_addr;
    int sockfd;
    int portno;
    std::string ip_address;
    bool init();
    bool running{false};
    void handle_client(int clientfd);
    public:
    Server(int port_num, std::string ip = "0.0.0.0");
    ~Server();
    void run();
    bool is_running();
};
