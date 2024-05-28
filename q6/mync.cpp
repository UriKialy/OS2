#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <csignal>
#include <sys/wait.h>
using namespace std;
bool running = true;

// Function to split a string by spaces into a vector of strings
vector<string> split(const string &str)
{
    vector<string> result;
    istringstream iss(str);
    for (string s; iss >> s;)
    {
        result.push_back(s);
    }
    return result;
}

// Function to handle a client connection for input redirection
void handle_client_input(int client_sock)
{
    dup2(client_sock, STDIN_FILENO);
}

// Function to handle a client connection for output redirection
void handle_client_output(int client_sock)
{
    dup2(client_sock, STDOUT_FILENO);
    dup2(client_sock, STDERR_FILENO); // Also redirect stderr to the client socket
}

void signal_handler(int signal)
{ // handle the signals
    if (signal == SIGINT)
    {
        running = false;
    }
}

// Function to start a stream unix server
int start_Strean_unix_server(const string &socket_path)
{
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0); // create a  unix socket if error handle it
    if (server_sock < 0)
    {
        perror("Error creating stream socket");
        exit(EXIT_FAILURE);
    }
    // set the server options
    struct sockaddr_un server_addr = {};
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path.c_str());

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding stream socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0)
    {
        perror("Error listening on stream socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

// Function to start a datagram unix server
int start_Datagram_unix_server(const string &socket_path)
{
    int server_sock = socket(AF_UNIX, SOCK_DGRAM, 0); // create a  unix socket if error handle it
    if (server_sock < 0)
    {
        perror("Error creating  datagram socket");
        exit(EXIT_FAILURE);
    }
    // set the server options
    struct sockaddr_un server_addr = {};
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path.c_str());

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding datagram socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

// Function to start a stream unix client
int start_Stream_unix_client(const string &socket_path)
{
    int client_sock = socket(AF_UNIX, SOCK_STREAM, 0); // create a  unix socket if error handle it
    if (client_sock < 0)
    {
        perror("Error creating  datagram socket");
        exit(EXIT_FAILURE);
    }
    // set the client options
    struct sockaddr_un client_addr = {};
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, socket_path.c_str());

    if (connect(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

// Function to start a stream datagram client
int start_Datagram_unix_client(const string &socket_path)
{
    int client_sock = socket(AF_UNIX, SOCK_DGRAM, 0); // create a  unix socket if error handle it
    if (client_sock < 0)
    {
        perror("Error creating  datagram socket");
        exit(EXIT_FAILURE);
    }
    // set the client options
    struct sockaddr_un client_addr = {};
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, socket_path.c_str());

    if (connect(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler); // Handle Ctrl+C to terminate the program gracefully
    int opt;
    char *program = argv[2];//"ttt and 9 digits"
    string input_redirect, output_redirect;
    while ((opt = getopt(argc, argv, "UDSSD:UDSCD:UDSSS:UDSCS:")) != -1)
    {
        switch (opt)
        {
        case 'UDSSD': // if it is a stream server
            input_redirect = optarg;
            break;
        case 'UDSCD': // if it is a datagram server
            input_redirect = optarg;
            break;
        case 'UDSSS': // if it is a stream client
            output_redirect = optarg;
            break;
        case 'UDSCS': // if it is a datagram client
            output_redirect = optarg;
            break;
        default:
            cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << endl;
            return EXIT_FAILURE;
        }
    }
    if (!program)
    { // If the program name is not provided, print usage and exit
        cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << endl;
        return EXIT_FAILURE;
    }

    string program_str = string(program);

    vector<string> split_program = split(program_str); // Split the program name and arguments
    vector<char *> args;                               // Vector of char* to store the arguments for execvp
    for (const auto &arg : split_program)
    { // Convert the arguments to char* and store in the vector
        args.push_back(const_cast<char *>(arg.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Error forking process"); // If fork fails, print error and exit
        return EXIT_FAILURE;
    }

    if (pid == 0)
    { // Child process
        if (!input_redirect.empty())
        { // if there is an input redirection
            if (input_redirect.substr(0, 4) == "UDSSD")
            {                                                            // if it is a stream server
                string path = input_redirect.substr(4);                  // get the path
                int server_sock = start_Strean_unix_server(path);        // start the server
                int client_sock = accept(server_sock, nullptr, nullptr); // accept the connection if error handle it
                if (client_sock < 0)
                {
                    perror("Error accepting connection from client");
                    close(server_sock);
                    return EXIT_FAILURE;
                }   
                handle_client_input(client_sock); // handle the client input
                if (input_redirect == output_redirect)
                {
                    handle_client_output(client_sock);
                    close(client_sock);
                }
                close(server_sock);
                close(client_sock);
            }
            else if (input_redirect.substr(0, 4) == "UDSCD")
            { // if it is a datagram server
                string path = input_redirect.substr(4);                  // get the path
                int server_sock = start_Datagram_unix_server(path);      // start the server
                int client_sock = accept(server_sock, nullptr, nullptr); // accept the connection if error handle it
                if (client_sock < 0)
                {
                    perror("Error accepting connection from client");
                    close(server_sock);
                    return EXIT_FAILURE;
                }
                handle_client_input(client_sock); // handle the client input
                if (input_redirect == output_redirect)
                {
                    handle_client_output(client_sock);
                    close(client_sock);
                }
                close(server_sock);
                close(client_sock);
            }
        }
        // if there is an output redirection handle it (almost identical to the input redirection, only here we have both TCPC and TCPS)
        if (!output_redirect.empty() && output_redirect != input_redirect)
        {
            int server_sock;
            if (output_redirect.substr(0, 4) == "UDSCS")
            {// if it is a stream client
                cout << "the output redirect was " << output_redirect << endl;
                string path = output_redirect.substr(4);
                server_sock = start_Stream_unix_client(path);

            }
            else if (output_redirect.substr(0, 4) == "UDSCD")
            { // if it is a datagram client
                string path = output_redirect.substr(4);
                server_sock = start_Datagram_unix_client(path);
            }   
                int client_sock = accept(server_sock, nullptr, nullptr);
                if (client_sock < 0)
                {
                    perror("Error accepting connection");
                    close(server_sock);
                    close(client_sock);
                    return EXIT_FAILURE;
                }
                handle_client_output(client_sock);
                close(server_sock);
                close(client_sock);
            
        }

        // Set the buffer to be line-buffered
        setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

        // Executing the specified program with the given arguments
        execvp(args[0], args.data());
        // execvp only returns if an error occurs
        perror("Error executing program");
        return EXIT_FAILURE;
    }
    else
    { // Parent process
        int status;
        while (running && waitpid(pid, &status, WNOHANG) == 0)
        { // wait for the child process to finish
            sleep(1);
        }
        if (!running)
        { // If the program was terminated by Ctrl+C, kill the child process
            kill(pid, SIGTERM);
            return EXIT_FAILURE;
        }
    }

    return 0;
}