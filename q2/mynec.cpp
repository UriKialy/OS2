#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <getopt.h>
#include <sstream>

// Function to split a string by spaces into a vector of strings
std::vector<std::string> split(const std::string &str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    for (std::string s; iss >> s;) {
        result.push_back(s);
    }
    return result;
}

int main(int argc, char *argv[]) {
    int opt;
    std::string program_str;

    // Using getopt to parse the command-line arguments
    while ((opt = getopt(argc, argv, "e:")) != -1) {
        switch (opt) {
            case 'e':
                program_str = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -e <program> [args]" << std::endl;
                return EXIT_FAILURE;
        }
    }

    if (!program_str.empty()) {
        std::cout << "Full program string: " << program_str << std::endl;

        // Split the program_str into executable and its arguments
        std::vector<std::string> split_args = split(program_str);

        if (split_args.empty()) {
            std::cerr << "No program specified to execute." << std::endl;
            return EXIT_FAILURE;
        }

        std::string program_path = "./" + split_args[0]; // Assuming the program is in the current directory
        std::vector<char*> args;
        args.push_back(const_cast<char*>(program_path.c_str()));

        for (size_t i = 1; i < split_args.size(); ++i) {
            args.push_back(const_cast<char*>(split_args[i].c_str()));
        }
        args.push_back(nullptr); // Null-terminating the argument list

        std::cout << "Executing program: " << program_path << std::endl;
        for (size_t i = 0; i < args.size() - 1; ++i) {
            std::cout << "Argument " << i << ": " << args[i] << std::endl;
        }

        // Executing the specified program with the given arguments
        execvp(args[0], args.data());
        // execvp only returns if an error occurs
        perror("Error executing program");
        return EXIT_FAILURE;
    } else {
        std::cerr << "Usage: " << argv[0] << " -e <program> [args]" << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
