#include <cstdio>
#include <cstring>
#include <climits>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <chrono>
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
 
void solve(const std::array<std::array<int, N>, N>& initial, int x, int y)
{
    int numprocs, myid, request = 1;
    int data[DATASIZE];
    MPI_Status status;

    
    MPI_Comm world = MPI_COMM_WORLD;
    MPI_Comm_size( world, &numprocs );
    MPI_Comm_rank( world, &myid );


    std::vector<Node> pq;
 
    if (myid == MASTER) 
    {
        Node root = Node(initial, x, y);
        root.calculateCost();
        pq.push_back(root);
        visitedNodes.push_back(root);
    }

    int foundSolution = 0, receivedSolution = 0;
    const int workPerCheck = 100;
    int workCounter = workPerCheck;
 
    while (!foundSolution)
    {
        if (myid == MASTER && visitedNodes.size()%1000 == 0) printf("%d\n", visitedNodes.size());
    
        while (!pq.empty() && workCounter)
        {
            workCounter--;
            Node min = pq.back();
    
            pq.pop_back();
            if (visitedNodes.size()%1000 == 0) printf("%d\n", visitedNodes.size());
    
            if (min.cost == 0)
            {
                min.printPath();
                foundSolution = myid;
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
                    pq[0].serialize(data);
                    MPI_Send( data, DATASIZE, MPI_INT, status.MPI_SOURCE, REPLY, world );
                }
            }
        }
        else
        {
            MPI_Send( &request, 1, MPI_INT, MASTER, REQUEST, world );
            MPI_Recv( data, DATASIZE, MPI_INT, MASTER, REPLY, world, &status );

            //add new node to stack
            Node newNode = Node(initial, x, y);
            newNode.deserialize(data);
            pq.push_back(newNode);
            visitedNodes.push_back(newNode);
        }
    }

    if (myid == MASTER)
    {
        printf("%d", receivedSolution);
    }
}
 
int main(int argc, char **argv)
{
    MPI_Init( &argc, &argv );
    
    //std::array<std::array<int, N>, N> initial = {{{{1,2,3}}, {{5,6,0}}, {{7,8,4}}}};
    std::array<std::array<int, N>, N> initial = {{{{1,5,2}}, {{4,3,0}}, {{7,8,6}}}};
 
    int final[N][N] =
    {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 0}
    };
 
    int x = 1, y = 2;
    
    auto start = std::chrono::steady_clock::now();
    solve(initial, x, y);
    auto end = std::chrono::steady_clock::now();
    printf("Elapsed time in seconds: %u s", std::chrono::duration_cast<std::chrono::seconds>(end - start).count()); 
    
    MPI_Finalize();
 
    return 0;
}