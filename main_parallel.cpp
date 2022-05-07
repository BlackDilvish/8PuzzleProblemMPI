#include <cstdio>
#include <cstring>
#include <climits>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <chrono>
#include <omp.h>
#include <string>
#include <fstream>
#include "node.h"
#include "mpi.h"

#define MASTER    0
#define REQUEST   1
#define REPLY     2
#define DATASIZE 13

std::vector<Node> visitedNodes;

int row[] = { 1, 0, -1, 0 };
int col[] = { 0, -1, 0, 1 };

int isSafe(int x, int y)
{
    return (x >= 0 && x < N && y >= 0 && y < N);
}

std::array<std::array<int, N>, N> getInputArray(const std::string& filename)
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
 
void solve(const std::array<std::array<int, N>, N>& initial, int x, int y)
{
    int numprocs, myid, request = 1;
    int data[DATASIZE];
    MPI_Status status;
    auto start = std::chrono::steady_clock::now();

    
    MPI_Comm world = MPI_COMM_WORLD;
    MPI_Comm_size( world, &numprocs );
    MPI_Comm_rank( world, &myid );

    std::vector<Node> nodesStack;
 
    if (myid == MASTER) 
    {
        Node root = Node(initial, x, y);
        root.calculateCost();
        nodesStack.push_back(root);
        visitedNodes.push_back(root);
    }

    int foundSolution = 0, receivedSolution = 0;
    const int workPerCheck = 1000;
    int workCounter = workPerCheck;
 
    while (!foundSolution)
    {
    
        while (!nodesStack.empty() && workCounter)
        {
            workCounter--;
            Node min = nodesStack.back();
    
            nodesStack.pop_back();
            if (visitedNodes.size()%1000 == 0) printf("%lu\n", visitedNodes.size());
    
            if (min.cost == 0)
            {
                min.printPath();
                foundSolution = myid + 1;
                printf("Found!\n");
                break;
            }
    
            #pragma omp parallel for
            for (int i = 0; i < 4; i++)
            {
                if (isSafe(min.x + row[i], min.y + col[i]))
                {        
                    Node child = min.copy();
                    child.swapBlank(min.x, min.y, min.x + row[i], min.y + col[i]);
                    child.calculateCost(); 

                    if ((std::find(visitedNodes.begin(), visitedNodes.end(), child) == visitedNodes.end()))
                    {
                        nodesStack.push_back(child);
                        visitedNodes.push_back(child);
                    }
                }
            }
        }

        //After some work
        //check if solution found by MPI_Allreduce
        MPI_Allreduce( &foundSolution, &receivedSolution, 1, MPI_INT, MPI_MAX, world);
        if (receivedSolution != 0)
        {
            break;
        }

        workCounter = workPerCheck;

        //handle job requests by MPI_Recv
        if (myid == MASTER)
        {
            for (int i=0; i<numprocs-1; i++)
            {
                MPI_Recv( &request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST, world, &status );
                if (request)
                {
                    //send data
                    nodesStack[0].serialize(data);
                    nodesStack.erase(nodesStack.begin());
                    MPI_Send( data, DATASIZE, MPI_INT, status.MPI_SOURCE, REPLY, world );
                }
            }
        }
        else
        {
            if (nodesStack.empty())
            {
              request = 1;
              MPI_Send( &request, 1, MPI_INT, MASTER, REQUEST, world );
              MPI_Recv( data, DATASIZE, MPI_INT, MASTER, REPLY, world, &status );
              
              //add new node to stack
              Node newNode = Node(initial, x, y);
              newNode.deserialize(data);
              nodesStack.push_back(newNode);
              visitedNodes.push_back(newNode);
            }
            else
            {
              request = 0;
              MPI_Send( &request, 1, MPI_INT, MASTER, REQUEST, world );
            }
        }
    }

    auto end = std::chrono::steady_clock::now();
    if (myid == MASTER)
    {
        printf("Solution found in: %ld seconds\n", std::chrono::duration_cast<std::chrono::seconds>(end - start).count()); 
    }
}
 
int main(int argc, char **argv)
{
    MPI_Init( &argc, &argv );
    
    std::array<std::array<int, N>, N> initial = getInputArray("input.txt");
 
    int x = 1, y = 2;
    
    solve(initial, x, y);
    
    MPI_Finalize();
 
    return 0;
}