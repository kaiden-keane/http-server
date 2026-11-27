#include "server.hpp"
#include <iostream> // used for cout()
#include <cstring> // used for memset()
#include <arpa/inet.h> // used for inet_pton()
#include <unistd.h> // used for close()
#include <cstdlib> // FOR EXIT
#include <cassert> // for assert()

#define QUEUELEN 5


bool Server::init() {
    // create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd <= 0) {
        std::cout << "ERROR Opening Socket" << std::endl;
        return false;
    }

    // specifying address
    std::memset(&this->serv_addr, 0, sizeof(this->serv_addr));
    this->serv_addr.sin_family = AF_INET;
    this->serv_addr.sin_port = htons(this->portno);

    if (this->ip_address.empty() || this->ip_address == "0.0.0.0") {
        this->serv_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        int inet_ret = inet_pton(AF_INET, this->ip_address.c_str(), &this->serv_addr.sin_addr);
        if (inet_ret < 0) {
            close(this->sockfd);
            return false;
        }
    }

    // allow port re-use
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cout << "Error setting sockopt" << std::endl;
        return false;
    }

    // bind the socket
    int bind_res = bind(this->sockfd, (struct sockaddr*)&this->serv_addr, sizeof(this->serv_addr));
    if (bind_res < 0) {
        std::cout << "Error Binding Socket" << std::endl;
        return false;
    }
    return true;
}


bool Server::is_running() {
    return running;
}


Server::Server(int port_num, std::string ip) : sockfd(-1), portno(port_num), ip_address(ip), running(false) {
    std::cout << port_num << "\t" << ip << std::endl;
    if (!this->init()) {
        std::cout << "error in init" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "init good" << std::endl;
}


Server::~Server() {
    if (sockfd >= 0) {
        if (close(sockfd) < 0) {
            std::cout << "Error closing sockfd" << std::endl;
        }
    }
}


void Server::run() {
    struct sockaddr_in client_addr;
    int clientfd;
    if (listen(sockfd, QUEUELEN) < 0) {
        std::cout << "Error During Socket Listen" << std::endl;
        return;
    }
    socklen_t client_addr_size = sizeof(client_addr);
    std::memset(&client_addr, 0, sizeof(client_addr));
    while (true) {
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (clientfd < 0) {
            std::cout << "Client Socket Error" << std::endl;
            continue;
        }
        std::cout << "handling client " << clientfd << std::endl;
        handle_client(clientfd);
        close(clientfd);
    }
}


void Server::handle_client(int clientfd) {
    RequestHandler request_handler;
    auto request = request_handler.handle_fd(clientfd);
    if (!request.has_value()) {
        return;
    }
    std::cout << request->method << "\n" << request->path << "\n" << request->http_version << std::endl;
    // based on the path, get the route
    // using the route mapped to function, execute the function if it exists
    // call the function, should give a response obj, maybe put this in a try?
}


void RequestHandler::update_state(const char buff[REQUEST_BUFF_SIZE], ssize_t read_amnt) {
    for (unsigned int buff_idx=0; buff_idx < read_amnt && state != ParserState::BODY; buff_idx++) {
        char c = buff[buff_idx];
        std::cout <<  "* " << buff_idx << "\t" << state << "\t" << c << std::endl;
        switch (state) {
            case ParserState::METHOD:
                // if current character is a ' ' METHOD -> PATH
                if (c == ' ') {
                    state = ParserState::PATH;
                } else {
                    request.method.push_back(c);
                }
                break;
            case ParserState::PATH:
                // if current character is a ' ' PATH -> VERSION
                if (c == ' ') {
                    state = ParserState::VERSION;
                } else {
                    request.path.push_back(c);
                }
                break;
            case ParserState::VERSION:
                // if current character is a '\r' VERSION -> LINE_END 
                if (c == '\r') {
                    state = ParserState::HEADER_NAME;
                    buff_idx += 1; // for the newline char
                } else {
                    request.http_version.push_back(c);
                }
                break;
            case ParserState::HEADER_NAME:
                // if current character is a ' ' HEADER_NAME -> HEAD_VALUE
                if (c == ':') {
                    state = ParserState::HEADER_VALUE;
                    buff_idx += 1; // for the space char
                } else {
                    current_header_key.push_back(c);
                }
                break;
            case ParserState::HEADER_VALUE:
                // if current character is a '\r' HEAD_VALUE -> LINE_END
                if (c == '\r') {
                    state = ParserState::LINE_END;
                    buff_idx += 1; // for the newline char
                } else {
                    current_header_val.push_back(c);
                }
                break;
            case ParserState::LINE_END: // when we get to here, we have already processed \r\n
                request.headers.emplace(std::move(current_header_key), std::move(current_header_val));
                current_header_key.clear();
                current_header_key.clear();
                // if current byte is '\r' the next few is '\n\r\n' we are done, first ensure we have read the next 3
                // std::cout << "\n\n\t" << read_amnt - buff_idx << std::endl;
                if (read_amnt - buff_idx >= 1 &&
                        (c == '\r' && buff[buff_idx+1] == '\n')) {
                    buff_idx += 1;
                    // header done, handle body or finish
                    state = ParserState::BODY;
                    finished_request = true;
                }
                // else LINE_END -> HEADER_NAME
                else {
                    // we should process this character as part of current_header_key
                    current_header_key.push_back(c);
                    state = ParserState::HEADER_NAME;
                }
                break;
            default:
                break;
        }

    }
}


std::optional<Request> RequestHandler::handle_fd(int clientfd) {

    ssize_t bytes_read;
    while (1) {
        std::cout << "reading" << std::endl;
        bytes_read = read(clientfd, request_buff, REQUEST_BUFF_SIZE);
        std::cout << "read " << bytes_read << " bytes" << std::endl;
        if (bytes_read <= 0) {
            break;
        }
        update_state(request_buff, bytes_read);
        std::cout << "ending state: " << state << std::endl;
        if (is_request_finished()) {
            return request;
        }
    }
    if (bytes_read < 0) {
        std::cout << "Error Reading Data" << std::endl;
    }
    else if (bytes_read == 0) {
        std::cout << "Connection Closed" << std::endl;
    }

    return std::nullopt;
}


bool RequestHandler::is_request_finished() {
    return finished_request;
}
