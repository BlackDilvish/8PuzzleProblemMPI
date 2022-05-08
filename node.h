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

struct Node
{
    Node* parent;
 
    std::array<std::array<int, N>, N> mat;
 
    int x, y;
    int cost = INT_MAX;
    int level = 0;

    static std::array<std::array<int, N>, N> finalBoard;

    Node(const std::array<std::array<int, N>, N>& initial, int blankX, int blankY)
    {
        mat = initial;
        x = blankX;
        y = blankY;
    }

    Node copy()
    {
        Node node(mat, x, y);

        node.parent = this;
        node.cost = INT_MAX;
        node.level = level;

        return node;
    }

    void swapBlank(int oldX, int oldY, int newX, int newY)
    {
        x = newX;
        y = newY;
        std::swap(mat[oldX][oldY], mat[newX][newY]);
        level++;
    }

    void printMatrix()
    {
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
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                mat[i][j] = data[i*N + j];
            }

        x = data[9];
        y = data[10];
        cost = data[11];
        level = data[12];
    }

    friend bool operator==(const Node& node1, const Node& node2)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
                if (node1.mat[i][j] != node2.mat[i][j])
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