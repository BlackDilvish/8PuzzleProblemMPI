#include <cstdio>
#include <cstring>
#include <climits>
#include<vector>
#include<queue>
#include<array>
#include<algorithm>

#define N 3

std::array<std::array<int, N>, N> finalBoard = {{{{0,1,2}}, {{3,4,5}}, {{6,7,8}}}};

struct Node
{
    Node* parent;
 
    std::array<std::array<int, N>, N> mat;
 
    int x, y;
    int cost = INT_MAX;
    int level = 0;

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

    void printPath()
    {
        // printMatrix();

        // Node* par = parent;
        // while (par != nullptr)
        // {
        //     par->printMatrix();
        //     par = par->parent;
        // }
        printf("%d\n", level);
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