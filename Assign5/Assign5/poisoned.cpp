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
            routingTable[i] = (i == id) ? 0 : 1e9;
            nextHop[i] = (i == id) ? id : -1;
        }
    }

    void printRoutingTable() const {
        cout << "Routing Table for Router " << id << ":\n";
        for (const auto &entry : routingTable) {
            cout << "To " << entry.first << " via " << nextHop.at(entry.first)
                 << " Cost: " << (entry.second == 1e9 ? "1e9" : to_string(entry.second)) << endl;
        }
        cout << endl;
    }

    bool isRouterOnPath(int currentRouter, int neighborId, int destination, const vector<Router> &routers) {
        int next = routers[neighborId].nextHop.at(destination);
        while (next != destination) {
            if (next == currentRouter || next == -1) {
                return true;
            }
            next = routers[next].nextHop.at(destination);
        }
        return false;
    }

    unordered_map<int, pair<int, int>> calculateRoutingUpdates(int neighborId, const unordered_map<int, int> &neighborRoutingTable,int linkCost, const vector<Router> &routers) {
        
        unordered_map<int, pair<int, int>> updates;

        for (const auto &entry : neighborRoutingTable) {
            int destination = entry.first;
            if (linkCost == 1e9 && nextHop[destination] == neighborId) {
                // cout << "Setting distance to destination " << destination
                //     << " as infinite due to link failure with neighbor " << neighborId << ".\n";
                updates[destination] = {1e9, -1};  // Set cost to infinite and invalidate the route.
                continue;
            }
            if (entry.second == 1e9) {
                if (nextHop[destination] == neighborId) {
                    // cout << "Invalidating route to " << destination 
                    //      << " through neighbor " << neighborId << endl;
                    updates[destination]={1e9,-1};
                }
                continue;
            }
            int newCost = entry.second + linkCost;

            if (isRouterOnPath(id, neighborId, destination, routers)) {
                // cout << "Poisoned reverse detected: Setting cost to INF for router " 
                //      << id << " via " << neighborId << " to " << destination << endl;
                newCost = 1e9;
            }

            if (newCost < routingTable.at(destination)) {
                // cout << "Router " << id << " found better path to " 
                //      << destination << " via " << neighborId << " with cost " << newCost << endl;
                updates[destination] = {newCost, neighborId};
            }
        }
        return updates;
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
        for (auto &edge : edges) {
            if ((edge.source == source && edge.destination == destination) ||
                (edge.source == destination && edge.destination == source)) {
                edge.cost = 1e9;
                cout << "Link failure between " << source << " and " << destination << endl;
            }
        }

        routers[source].routingTable[destination] = 1e9;
        routers[source].nextHop[destination] = -1;
        routers[destination].routingTable[source] = 1e9;
        routers[destination].nextHop[source] = -1;
    }

    void printAllRoutingTables() const {
        for (const auto &router : routers) {
            router.printRoutingTable();
        }
    }

    bool runIteration() {
        bool globalUpdate = false;
        vector<unordered_map<int, pair<int, int>>> allUpdates(numRouters);

        for (int i = 0; i < numRouters; ++i) {
            Router &currentRouter = routers[i];

            for (const auto &edge : edges) {
                if (edge.source == i) {
                    int neighborId = edge.destination;
                    int linkCost = edge.cost;

                    auto updates = currentRouter.calculateRoutingUpdates(
                        neighborId, routers[neighborId].routingTable, linkCost, routers);

                    if (!updates.empty()) {
                        cout << "Updates detected for router " << i << endl;
                        globalUpdate = true;
                        allUpdates[i].insert(updates.begin(), updates.end());
                    }
                }
            }
        }

        for (int i = 0; i < numRouters; ++i) {
            for (const auto &update : allUpdates[i]) {
                int destination = update.first;
                int newCost = update.second.first;
                int newNextHop = update.second.second;

                // cout << "Applying update: Router " << i << " to " << destination 
                //      << " via " << newNextHop << " with cost " << newCost << endl;

                routers[i].routingTable[destination] = newCost;
                routers[i].nextHop[destination] = newNextHop;
            }
        }

        return globalUpdate;
    }
};

int main() {
    freopen("input.txt","r",stdin);
    freopen("output.txt","w",stdout);
    int numRouters = 3;
    Network network(numRouters);
    network.addEdge(0, 1, 4);
    network.addEdge(0, 2, 101);
    network.addEdge(1, 2, 1);


    // network.addEdge(0, 1, 3);
    // network.addEdge(0, 2, 5);
    // network.addEdge(1, 2, 2);
    // network.addEdge(2, 3, 1);
    // network.addEdge(1, 3, 4);
    // network.addEdge(3, 4, 6);

    cout << "Initial Routing Tables:\n";
    network.printAllRoutingTables();

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

    cout << "\nSimulating link failure between nodes 3 and 4...\n";
    network.removeEdge(0, 1);

    cout << "\nRouting Tables After Link Failure:\n";
    network.printAllRoutingTables();

    iteration = 0;
    while (true) {
        cout << "Iteration " << iteration << " after link failure:\n";
        bool updated = network.runIteration();

        network.printAllRoutingTables();

        if (!updated) {
            cout << "No updates in iteration " << iteration << ". Algorithm has converged.\n";
            break;
        }
        iteration++;
    }

    return 0;
}
