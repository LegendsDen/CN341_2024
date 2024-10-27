#include <iostream>
#include <vector>
#include <climits>
#include <unordered_map>
#include <algorithm>

using namespace std;

const int INF = INT_MAX;

struct Edge {
    int source, destination, cost;
};

class Router {
public:
    int id;
    unordered_map<int, int> routingTable; // Destination -> Cost
    unordered_map<int, int> nextHop;      // Destination -> Next Hop

    Router(int id) : id(id) {}

    void initializeRoutingTable(int numRouters) {
        for (int i = 0; i < numRouters; ++i) {
            routingTable[i] = (i == id) ? 0 : INF;
            nextHop[i] = (i == id) ? id : -1;
        }
    }

    void printRoutingTable() const {
        cout << "Routing Table for Router " << id << ":\n";
        for (const auto &entry : routingTable) {
            cout << "To " << entry.first << " via " << nextHop.at(entry.first) 
                 << " Cost: " << (entry.second == INF ? "INF" : to_string(entry.second)) << endl;
        }
        cout << endl;
    }

bool updateRoutingTable(int neighborId, const unordered_map<int, int> &neighborRoutingTable, int linkCost, unordered_map<int, int> &tempRoutingTable, unordered_map<int, int> &tempNextHop) {
    unordered_map<int, int> minCostTable;  // Temporary storage to find the minimum cost for each destination
    unordered_map<int, int> bestNextHop;   // Track the best next hop for each destination

    for (const auto &entry : neighborRoutingTable) {
        int destination = entry.first;

        // Skip if the neighbor’s cost to the destination is INF
        if (entry.second == INF) continue;

        // Calculate new cost through this neighbor
        int newCost = entry.second + linkCost;

        // If it's the first time seeing this destination, or if the new cost is lower, update minCostTable
        if (minCostTable.find(destination) == minCostTable.end() || newCost < minCostTable[destination]) {
            minCostTable[destination] = newCost;
            bestNextHop[destination] = neighborId;
        }
    }

    // Update the tempRoutingTable with the minimum cost found and check if any updates occurred
    bool updated = false;
    for (const auto &entry : minCostTable) {
        int destination = entry.first;
        int minCost = entry.second;
        int hop = bestNextHop[destination];

        // Only update if the minimum cost or next hop has changed
        if (tempRoutingTable[destination] != minCost || tempNextHop[destination] != hop) {
            tempRoutingTable[destination] = minCost;
            tempNextHop[destination] = hop;
            updated = true;
        }
    }

    return updated;
}

};

class Network {
public:
    int numRouters;
    vector<Router> routers;
    vector<Edge> edges;

    Network(int numRouters) : numRouters(numRouters) {
        for (int i = 0; i < numRouters; ++i) {
            Router router(i);
            router.initializeRoutingTable(numRouters);
            routers.push_back(router);
        }
    }

    void addEdge(int source, int destination, int cost) {
        edges.push_back({source, destination, cost});
                edges.push_back({destination, source, cost});

        routers[source].routingTable[destination] = cost;
        routers[source].nextHop[destination] = destination;
        routers[destination].routingTable[source] = cost;
        routers[destination].nextHop[source] = source;
    }

    void removeEdge(int source, int destination) {
        edges.erase(
            remove_if(edges.begin(), edges.end(), [source, destination](const Edge &e) {
                return (e.source == source && e.destination == destination) ||
                       (e.source == destination && e.destination == source);
            }),
            edges.end()
        );

        // Set the costs to infinity for removed connections
        routers[source].routingTable[destination] = INF;
        routers[source].nextHop[destination] = -1;
        routers[destination].routingTable[source] = INF;
        routers[destination].nextHop[source] = -1;
    }

    void printAllRoutingTables() const {
        for (const auto &router : routers) {
            router.printRoutingTable();
        }
    }

bool runIteration() {
    bool globalUpdate = false;

    // Temporary routing tables to hold the updated values for this iteration
    vector<unordered_map<int, int>> tempRoutingTables(numRouters);
    vector<unordered_map<int, int>> tempNextHops(numRouters);

    // Initialize the temporary routing tables with the current values
    for (int i = 0; i < numRouters; ++i) {
        tempRoutingTables[i] = routers[i].routingTable;
        tempNextHops[i] = routers[i].nextHop;
    }

    // Iterate over all routers to update their routing tables based on neighbors
    for (int i = 0; i < numRouters; ++i) {
        Router &currentRouter = routers[i];

        // For each neighbor, check its routing table
        for (const auto &entry : currentRouter.routingTable) {
            int destination = entry.first;
            int currentCost = entry.second;
            int tempy=tempRoutingTables[i][destination];
            int tempyHops= tempNextHops[i][destination];
            
            // Skip if the cost from this router to itself is zero or INF
            if (destination == i ) continue;
           tempRoutingTables[i][destination]=INF;
            // Iterate through the neighbors of the current router
            for (const auto &edge : edges) {
                if (edge.source == i) {  // If this edge is from the current router
                    int neighborId = edge.destination;
                    int linkCost = edge.cost;

                    // Use the neighbor's routing table to determine costs
                    int neighborCost = routers[neighborId].routingTable[destination];

                    // Skip if the neighbor's cost to the destination is INF
                    if (neighborCost == INF) continue;

                    // Calculate total cost through this neighbor
                    int newCost = neighborCost + linkCost;

                    // Update the temporary routing table if the new cost is lower
                    if (newCost < tempRoutingTables[i][destination]) {
                        tempRoutingTables[i][destination] = newCost;
                        tempNextHops[i][destination] = neighborId;  // Update the next hop
                    }
                }
            }
            if(tempy!= tempRoutingTables[i][destination])
            {
                globalUpdate=true;
            }

            
        }
    }

    // Commit updates from temporary tables back to the actual routing tables
    for (int i = 0; i < numRouters; ++i) {
        routers[i].routingTable = tempRoutingTables[i];
        routers[i].nextHop = tempNextHops[i];
    }

    return globalUpdate;
}

    bool detectCountToInfinityProblem() const {
        for (const auto &router : routers) {
            for (const auto &entry : router.routingTable) {
                if (entry.second > 100&&entry.second!=INF) {
                    
                    cout << "Router " << router.id << " is experiencing the count-to-infinity problem for destination " << entry.first << " with cost " << entry.second << endl;
                    return true;
                }
            }
        }
        return false;
    }
};

int main() {
        freopen("input.txt","r",stdin);
    freopen("output.txt","w",stdout);
    int numRouters = 5;
    Network network(numRouters);

    network.addEdge(0, 1, 3);
    network.addEdge(0, 2, 15);
    network.addEdge(1, 2, 2);
    network.addEdge(2, 3, 1);
    network.addEdge(1, 3, 4);
    network.addEdge(3, 4, 6);
    cout << "Initial Routing Tables:\n";
    network.printAllRoutingTables();

    // Initial Convergence
    int iteration = 0;
    while (true) {
        cout << "Iteration " << iteration << ":\n";
        bool updated = network.runIteration();
        
        network.printAllRoutingTables();

        if (!updated) {
            cout << "No updates in iteration " << iteration << ". Algorithm has converged.\n";
            break;
        }
        
        iteration++;
    }

    // // Simulate link failure between nodes 3 and 4
    cout << "\nSimulating link failure between nodes 3 and 4...\n";
    network.removeEdge(3, 4);
        cout << "\nNew intial table\n";

        network.printAllRoutingTables();

    // Run DVR again and detect count-to-infinity
    iteration = 0;
    while (true) {
        cout << "Iteration " << iteration << " after link failure:\n";
        bool updated = network.runIteration();
        
        network.printAllRoutingTables();
       bool cti= network.detectCountToInfinityProblem();
       if(cti )
    {
        break;
    }
        
        if (!updated) {
            cout << "No updates in iteration " << iteration << ". Algorithm has converged.\n";
            break;
        }
 
        iteration++;
    }

    return 0;
}