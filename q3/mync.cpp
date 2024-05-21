#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <csignal>
#include <sys/wait.h>

bool running = true;

// Function to split a string by spaces into a vector of strings
std::vector<std::string> split(const std::string &str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    for (std::string s; iss >> s;) {
        result.push_back(s);
    }
    return result;
}

// Function to handle a client connection for input redirection
void handle_client_input(int client_sock) {
    dup2(client_sock, STDIN_FILENO);
    close(client_sock);
}

// Function to handle a client connection for output redirection
void handle_client_output(int client_sock) {
    dup2(client_sock, STDOUT_FILENO);
    close(client_sock);
}

// Function to start a TCP server
int start_tcp_server(const std::string &port) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(port));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 1) < 0) {
        perror("Error listening on socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

// Function to start a TCP client
int start_tcp_client(const std::string &hostname, const std::string &port) {
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct hostent *server = gethostbyname(hostname.c_str());
    if (server == nullptr) {
        std::cerr << "Error: No such host" << std::endl;
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(port));
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler); // Handle Ctrl+C to terminate the program gracefully

    int opt;
    char *program = nullptr;
    std::string input_redirect, output_redirect;

    // Using getopt to parse the command-line arguments
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1) {
        switch (opt) {
            case 'e':
                program = optarg;
                break;
            case 'i':
                input_redirect = optarg;
                break;
            case 'o':
                output_redirect = optarg;
                break;
            case 'b':
                input_redirect = optarg;
                output_redirect = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << std::endl;
                return EXIT_FAILURE;
        }
    }

    if (!program) {
        std::cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << std::endl;
        return EXIT_FAILURE;
    }

    // Add "./" to the beginning of the program name
    std::string program_str = "./" + std::string(program);

    std::vector<std::string> split_program = split(program_str);
    std::vector<char*> args;
    for (const auto &arg : split_program) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking process");
        return EXIT_FAILURE;
    }

    if (pid == 0) { // Child process
        if (!input_redirect.empty()) {
            if (input_redirect.substr(0, 4) == "TCPS") {
                int port = std::stoi(input_redirect.substr(4));
                int server_sock = start_tcp_server(std::to_string(port));
                int client_sock = accept(server_sock, nullptr, nullptr);
                if (client_sock < 0) {
                    perror("Error accepting connection");
                    close(server_sock);
                    return EXIT_FAILURE;
                }
                handle_client_input(client_sock);
                close(server_sock);
            }
            // Additional input redirect implementations can be added here
        }

        if (!output_redirect.empty()) {
            if (output_redirect.substr(0, 4) == "TCPS") {
                int port = std::stoi(output_redirect.substr(4));
                int server_sock = start_tcp_server(std::to_string(port));
                int client_sock = accept(server_sock, nullptr, nullptr);
                if (client_sock < 0) {
                    perror("Error accepting connection");
                    close(server_sock);
                    return EXIT_FAILURE;
                }
                handle_client_output(client_sock);
                close(server_sock);
            } else if (output_redirect.substr(0, 4) == "TCPC") {
                std::string host_port = output_redirect.substr(4);
                size_t comma_pos = host_port.find(',');
                if (comma_pos != std::string::npos) {
                    std::string hostname = host_port.substr(0, comma_pos);
                    std::string port = host_port.substr(comma_pos + 1);
                    int client_sock = start_tcp_client(hostname, port);
                    handle_client_output(client_sock);
                } else {
                    std::cerr << "Invalid TCPC format. Expected TCPC<hostname,port>" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            // Additional output redirect implementations can be added here
        }

        // Set the buffer to be line-buffered
        setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

        // Executing the specified program with the given arguments
        execvp(args[0], args.data());
        // execvp only returns if an error occurs
        perror("Error executing program");
        return EXIT_FAILURE;
    } else { // Parent process
        int status;
        while (running && waitpid(pid, &status, WNOHANG) == 0) {
            sleep(1);
        }
        if (!running) {
            kill(pid, SIGTERM);
        }
    }

    return 0;
}
