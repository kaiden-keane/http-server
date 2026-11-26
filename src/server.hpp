#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <optional>

#define REQUEST_BUFF_SIZE 2048


enum HTTPMethod {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    HEAD,
    OPTIONS,
    CONNECT,
    TRACE,
};

// note: Nginx only uses 1.0 and 1.1 for backend communication
// default is 1.0 so we can prob ignore parts of 1.1 for now
enum HTTPVersion {
    HTTP_1_0,
    HTTP_1_1,
};

enum ParserState {
    METHOD,
    PATH,
    VERSION,
    LINE_END,
    HEADER_NAME,
    HEADER_VALUE,
    BODY,
};

inline int mod_subtract(int a, int b, int modulus);

struct Request {
    std::string body;
    HTTPMethod method;
    std::string path;
    HTTPVersion http_version;
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
        char request_buff[REQUEST_BUFF_SIZE];
        ParserState state;
        void update_state(const char buff[REQUEST_BUFF_SIZE], ssize_t read_amnt);
        bool request_is_finished();
        Request create_request();
        unsigned int state_start_idx;
        unsigned int line_start_idx;
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
    bool running;
    void handle_client(int clientfd);
    public:
    Server(int port_num, std::string ip = "0.0.0.0");
    ~Server();
    void run();
    bool is_running();
};
