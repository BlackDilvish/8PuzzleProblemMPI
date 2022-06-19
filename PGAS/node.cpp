#include "node.h"
#include <fstream>
#include <string>

std::array<std::array<int, N_ARR>, N_ARR> Node::finalBoard = {{{{0,1,2}}, {{3,4,5}}, {{6,7,8}}}};

int isSafe(int x, int y)
{
    return (x >= 0 && x < N_ARR && y >= 0 && y < N_ARR);
}

std::array<std::array<int, N_ARR>, N_ARR> getInputArray(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in);
    std::string line;
    std::array<std::array<int, 3>, 3> initial{};

    int i = 0;
    while (std::getline(file, line))
    {
        int j = 0;
        for (char c : line)
        {
            if (c != ' ')
            {
                initial[i][j++] = c - '0';
            }
        }
        i++;
    }

    return initial;
}

std::pair<int, int> getBlankPosition(const std::array<std::array<int, N_ARR>, N_ARR>& board)
{
    for (size_t i=0; i<board.size(); i++)
        for (size_t j=0; j<board[i].size(); j++)
            if (board[i][j] == 0)
                return std::make_pair(i, j);
                
    return std::make_pair(-1, -1);
}