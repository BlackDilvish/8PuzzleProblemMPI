#pragma once

#include <cstdio>
#include <cstring>
#include <climits>
#include <vector>
#include <queue>
#include <string>
#include <array>
#include <algorithm>

#define N 3

std::array<std::array<int, N>, N> stringToArray(const std::string& str);
std::string arrayToString(const std::array<std::array<int, N>, N>& array);

struct Node
{ 
    //std::array<std::array<int, N>, N> mat;
    std::string matString;
 
    int x, y;
    int cost = INT_MAX;
    int level = 0;

    static std::array<std::array<int, N>, N> finalBoard;

    Node(const std::array<std::array<int, N>, N>& initial, int blankX, int blankY)
    {
        // mat = initial;
        matString = arrayToString(initial);
        x = blankX;
        y = blankY;
    }

    Node(const Node&) = default;

    Node copy()
    {
        // Node node(mat, x, y);
        Node node(stringToArray(matString), x, y);

        node.cost = INT_MAX;
        node.level = level;

        return node;
    }

    void swapBlank(int oldX, int oldY, int newX, int newY)
    {
        x = newX;
        y = newY;
        auto mat = stringToArray(matString);
        std::swap(mat[oldX][oldY], mat[newX][newY]);
        matString = arrayToString(mat);
        level++;
    }

    void printMatrix()
    {
        auto mat = stringToArray(matString);
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
                printf("%d ", mat[i][j]);
            printf("\n");
        }
        printf("\n");
    }

    void calculateCost()
    {
        auto mat = stringToArray(matString);
        cost = 0;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                if (mat[i][j] && mat[i][j] != finalBoard[i][j])
                    cost++;
    }

    bool isSafe(int x, int y)
    {
        return (x >= 0 && x < N && y >= 0 && y < N);
    }

    std::string path()
    {
        return "Number of levels from start: " + std::to_string(level);
    }

    void serialize(int* data)
    {
        auto mat = stringToArray(matString);
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                data[i*N + j] = mat[i][j];
            }

        data[9] = x;
        data[10] = y;
        data[11] = cost;
        data[12] = level;
    }

    void deserialize(int* data)
    {
        auto mat = stringToArray(matString);
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                mat[i][j] = data[i*N + j];
            }

        matString = arrayToString(mat);

        x = data[9];
        y = data[10];
        cost = data[11];
        level = data[12];
    }

    friend bool operator==(const Node& node1, const Node& node2)
    {
        auto mat1 = stringToArray(node1.matString);
        auto mat2 = stringToArray(node2.matString);
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
                if (mat1[i][j] != mat2[i][j])
                {
                    return false;
                }
        }

        return true;
    }

    friend bool operator>(const Node& node1, const Node& node2)
    {
        return (node1.cost + node1.level) > (node2.cost + node2.level);
    }
};

int isSafe(int x, int y);
std::array<std::array<int, N>, N> getInputArray(const std::string& filename);
std::pair<int, int> getBlankPosition(const std::array<std::array<int, N>, N>& board);