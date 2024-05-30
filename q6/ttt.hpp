#ifndef TTT_HPP
#define TTT_HPP

#include <vector>
#include <string>

void print_board(const std::vector<char> &board);
bool check_win(const std::vector<char> &board, char player);
void play_ttt(const std::string &strategy);

#endif
