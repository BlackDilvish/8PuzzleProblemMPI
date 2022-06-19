#include <cstdio>
#include <cstring>
#include <climits>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <chrono>
#include <string>
#include <fstream>
#include "node.h"
#include <upcxx/upcxx.hpp>

#define MASTER    0
#define REQUEST   1
#define REPLY     2
#define DATASIZE 13

std::vector<Node> visitedNodes;

int row[] = { 1, 0, -1, 0 };
int col[] = { 0, -1, 0, 1 };
 
void solve(const std::array<std::array<int, N>, N>& initial)
{
    int numprocs, myid, request = 1;
    int data[DATASIZE];
    //MPI_Status status;
    std::string solutionPath;
    auto start = std::chrono::steady_clock::now();
    bool sharedInitialized = false;

    // 1. Initialize MPI
    //MPI_Comm world = MPI_COMM_WORLD;
    numprocs = upcxx::rank_n(); //MPI_Comm_size( world, &numprocs );
    myid = upcxx::rank_me(); //MPI_Comm_rank( world, &myid );

    std::vector<Node> nodesStack;
 
    // 2. Set first node
    if (myid == MASTER) 
    {
        auto [x, y] = getBlankPosition(initial);
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
        // 3. Do some work
        while (!nodesStack.empty() && workCounter)
        {
            workCounter--;
            Node min = nodesStack.back();
    
            nodesStack.pop_back();
            if (visitedNodes.size()%1000 == 0) printf("Proc %u: visited nodes = %lu\n", myid, visitedNodes.size());
    
            if (min.cost == 0)
            {
                printf("Found!\n");
                solutionPath = min.path();
                foundSolution = myid + 1;
                break;
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
                        nodesStack.push_back(child);
                        visitedNodes.push_back(child);
                    }
                }
            }
        }

        //4. After some work, check if solution was found by MPI_Allreduce
        upcxx::reduce_all(foundSolution, upcxx::op_fast_add).wait(); //MPI_Allreduce( &foundSolution, &receivedSolution, 1, MPI_INT, MPI_MAX, world);
        if (receivedSolution != 0)
        {
            break;
        }

        workCounter = workPerCheck;

        if (!sharedInitialized)
        {
            upcxx::global_ptr<Node> all_nodes_ptr = nullptr;
            //5. Handle job requests by MPI_Send and MPI_Recv
            if (myid == MASTER) // tak jak w reduce_to_rank0_global wsadzic do tablicy jakies node'y dla n procesow
            {
                // for (int i=0; i<numprocs-1; i++)
                // {
                //     MPI_Recv( &request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST, world, &status );
                //     if (request)
                //     {
                //         //Send data to workers
                //         nodesStack.front().serialize(data);
                //         nodesStack.erase(nodesStack.begin());
                //         MPI_Send( data, DATASIZE, MPI_INT, status.MPI_SOURCE, REPLY, world );
                //     }
                // }

                all_nodes_ptr = upcxx::new_array<Node>(upcxx::rank_n());
                all_nodes_ptr = upcxx::broadcast(all_nodes_ptr, 0).wait();

                for (int i=0; i<numprocs; i++)
                {
                    upcxx::global_ptr<Node> proc_node_ptr = all_nodes_ptr + i;

                    Node nodeToSend = nodesStack.front();
                    nodesStack.erase(nodesStack.begin());

                    upcxx::rput(nodeToSend, proc_node_ptr).wait();
                }
            }

            upcxx::barrier();

            if (myid != MASTER) // jak nie ma co robic to niech wyciagnie sobie z tablicy node'a
            {
                if (nodesStack.empty())
                {
                //   request = 1;
                //   MPI_Send( &request, 1, MPI_INT, MASTER, REQUEST, world );
                //   MPI_Recv( data, DATASIZE, MPI_INT, MASTER, REPLY, world, &status );
                
                //   //Add new node to stack
                //   Node newNode = Node(initial, 0, 0);
                //   newNode.deserialize(data);
                //   nodesStack.push_back(newNode);
                //   visitedNodes.push_back(newNode);
                    Node* local_nodes_ptrs = all_nodes_ptr.local();
                    Node newNode = local_nodes_ptrs[myid];
                    nodesStack.push_back(newNode);
                    visitedNodes.push_back(newNode);
                }
            }

            sharedInitialized = true;
            upcxx::delete_array(all_hits_ptr);
        }
        
    }

    //6. Save solution in output.txt
    auto end = std::chrono::steady_clock::now();
    if (myid == foundSolution-1)
    {
        std::ofstream file;
        file.open("output.txt");
        file << solutionPath << std::endl;
        file << "Solution found in: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() <<" seconds\n"; 
        file.close();
    }
    // upcxx::delete_array(all_hits_ptr);
}
 
int main(int argc, char **argv)
{
    upcxx::init(); //MPI_Init( &argc, &argv );
    
    std::array<std::array<int, N>, N> initial = getInputArray("input.txt");
    solve(initial);
    
    upcxx::finalize(); //MPI_Finalize();
 
    return 0;
}