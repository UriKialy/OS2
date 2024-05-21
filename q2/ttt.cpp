#include "ttt.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <limits>


#define BOARD_SIZE 9

void print_board(const std::vector<char>& board) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (i % 3 == 0 && i != 0)
            std::cout << std::endl;
        std::cout << "[" << board[i] << "] ";
    }
    std::cout << std::endl;
}

bool check_win(const std::vector<char>& board, char player) {
    std::vector<std::vector<int>> win_positions = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // Rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // Columns
        {0, 4, 8}, {2, 4, 6}              // Diagonals
    };
    for (const auto& win_pos : win_positions) {
        if (board[win_pos[0]] == player &&
            board[win_pos[1]] == player &&
            board[win_pos[2]] == player) {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strlen(argv[1]) != 9) {
        std::cout << "Error" << std::endl;
        return 1;
    }

    std::string strategy = argv[1];

    for (char ch = '1'; ch <= '9'; ++ch) {
        if (strategy.find(ch) == std::string::npos) {
            std::cout << "Error" << std::endl;
            return 1;
        }
    }

    std::vector<char> board(BOARD_SIZE, '-');
    int move, played_moves = 0;
    char player = 'X', opponent = 'O';

    while (true) {
        // Program's move
        if (played_moves < 8) {
            for (int i = 0; i < 9; ++i) {
                int pos = strategy[i] - '1';
                if (board[pos] != 'X' && board[pos] != 'O') {
                    board[pos] = player;
                    std::cout << pos + 1 << std::endl;
                    break;
                }
            }
        } else {
            for (int i = 8; i >= 0; --i) {
                int pos = strategy[i] - '1';
                if (board[pos] != 'X' && board[pos] != 'O') {
                    board[pos] = player;
                    std::cout << pos + 1 << std::endl;
                    break;
                }
            }
        }

        played_moves++;
        print_board(board);

        if (check_win(board, player)) {
            std::cout << "I win" << std::endl;
            return 0;
        }

        if (played_moves == 9) {
            std::cout << "DRAW" << std::endl;
            return 0;
        }

        // Player's move
        std::cout << "Enter your move: ";
        while (true) {
            if (!(std::cin >> move) || move < 1 || move > 9 || board[move - 1] == 'X' || board[move - 1] == 'O') {
                std::cout << "Invalid move. Enter a number between 1 and 9: ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                break;
            }
        }
        board[move - 1] = opponent;

        played_moves++;
        print_board(board);

        if (check_win(board, opponent)) {
            std::cout << "I lost" << std::endl;
            return 0;
        }

        if (played_moves == 9) {
            std::cout << "DRAW" << std::endl;
            return 0;
        }
    }

    return 0;
}
