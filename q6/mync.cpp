#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sstream>
#include <csignal>
#include <sys/wait.h>

using namespace std;

bool running = true;

// Function to split a string by spaces into a vector of strings
vector<string> split(const string &str) {
    vector<string> result;
    istringstream iss(str);
    for (string s; iss >> s;) {
        result.push_back(s);
    }
    return result;
}

// Function to handle a client connection for datagram sockets
void handle_datagram_client(int sock_fd) {
    char buffer[1024];
    while (running) {
        ssize_t bytes_received = recv(sock_fd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("Error receiving data");
            break;
        } else if (bytes_received == 0) {
            cout << "Client disconnected" << endl;
            break;
        }
        // Process received data from buffer
        cout << "Received: " << string(buffer, bytes_received) << endl;
        // Send response if needed
        // ...
    }
}

// Function to handle a client connection for stream sockets
void handle_stream_client(int sock_fd) {
    char buffer[1024];
    while (running) {
        ssize_t bytes_received = recv(sock_fd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("Error receiving data");
            break;
        } else if (bytes_received == 0) {
            cout << "Client disconnected" << endl;
            break;
        }
        // Process received data from buffer
        cout << string(buffer, bytes_received);
    }
}

// Function to create a Unix domain socket (server)
int create_unix_socket_server(const string &type, const string &path) {
    int sock_fd;
    if (type == "UDS") {
        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0); // For stream sockets
    } else if (type == "UDD") {
        sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // For datagram sockets
    } else {
        cerr << "Invalid socket type: " << type << endl;
        return -1;
    }

    if (sock_fd < 0) {
        perror("Error creating socket");
        return -1;
    }

    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path.c_str());

    // Remove any existing socket file
    unlink(path.c_str());

    if (bind(sock_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Error binding socket");
        close(sock_fd);
        return -1;
    }

    if (type == "UDS") {
        if (listen(sock_fd, 1) < 0) { // Listen for connections for stream sockets
            perror("Error listening on socket");
            close(sock_fd);
            return -1;
        }
    }

    return sock_fd;
}

// Function to connect to a Unix domain socket (client)
int connect_unix_socket_client(const string &type, const string &path) {
    int sock_fd;
    if (type == "UDC") {
        sock_fd = socket(AF_UNIX, SOCK_STREAM, 0); // For stream sockets
    } else if (type == "UDS") {
        sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // For datagram sockets
    } else {
        cerr << "Invalid socket type: " << type << endl;
        return -1;
    }

    if (sock_fd < 0) {
        
