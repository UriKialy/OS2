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
#include <fcntl.h>
#include <sys/select.h>

using namespace std;
bool running = true;

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

int start_tcp_server(const string &port)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Error setting socket options");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(stoi(port));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0)
    {
        perror("Error listening on socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

int start_tcp_client(const string &hostname, const string &port)
{
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct hostent *server = gethostbyname(hostname.c_str());
    if (server == nullptr)
    {
        cerr << "Error: No such host" << endl;
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(stoi(port));
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    return client_sock;
}

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        running = false;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    int opt;
    char *program = nullptr;
    string input_redirect, output_redirect;
    bool io_mux_mode = false;

    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1)
    {
        switch (opt)
        {
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
            if (strncmp(optarg, "TCPMUXS", 7) == 0)
            {
                io_mux_mode = true;
                input_redirect = optarg;
                output_redirect = optarg;
            }
            else
            {
                input_redirect = optarg;
                output_redirect = optarg;
            }
            break;
        default:
            cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << endl;
            return EXIT_FAILURE;
        }
    }

    if (!program)
    {
        cerr << "Usage: " << argv[0] << " -e <program> [args] [-i <input_redirect>] [-o <output_redirect>] [-b <bi_redirect>]" << endl;
        return EXIT_FAILURE;
    }

    if (io_mux_mode)
    {
        string port = input_redirect.substr(7);
        int server_sock = start_tcp_server(port);

        fd_set master_set, read_fds;
        FD_ZERO(&master_set);
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &master_set);
        int fd_max = server_sock;

        while (running)
        {
            read_fds = master_set;
            if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
            {
                perror("select");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i <= fd_max; i++)
            {
                if (FD_ISSET(i, &read_fds))
                {
                    if (i == server_sock)
                    {
                        int client_sock = accept(server_sock, nullptr, nullptr);
                        if (client_sock < 0)
                        {
                            perror("accept");
                            continue;
                        }

                        pid_t pid = fork();
                        if (pid < 0)
                        {
                            perror("fork");
                            close(client_sock);
                        }
                        else if (pid == 0)
                        {
                            close(server_sock);
                            dup2(client_sock, STDIN_FILENO);
                            dup2(client_sock, STDOUT_FILENO);
                            dup2(client_sock, STDERR_FILENO);
                            close(client_sock);

                            vector<string> split_program = split(string(program));
                            vector<char *> args;
                            for (const auto &arg : split_program)
                            {
                                args.push_back(const_cast<char *>(arg.c_str()));
                            }
                            args.push_back(nullptr);

                            // Add absolute path to ttt executable
                            char ttt_path[1024];
                            if (realpath(args[0], ttt_path) == nullptr)
                            {
                                perror("realpath");
                                exit(EXIT_FAILURE);
                            }
                            args[0] = ttt_path;

                            execvp(args[0], args.data());
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            close(client_sock);
                        }
                    }
                }
            }
        }

        close(server_sock);
    }
    else
    {
        vector<string> split_program = split(string(program));
        vector<char *> args;
        for (const auto &arg : split_program)
        {
            args.push_back(const_cast<char *>(arg.c_str()));
        }
        args.push_back(nullptr);

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0)
        {
            if (!input_redirect.empty())
            {
                if (input_redirect.substr(0, 4) == "TCPS")
                {
                    int port = stoi(input_redirect.substr(4));
                    int server_sock = start_tcp_server(to_string(port));
                    int client_sock = accept(server_sock, nullptr, nullptr);
                    if (client_sock < 0)
                    {
                        perror("accept");
                        close(server_sock);
                        return EXIT_FAILURE;
                    }
                    dup2(client_sock, STDIN_FILENO);
                    if (input_redirect == output_redirect)
                    {
                        dup2(client_sock, STDOUT_FILENO);
                        dup2(client_sock, STDERR_FILENO);
                        close(client_sock);
                    }
                    close(server_sock);
                    close(client_sock);
                }
            }

            if (!output_redirect.empty() && output_redirect != input_redirect)
            {
                if (output_redirect.substr(0, 4) == "TCPS")
                {
                    int port = stoi(output_redirect.substr(4));
                    int server_sock = start_tcp_server(to_string(port));
                    int client_sock = accept(server_sock, nullptr, nullptr);
                    if (client_sock < 0)
                    {
                        perror("accept");
                        close(server_sock);
                        close(client_sock);
                        return EXIT_FAILURE;
                    }
                    dup2(client_sock, STDOUT_FILENO);
                    dup2(client_sock, STDERR_FILENO);
                    close(server_sock);
                    close(client_sock);
                }
                else if (output_redirect.substr(0, 4) == "TCPC")
                {
                    string host_port = output_redirect.substr(4);
                    size_t comma_pos = host_port.find(',');
                    if (comma_pos != string::npos)
                    {
                        string hostname = host_port.substr(0, comma_pos);
                        string port = host_port.substr(comma_pos + 1);
                        int client_sock = start_tcp_client(hostname, port);
                        dup2(client_sock, STDOUT_FILENO);
                        dup2(client_sock, STDERR_FILENO);
                        close(client_sock);
                    }
                    else
                    {
                        cerr << "Invalid TCPC format. Expected TCPC<hostname,port>" << endl;
                        return EXIT_FAILURE;
                    }
                }
            }
            setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

            execvp(args[0], args.data());
            perror("execvp");
            return EXIT_FAILURE;
        }
        else
        {
            int status;
            while (running && waitpid(pid, &status, WNOHANG) == 0)
            {
                sleep(1);
            }
            if (!running)
            {
                kill(pid, SIGTERM);
                return EXIT_FAILURE;
            }
        }
    }

    return 0;
}
