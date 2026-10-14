#pragma once

// #include <functional>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory> // unique ptr etc

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


// using RouteHandler = std::function<Response(const Request&)>;
class RequestParser {
    private:
        char buff[2048];
        const unsigned int buff_size = 2048;
        ParserState state;
        void update_state(int &buff_idx);
        unsigned int state_start_idx;
        unsigned int line_start_idx;
        unsigned int read_idx;
        int write_idx;
    std::unique_ptr<Request> parse_fd(int client_fd);
    public:
    RequestParser();
};

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
