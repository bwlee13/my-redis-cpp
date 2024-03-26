#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include "RedisParser.cpp"

const int PORT = 6379;
//std::ostream& writeString(std::ostream& out, std::string const& s);
void handle_conn(int client_fd);
std::vector<std::string> splitRedisCommand(std::string input, std::string separator, int separatorLength);

int main(int argc, char** argv) {

    std::cout << "Logs from your program will appear here!\n";

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }
    // // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 6379\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    std::cout << "Listening at " << PORT << "." << std::endl;
    std::cout << "Waiting for a client to connect...\n";
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::vector<std::thread> threads;
    while (true) {
        int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr_len);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection \n";
            continue;
        }
        threads.push_back(std::thread(handle_conn, client_fd));
    }
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    close(server_fd);

    return 0;
}


void handle_conn(int client_fd) {
    char read_buffer[1024];
    const char* pong = "+PONG\r\n";

    while (true) {
        ssize_t bytes_received = recv(client_fd, read_buffer, sizeof(read_buffer), 0);
        std::string convertedBuffer(read_buffer);
        if (bytes_received <= 0) {
            std::cerr << "Client disconnected or read error \n";
            break;
        }

        std::cerr << "Testing \n";
        auto [count, message] = RedisParser::parse(convertedBuffer);
        std::cout << "raw: " << message.raw << std::endl;
        std::cout << "data: " << message.data << std::endl;

        std::cerr << "Done \n";
        std::string command_client = message.array[0];

        std::transform(command_client.begin(), command_client.end(), command_client.begin(), ::toupper);

        std::string write_string = "";

        if (command_client == "ECHO")
        {
            std::string echo_message = message.array[1];

            write_string = "$" + std::to_string(echo_message.size()) + "\r\n" + echo_message + "\r\n";
        }
        else if (command_client == "SET")
        {
            RedisParser::store[message.array[1]] = message.array[2];
            write_string = "+OK\r\n";
        }
        else if (command_client == "GET")
        {
            if (auto search = RedisParser::store.find(message.array[1]); search == RedisParser::store.end())
            {
                write_string = "$-1\r\n";
            }
            else
            {
                std::string result_store = RedisParser::store[message.array[1]];
                write_string = write_string = "$" + std::to_string(result_store.size()) + "\r\n" + result_store + "\r\n";
            }
        }
        else
        {
            write_string = "+PONG\r\n";
        }

        int size_string = write_string.size();
        auto sent_fd = write(client_fd, write_string.c_str(), size_string);
    }
    close(client_fd);
}

std::vector<std::string> splitRedisCommand(std::string input, std::string separator, int separatorLength) {
    // 1.: *x = x number of components, command, command + args etc
    // 2.: $x = x length command
    // 3.: command
    // 4?: $x = x length of argument
    // 5?: argument/s
    std::size_t foundSeparator = input.find(separator);
    std::vector<std::string> result;

    if (foundSeparator == std::string::npos) {
        result.push_back(input);
    }

    while (foundSeparator != std::string::npos) {
        std::string splitOccurrence = input.substr(0, foundSeparator);
        result.push_back(splitOccurrence);
        input = input.substr(foundSeparator + separatorLength, input.length() - foundSeparator + separatorLength);
        foundSeparator = input.find(separator);
    }

    return result;
}