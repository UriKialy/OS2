#include "q1.h"

int validet(int check)
{
    int arr[] = {};
    for (int i = 0; i < 9; i++)
    {
        int num = check % 10;
        if (arr[num] == 1)
        {
            return 0;
        }
        arr[num] = 1;
        check = check / 10;
    }
    return 1;
}

int reverse(int num)
{
    int res = 0;
    for (int i = 0; i < 9; i++)
    {
        res = res * 10;
        res += num % 10;
        num = num / 10;
    }
    return res;
}
int isGameFinished(int board[3][3])
{
    
    if ((board[0][0] == board[1][1] && board[2][2] == board[1][1] && board[0][0] == X)
    ||(board[0][2] == board[1][1] && board[2][0] == board[1][1] && board[0][2] == X))
    {
        return 0;
    }
    else if ((board[0][0] == board[1][1] && board[2][2] == board[1][1] && board[0][0] == O)
    ||(board[0][2] == board[1][1] && board[2][0] == board[1][1] && board[0][2] == O))
    {
        return 1;
    }
    for (int i = 0; i < 3; i++)
    {
            if ((board[i][0]==board[i][1] && board[i][1]==board[i][2] && board[i][0] == X)||
            (board[2][i]==board[1][i] && board[2][i]==board[0][i] && board[0][i] == X))
            {
                return 0;
            }
            else if ((board[i][0]==board[i][1] && board[i][1]==board[i][2] && board[i][0] == O)||
            (board[2][i]==board[1][i] && board[2][i]==board[0][i] && board[0][i] == O))
            {
                return 1;
            }
    }
    int check=0;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            if(board[i][j]==EMPTY){
                check++;
            }
        }
    }
    if(check==9){
        return -1;//tie
    }
    return 2; //not finished
}

// the user is O and the computer is X
void ttt(int strategy)
{
    if (validet(strategy) == 0)
    {
        printf("errorn");
        return;
    }
    strategy = reverse(strategy);
    int board[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            board[i][j] = EMPTY; // init the board
        }
    }
    size_t temp = strategy % 10;
    strategy = strategy / 10;
    int userChoise;
    int row = (temp - 1) / 3;
    int col = (temp - 1) % 3;
    board[row][col] = X;
    while (1)
    {

        printf("please enter a number between 1-9\n");
        scanf("%d", &userChoise);
        if (userChoise < 1 || userChoise > 9 || board[(userChoise - 1) / 3][(userChoise - 1) % 3] != EMPTY)
        {
            printf("please try again, invalid number\n");
            continue;
        }
        board[(userChoise - 1) / 3][(userChoise - 1) % 3] = O;
        if (isGameFinished(board) == 1)
        {
            printf("you won\n");
            return;
        }
        else if (isGameFinished(board) == 0)
        {
            printf("you lost\n");
            return;
        }
        else if (isGameFinished(board) == -1)
        {
            printf("it's a tie\n");
            return;
        }
        if (temp == 0)
        {
            printf("it's a tie\n");
            return;
        }
        temp = strategy % 10;
        strategy = strategy / 10;
        row = (temp - 1) / 3;
        col = (temp - 1) % 3;
        if (board[row][col] != EMPTY)
        {
            board[row][col] = X;
        }
        else
        {
            for (int i = 0; i < 8; i++){
                temp = strategy % 10;
                strategy = strategy / 10;
                row = (temp - 1) / 3;
                col = (temp - 1) % 3;
                if (board[row][col] == EMPTY)
                {
                    board[row][col] = X;
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("argv should contain 2 args\n");
        return 1;
    }
    if(strlen(argv[1])!=9){
        printf("argv should contain 9 digits\n");
        return 1;
    }
    return 0;
}