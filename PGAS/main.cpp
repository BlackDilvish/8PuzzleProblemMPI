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

#define MASTER 0
#define REQUEST 1
#define REPLY 2
#define DATASIZE 13

std::vector<Node> visitedNodes;
upcxx::global_ptr<Node> all_nodes_ptr = nullptr;

int row[] = {1, 0, -1, 0};
int col[] = {0, -1, 0, 1};

void solve(const std::array<std::array<int, N_ARR>, N_ARR> &initial)
{
    int numprocs, myid, request = 1;
    int data[DATASIZE];
    std::string solutionPath;
    auto start = std::chrono::steady_clock::now();
    bool sharedInitialized = false;

    // 1. Initialize process id
    numprocs = upcxx::rank_n();
    myid = upcxx::rank_me();

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
            if (visitedNodes.size() % 1000 == 0)
                printf("Proc %u: visited nodes = %lu\n", myid, visitedNodes.size());

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
        upcxx::barrier();

        // 4. After some work, check if solution was found by upcxx::reduce_all
        receivedSolution = upcxx::reduce_all(foundSolution, upcxx::op_fast_add).wait();
        if (receivedSolution != 0)
        {
            break;
        }

        workCounter = workPerCheck;

        if (!sharedInitialized)
        {
            // 5. Add nodes for workers in shered memory
            if (myid == MASTER)
            {

                all_nodes_ptr = upcxx::new_array<Node>(upcxx::rank_n());
                for (int i = 0; i < numprocs; i++)
                {
                    upcxx::global_ptr<Node> proc_node_ptr = all_nodes_ptr + i;

                    Node nodeToSend = nodesStack.front();
                    nodesStack.erase(nodesStack.begin());

                    upcxx::rput(nodeToSend, proc_node_ptr).wait();
                }
            }

            all_nodes_ptr = upcxx::broadcast(all_nodes_ptr, 0).wait();

            upcxx::barrier();

            // 6. Receive nodes from master
            if (myid != MASTER)
            {
                if (nodesStack.empty())
                {
                    Node newNode = upcxx::rget(all_nodes_ptr + myid).wait();
                    newNode.is_expired = true;
                    upcxx::rput(newNode, all_nodes_ptr + myid).wait();
                    nodesStack.push_back(newNode);
                    visitedNodes.push_back(newNode);
                }
            }

            sharedInitialized = true;
        }
        else
        {
            // 7. Add new node if old is expired
            if (myid == MASTER)
            {
                for (int i = 0; i < numprocs; i++)
                {
                    upcxx::global_ptr<Node> proc_node_ptr = all_nodes_ptr + i;
                    Node node = upcxx::rget(proc_node_ptr).wait();

                    if (node.is_expired)
                    {
                        Node nodeToSend = nodesStack.front();
                        nodesStack.erase(nodesStack.begin());
                        upcxx::rput(nodeToSend, proc_node_ptr).wait();
                    }
                }
            }

            all_nodes_ptr = upcxx::broadcast(all_nodes_ptr, 0).wait();

            upcxx::barrier();

            // 8. If worker is not working, get new node from shared memory
            if (myid != MASTER)
            {
                if (nodesStack.empty())
                {
                    Node newNode = upcxx::rget(all_nodes_ptr + myid).wait();
                    newNode.is_expired = true;
                    upcxx::rput(newNode, all_nodes_ptr + myid).wait();
                    nodesStack.push_back(newNode);
                    visitedNodes.push_back(newNode);
                }
            }
        }
    }

    // 9. Save solution in output.txt
    auto end = std::chrono::steady_clock::now();
    if (myid == MASTER)
    {
        upcxx::delete_array(all_nodes_ptr);
    }
    if (myid == foundSolution - 1)
    {
        std::cout << "Saving solution...\n";
        std::ofstream file;
        file.open("output.txt");
        file << solutionPath << std::endl;
        file << "Solution found in: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds\n";
        file.close();
    }
}

int main(int argc, char **argv)
{
    upcxx::init();

    std::array<std::array<int, N_ARR>, N_ARR> initial = getInputArray("input.txt");
    solve(initial);

    upcxx::finalize();

    return 0;
}