#include <cstdio>
#include <cstring>
#include <climits>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include "node.h"

std::vector<Node> visitedNodes;

int row[] = { 1, 0, -1, 0 };
int col[] = { 0, -1, 0, 1 };

int isSafe(int x, int y)
{
    return (x >= 0 && x < N && y >= 0 && y < N);
}
 
void solve(const std::array<std::array<int, N>, N>& initial, int x, int y)
{
    //std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::vector<Node> pq;
 
    Node root = Node(initial, x, y);
    root.calculateCost();
 
    pq.push_back(root);
    visitedNodes.push_back(root);
 
    while (!pq.empty())
    {
        Node min = pq.back();
 
        pq.pop_back();
        if (visitedNodes.size()%1000 == 0) printf("%d\n", visitedNodes.size());
 
        if (min.cost == 0)
        {
            min.printPath();
            return;
        }
 
        for (int i = 0; i < 4; i++)
        {
            if (isSafe(min.x + row[i], min.y + col[i]))
            {        
                Node child = min.copy();
                child.swapBlank(min.x, min.y, min.x + row[i], min.y + col[i]);
                child.calculateCost(); 

                if ((std::find(visitedNodes.begin(), visitedNodes.end(), child) == visitedNodes.end()))
                {
                    pq.push_back(child);
                    visitedNodes.push_back(child);
                }
            }
        }
    }
}
 
int main()
{
    std::array<std::array<int, N>, N> initial = {{{{1,2,3}}, {{5,6,0}}, {{7,8,4}}}};
 
    int final[N][N] =
    {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 0}
    };
 
    int x = 1, y = 2;
    solve(initial, x, y);
 
    return 0;
}