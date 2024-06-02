#ifndef TTT_HPP
#define TTT_HPP

#include <vector>
#include <string>

void print_board(const std::vector<char> &board);
bool check_win(const std::vector<char> &board, char player);
void play_ttt(const std::string &strategy);

#endif
// part 1: ./ttt 123456789
// part 2: ./mync -e "ttt 123456789"
// part 3: ./mync -e "ttt 123456789 -i TCPS4050 == nc localhost 4050
//./mync -e "ttt 123456789 -o TCPS4050 == nc localhost 4050
//./mync -e "ttt 123456789 -b TCPS4050 == nc localhost 4050
//./mync -e "ttt 123456789" -i TCPS4050 -o TCPClocalhost,4455 == nc -l 4455 & nc localhost 4050
// part 3.5: ./mync -i TCPS5000 == ./mync -i TCPClocalhost,5000
// part 4: ./mync -e "ttt 123456789" -i UDPS4050 == nc -u localhost 4050
//./mync -e "ttt 123456789" -i UDPS4050 -o TCPClocalhost,4455 == nc -u localhost 4050 & nc -l 4455
// part 5: ./mync -e "ttt 123456789" -b TCPMUXS4050 == nc localhost 4050
// part 6 ./mync -e "ttt 987654321" -i UDSSS/tmp/sock == nc -U /tmp/sock
//./mync -e "ttt 987456321" -i UDSSD/tmp/sck == nc -U -u /tmp/sck
