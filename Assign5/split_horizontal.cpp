#include <iostream>
#include <vector>
#include <climits>
#include <unordered_map>
#include <algorithm>
#include <set>

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
    vector <Edge> neigh;
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

};

class Network {
public:
    int numRouters;
    vector<Router> routers;
    vector<Edge> edges;
    set<int> upd;
    Network(int numRouters) : numRouters(numRouters) {
        for (int i = 0; i < numRouters; ++i) {
            Router router(i);
            router.initializeRoutingTable(numRouters);
            routers.push_back(router);
            upd.insert(i);
        }
    }

    void reinitupd()
    {
         for (int i = 0; i < numRouters; ++i) {
           
            upd.insert(i);
        }
    }

    void addEdge(int source, int destination, int cost) {

                routers[source].neigh.push_back({source, destination, cost});
                routers[destination].neigh.push_back({destination, source, cost});

       routers[source].routingTable[destination] = cost;
        routers[source].nextHop[destination] = destination;
        routers[destination].routingTable[source] = cost;
        routers[destination].nextHop[source] = source;
    }

    void removeEdge(int source, int destination) {
         // Remove the edge from the neighbor lists of the routers
    routers[source].neigh.erase(
        remove_if(routers[source].neigh.begin(), routers[source].neigh.end(),
            [destination](const Edge &e) { return e.destination == destination; }),
        routers[source].neigh.end()
    );

    routers[destination].neigh.erase(
        remove_if(routers[destination].neigh.begin(), routers[destination].neigh.end(),
            [source](const Edge &e) { return e.destination == source; }),
        routers[destination].neigh.end()
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
set <int> nextupd;
    // Initialize the temporary routing tables with the current values
    for (auto i:upd) {
        tempRoutingTables[i] = routers[i].routingTable;
        tempNextHops[i] = routers[i].nextHop;
    }

    // Iterate over all routers to update their routing tables based on neighbors
    for (auto i:upd) {
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
           tempNextHops[i][destination] = -1;  
           //Initializing the temporary cost and hop to inf and -1 so that we can find the minimum cost to a destination using the neighbours values and not the previous value

            // Iterate through the neighbors of the current router
            for (const auto &edge : routers[i].neigh) {
                    int neighborId = edge.destination;
                    int linkCost = edge.cost;
                                                  //                cout<<destination<<" "<<i<<" "<<neighborId<<" "<<routers[neighborId].nextHop[destination]<<" "<<routers[i].routingTable[destination]<<endl;

                   if (routers[neighborId].nextHop[destination] == i) {
                       
                                            //  cout<<i<<" "<< neighborId<<" "<<destination<<" "<<routers[i].routingTable[destination]<<endl;

                       continue;}  // Split Horizon check

                    // Use the neighbor's routing table to determine costs
                    int neighborCost = routers[neighborId].routingTable[destination];

                    // Skip if the neighbor's cost to the destination is INF
                    if (neighborCost == INF) continue;

                    // Calculate total cost through this neighbor
                    int newCost = neighborCost + linkCost;
                   // cout<<neighborCost<<" " <<linkCost<<" "<<newCost<<endl;

                    // Update the temporary routing table if the new cost is lower
                    if (newCost < tempRoutingTables[i][destination]) {
                        tempRoutingTables[i][destination] = newCost;
                        tempNextHops[i][destination] = neighborId;  // Update the next hop

                    
                }
            }
            if(tempy!= tempRoutingTables[i][destination])
            {
                    nextupd.insert(destination);
                globalUpdate=true;
            }

            
        }
    }

    // Commit updates from temporary tables back to the actual routing tables
    for (auto i :upd) {
        routers[i].routingTable = tempRoutingTables[i];
        routers[i].nextHop = tempNextHops[i];
    }
upd=nextupd;
    return globalUpdate;
}
bool isInPath(int startRouter, int destination,int inbetw) {
    int current = startRouter;
    while (current != destination && current != -1) {
        if (current == inbetw) return true; // Destination reached
        current = routers[current].nextHop[destination];

        if(current==startRouter)
        {
            return false;
        }
        if(current!=-1&&routers[current].nextHop[destination]!=-1&&current==routers[routers[current].nextHop[destination]].nextHop[destination])
        {
            return false;
        }
       // cout<<current<<endl;
    }
    return false; // Destination not reachable from startRouter
}

bool split_horizontal_Iteration() {
    bool globalUpdate = false;

    // Temporary routing tables to hold the updated values for this iteration
    vector<unordered_map<int, int>> tempRoutingTables(numRouters);
    vector<unordered_map<int, int>> tempNextHops(numRouters);
set <int> nextupd;

    // Initialize the temporary routing tables with the current values
    for (auto i:upd) {
        tempRoutingTables[i] = routers[i].routingTable;
        tempNextHops[i] = routers[i].nextHop;
    }

    // Iterate over all routers to update their routing tables based on neighbors
    for (auto i:upd) {
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
       //    tempNextHops[i][destination] = -1;  
           //Initializing the temporary cost and hop to inf and -1 so that we can find the minimum cost to a destination using the neighbours values and not the previous value

            // Iterate through the neighbors of the current router
            for (const auto &edge : routers[i].neigh) {
                    int neighborId = edge.destination;
                    int linkCost = edge.cost;

                       if (isInPath(neighborId, destination,i)) {
                       continue;}  // Split Horizon check

                    // Use the neighbor's routing table to determine costs
                    int neighborCost = routers[neighborId].routingTable[destination];

                    // Skip if the neighbor's cost to the destination is INF
                    if (neighborCost == INF){ 
                        tempNextHops[neighborId][destination]=-1; continue;}

                    // Calculate total cost through this neighbor
                    int newCost = neighborCost + linkCost;
                   // cout<<neighborCost<<" " <<linkCost<<" "<<newCost<<endl;

                    // Update the temporary routing table if the new cost is lower
                    if (newCost < tempRoutingTables[i][destination]) {
                        tempRoutingTables[i][destination] = newCost;
                       tempNextHops[i][destination] = neighborId;  // Update the next hop
                           nextupd.insert(destination);

                    }
                
            }
            if(tempy!= tempRoutingTables[i][destination])
            {
    // if(tempy< tempRoutingTables[i][destination])
    // {
    //     tempRoutingTables[i][destination]=INF;
    //      tempNextHops[i][destination] = -1;
    //    for(auto y: routers[i].neigh)
    //    {
    //        if(y!=destination){
    //        tempRoutingTables[y][destination]=INF;
    //      tempNextHops[y][destination] = -1;
    //        }
    //    }
       
    // }
                globalUpdate=true;
            }
// if( tempRoutingTables[i][destination]==INF)
// {
//     tempNextHops[i][destination] =-1;
// }
            
        }
    }

    // Commit updates from temporary tables back to the actual routing tables
    for (auto i:upd) {
        routers[i].routingTable = tempRoutingTables[i];
        routers[i].nextHop = tempNextHops[i];
    }
upd=nextupd;
    return globalUpdate;
}

    bool detectCountToInfinityProblem() const {
        for (const auto &router : routers) {
            for (const auto &entry : router.routingTable) {
                if (entry.second > 100 &&entry.second!=INF) {
                    
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

    // network.addEdge(0, 1, 2);
    // network.addEdge(0, 2, 4);
    // network.addEdge(1, 2, 1);
    // network.addEdge(1, 3, 7);
    // network.addEdge(2, 4, 3);
    // network.addEdge(3, 4, 1);
network.addEdge(0, 1, 3);
    network.addEdge(0, 2, 5);
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
        network.reinitupd();
    // Run DVR again and detect count-to-infinity
    iteration = 0;
    while (true) {
        
        cout << "Iteration " << iteration << " after link failure:\n";
        bool updated = network.split_horizontal_Iteration();
        
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
