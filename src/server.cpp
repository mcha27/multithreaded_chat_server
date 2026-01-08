#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <vector>
#include <algorithm>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {
    int server_fd;
    sockaddr_in server_addr{};
    std::vector<int> client_fds;
    char buffer[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    // Allow socket reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        
        int max_fd = server_fd;
        
        // Add all client sockets to set
        for (int client_fd : client_fds) {
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        // Wait for activity on any socket
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // Check for new connection
        if (FD_ISSET(server_fd, &read_fds)) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(server_fd, (sockaddr*)&client_addr, &client_len);
            
            if (new_client < 0) {
                perror("accept failed");
            } 
            else if (client_fds.size() >= MAX_CLIENTS) {
                std::cout << "Max clients reached, rejecting connection\n";
                close(new_client);
            } 
            else {
                client_fds.push_back(new_client);
                std::cout << "New client connected (fd: " << new_client 
                         << "). Total clients: " << client_fds.size() << std::endl;
            }
        }

        // Check all clients for incoming data
        for (auto it = client_fds.begin(); it != client_fds.end(); ) {
            int client_fd = *it;
            
            if (FD_ISSET(client_fd, &read_fds)) {
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
                
                if (bytes_received <= 0) {
                    // Client disconnected or error
                    if (bytes_received == 0) {
                        std::cout << "Client disconnected (fd: " << client_fd << ")\n";
                    } else {
                        perror("recv error");
                    }
                    close(client_fd);
                    it = client_fds.erase(it);
                } 
                else {
                    // Data received
                    std::cout << "Client (fd: " << client_fd << "): " << buffer;
                    
                    const char* response = "Hello from C++ server!";
                    ssize_t sent = send(client_fd, response, strlen(response), 0);
                    if (sent < 0) {
                        perror("send error");
                    }
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }

    // Cleanup
    for (int client_fd : client_fds) {
        close(client_fd);
    }
    close(server_fd);

    return 0;
}