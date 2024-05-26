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

// Function to handle a client connection for input redirection
void handle_client_input(int client_sock) {
    dup2(client_sock, STDIN_FILENO);
}

// Function to handle a client connection for output redirection
void handle_client_output(int client_sock) {
    dup2(client_sock, STDOUT_FILENO);
    dup2(client_sock, STDERR_FILENO); // Also redirect stderr to the client socket
}

// Function to start a TCP server
int start_tcp_server(const string &port) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);// create a socket if error handle it
    if (server_sock < 0) {// handle error
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {// set socket options if error handle it
        perror("Error setting socket options");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    // set the server options
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(stoi(port));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { // bind the socket if error handle it
        perror("Error binding socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 1) < 0) { // listen on the socket if error handle it
        perror("Error listening on socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

// Function to start a TCP client almost identical to the server (just from the other side ;) )
int start_tcp_client(const string &hostname, const string &port) {
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);// create a socket if error handle it
    if (client_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct hostent *server = gethostbyname(hostname.c_str());   // get the host by name if error handle it
    if (server == nullptr) {
        cerr << "Error: No such host" << endl;
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    // set the server options
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(stoi(port));
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);// copy the address

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {// connect to the server if error handle it
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

void signal_handler(int signal) {// handle the signals
    if (signal == SIGINT) {
        running = false;
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler); // Handle Ctrl+C to terminate the program gracefully

    int opt;
    char *program = nullptr;
    string input_redirect, output_redirect;

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
                cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << endl;
                return EXIT_FAILURE;
        }
    }

    if (!program) { // If the program name is not provided, print usage and exit
        cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << endl;
        return EXIT_FAILURE;
    }
    
    // Add "./" to the beginning of the program name
    string program_str = "./" + string(program);

    vector<string> split_program = split(program_str); // Split the program name and arguments
    vector<char*> args; // Vector of char* to store the arguments for execvp
    for (const auto &arg : split_program) { // Convert the arguments to char* and store in the vector
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking process"); // If fork fails, print error and exit
        return EXIT_FAILURE;
    }

    if (pid == 0) { // Child process
        if (!input_redirect.empty()) {// if there is an input redirection
            if (input_redirect.substr(0, 4) == "TCPS") {
                int port = stoi(input_redirect.substr(4));// get the port
                cout << "the port was " << port << endl;
                int server_sock = start_tcp_server(to_string(port));// start the server
                int client_sock = accept(server_sock, nullptr, nullptr);// accept the connection if error handle it
                if (client_sock < 0) {
                    perror("Error accepting connection");
                    close(server_sock);
                    return EXIT_FAILURE;
                }
                handle_client_input(client_sock);// handle the client input 
                if (input_redirect == output_redirect) {
                    handle_client_output(client_sock);
                    close(client_sock);
                }
                close(server_sock);
                close(client_sock);
            }
            
        }
        // if there is an output redirection handle it (almost identical to the input redirection, only here we have both TCPC and TCPS)
        if (!output_redirect.empty() && output_redirect != input_redirect) {
            if (output_redirect.substr(0, 4) == "TCPS") {
                cout << "the output redirect was " << output_redirect << endl;
                int port = stoi(output_redirect.substr(4));
                int server_sock = start_tcp_server(to_string(port));
                 std::cerr << "Attempting to accept TCP output on port " << port << "...\n";
                int client_sock = accept(server_sock, nullptr, nullptr);
                if (client_sock < 0) {
                    perror("Error accepting connection");
                    close(server_sock);
                    close(client_sock);
                    return EXIT_FAILURE;
                }
                handle_client_output(client_sock);
                close(server_sock);
                close(client_sock);
            } else if (output_redirect.substr(0, 4) == "TCPC") {
                string host_port = output_redirect.substr(4);
                size_t comma_pos = host_port.find(',');
                if (comma_pos != string::npos) {
                    string hostname = host_port.substr(0, comma_pos);
                    string port = host_port.substr(comma_pos + 1);
                    //cout << "starting tcp with " << hostname << " and port " << port << endl;
                    int client_sock = start_tcp_client(hostname, port);
                    handle_client_output(client_sock);
                    close(client_sock);
                } else {
                    cerr << "Invalid TCPC format. Expected TCPC<hostname,port>" << endl;
                    return EXIT_FAILURE;
                }
            }
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
        while (running && waitpid(pid, &status, WNOHANG) == 0) {// wait for the child process to finish
            sleep(1);
        }
        if (!running) { // If the program was terminated by Ctrl+C, kill the child process
            kill(pid, SIGTERM);
            return EXIT_FAILURE;
        }
        
    }

    return 0;
}