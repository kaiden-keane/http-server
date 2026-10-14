#include "server.hpp"
#include <iostream> // used for cout()
#include <cstring> // used for memset()
#include <arpa/inet.h> // used for inet_pton()
#include <unistd.h> // used for close()
#include <cstdlib> // FOR EXIT
#include <cassert> // for assert()

#define QUEUELEN 5


inline int mod_subtract(int a, int b, int modulus) {
    return (a - b + modulus) % modulus;
}


bool Server::init() {
    // create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd < 0) {
        std::cout << "ERROR Opening Socket" << std::endl;
        return false;
    }

    // specifying address
    std::memset(&this->serv_addr, 0, sizeof(this->serv_addr));
    this->serv_addr.sin_family = AF_INET;
    this->serv_addr.sin_port = htons(this->portno);

    if (this->ip_address.empty() | this->ip_address == "0.0.0.0") {
        this->serv_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        int inet_ret = inet_pton(AF_INET, this->ip_address.c_str(), &this->serv_addr.sin_addr);
        if (inet_ret < 0) {
            close(this->sockfd);
            return false;
        }
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

Server::Server(int port_num, std::string ip) : portno(port_num), ip_address(ip), sockfd(-1), running(false) {
    if (!this->init()) {
        std::exit(EXIT_FAILURE);
    }
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
        handle_client(clientfd);
    }
}


void Server::handle_client(int clientfd) {
    RequestParser parser;

}


void RequestParser::update_state(int &buff_idx) {
    bool new_state = false;
    char c = buff[buff_idx];
    switch (state) {
        case ParserState::METHOD:
            // if current character is a ' ' METHOD -> PATH
            if (c == ' ') {
                new_state = true;
                state = ParserState::PATH;
                buff_idx = (buff_idx + 1) % buff_size;
            }
            break;
        case ParserState::PATH:
            // if current character is a ' ' PATH -> VERSION
            if (c == ' ') {
                new_state = true;
                state = ParserState::VERSION;
                buff_idx = (buff_idx + 1) % buff_size;
            }
            break;
        case ParserState::VERSION:
            // if current character is a '\r' VERSION -> LINE_END 
            if (c == '\r') {
                new_state = true;
                state = ParserState::HEADER_NAME;
                buff_idx = (buff_idx + 2) % buff_size;
            }
            break;
        case ParserState::LINE_END:
            // if the next bit is not '\n\r\n' LINE_END -> HEADER_NAME
            // else we are done

                new_state = true;
            new_state = ParserState::HEADER_NAME;
            break;
        case ParserState::HEADER_NAME:
            // if current character is a ' ' HEADER_NAME -> HEAD_VALUE
            if (c == ' ') {
                new_state = true;
                state = ParserState::HEADER_VALUE;
                buff_idx = (buff_idx + 1) % buff_size;
            }
            break;
        case ParserState::HEADER_VALUE:
            // if current character is a '\r' HEAD_VALUE -> LINE_END
            if (c == '\r') {
                new_state = true;
                state = ParserState::LINE_END;
                buff_idx = (buff_idx + 2) % buff_size;
            }
            break;
    }
}

std::unique_ptr<Request> RequestParser::parse_fd(int clientfd) {
    auto request = std::make_unique<Request>();
    ssize_t bytes_read;
    size_t bytes_to_process;
    int buff_idx;
    while (1) {
        // if read_idx < write_idx then we only try to fill up buffer, otherwise, fill to read_idx
        // note: read_idx is goiung to be larger than write_idx in this comparison
        size_t read_amnt = read_idx < write_idx ? buff_size - write_idx : read_idx - write_idx;
        bytes_read = read(clientfd, &buff[write_idx], read_amnt);
        if (bytes_read < 0) {
            std::cout << "Error Reading Data" << std::endl;
            return nullptr;
        }
        else if (bytes_read == 0) {
            std::cout << "Connection Closed" << std::endl;
            break;
        }
        // update the write index, due to our read_amnt, this should be less than read_idx
        write_idx = (write_idx + bytes_read) % buff_size;
        // amount to process is the amount of bytes between read and write index. w - r
        bytes_to_process = mod_subtract(write_idx, read_idx, buff_size);
        for (int i=0; i < bytes_to_process; i++) {
            buff_idx = (read_idx+i) % buff_size;
            update_state(buff_idx);
        }
    }
    return request;
}

