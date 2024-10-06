// Rinnegan
#include<bits/stdc++.h>
#pragma GCC optimize("Ofast")
#pragma GCC target("avx,avx2,fma")
#pragma GCC optimization("unroll-loops")
using namespace std;
#include <ext/pb_ds/assoc_container.hpp> 
#include <ext/pb_ds/tree_policy.hpp>
using namespace __gnu_pbds; 
 #define ordered_set tree<ll, null_type,less<ll>, rb_tree_tag,tree_order_statistics_node_update>
//order_of_key (k) : Number of items strictly smaller than k 
//find_by_order(k) : K-th element in a set (counting from zero)
typedef long long ll;
#define vi vector <ll>
#define pa pair<ll,ll>
#define pb push_back
#define Bits __builtin_popcountll
#define rev(x) reverse (x.begin(),x.end());
#define print1(x,n) for(int i=1;i<=n;i++) cout<<x[i]<<" "; cout<<endl;
#define inpu(x,n) for(int i=0;i<n;i++) cin>>x[i]
#define inpu1(x,n) for(int i=1;i<=n;i++) cin>>x[i]
#define endl "\n"
ll zero=0;
#define na cout<<"NO"<<endl;
#define ya cout<<"YES"<<endl;
#define so(x) sort (x.begin(),x.end());
#define sob(x) sort (x.rbegin(),x.rend());
#define sz(x) static_cast<int>((x).size())
const ll inf=1e18+5;
const ll mod=1e9+7;
const ll N=2e5+5;


const ll A=1e9+42;
const ll B=1e9+69;
#ifndef ONLINE_JUDGE
#define debug(x) cerr<<#x<<"--->"; _print(x);cerr<<endl;
#else
#define debug(x)
#endif

void _print(int t) {cerr << t;}
void _print(ll t) {cerr << t;}
void _print(string t) {cerr << t;}
void _print(char t) {cerr << t;}
void _print(double t) {cerr << t;}

template <class T> void _print(set <T> v);
template <class T, class V> void _print(map <T, V> v);
template <class T> void _print(multiset <T> v);
template <class T> void _print(vector <T> v);
template <class T>void _print(vector <vector<T>> v) { cerr << "(\n"; for (vector<T> i : v) {cerr << "  ";  _print(i); cerr << "\n";}cerr << ")";}
template <class T, class V> void _print(pair <T, V> p) {cerr << "{"; _print(p.first); cerr << ","; _print(p.second); cerr << "}";}
template <class T> void _print(vector <T> v) {cerr << "[ "; for (T i : v) {_print(i); cerr << " ";} cerr << "]";}
template <class T> void _print(set <T> v) {cerr << "[ "; for (T i : v) {_print(i); cerr << " ";} cerr << "]";}
template <class T> void _print(multiset <T> v) {cerr << "[ "; for (T i : v) {_print(i); cerr << " ";} cerr << "]";}
template <class T, class V> void _print(map <T, V> v) {cerr << "[ "; for (auto i : v) {_print(i); cerr << " ";} cerr << "]";}
template <class T> istream &operator>>(istream &in, vector<T> &v) {for (auto &elem : v) { in >> elem;}return in;}



////////////////////////////////////////////////SOLVE//////////////////////////////////////////////////////////////////////////
int priority(){
    int j=rand()%3;
    debug(j+1);
    return j+1;

}
int bursty(){
    int j=rand()%2;
    return j*10;
}
int j;
int see;
// Constants
const int NUM_PORTS = 8;      // Number of input/output ports
const int BUFFER_SIZE = 64;   // Buffer size per input/output port
const int SIMULATION_TIME = 100;  // Simulation time units
const int TRAFFIC_INTERVAL = 5;  // Interval for generating traffic
// Packet structure
struct Packet {
    int id;          // Packet ID
    int priority;    // Priority of the packet (0 for low, 1 for high)
    int arrivalTime; // Time when packet arrives
    int inputPort;   // The input port from which the packet arrived
    int outputPort;  // The output port to which the packet is directed
};

class RouterSwitchFabric {
private:
    vector<vector<queue<Packet>>>InputQueues;
    vector<int>Input_Match,Output_Match;
    vector<queue<Packet>> outputQueues;  // Output queues (one per output port)
    int packetCount;
    int packetDropped;

    // Statistics
    int totalPacketsProcessed;
    int totalTurnaroundTime;
    int totalWaitingTime;   
    int totalBufferOccupancy;

    // Mutex and condition variable for thread synchronization
    mutex mtx;

public:
    RouterSwitchFabric() : InputQueues(NUM_PORTS,vector<queue<Packet>>(NUM_PORTS)), outputQueues(NUM_PORTS),Input_Match(NUM_PORTS),Output_Match(NUM_PORTS) {
        packetCount = 0;
        packetDropped = 0;
        totalPacketsProcessed = 0;
        totalTurnaroundTime = 0;
        totalWaitingTime = 0;
        totalBufferOccupancy = 0;
       
    }

    // Generate packets for input queues with variable traffic conditions
    void generateTraffic(int time, bool uniformTraffic, bool nonUniformTraffic, bool burstyTraffic) {
            cout<<"high"<<endl;
        while (time < SIMULATION_TIME) {
            // cout<<"lambda" <<time<<" ";
            {
                unique_lock<mutex> lock(mtx);
                for (int i = 0; i < NUM_PORTS; i++) {

                    int numPackets = 5;
                    if(burstyTraffic&&i<4){
                        numPackets=bursty();
                    }
                    // Create new packets and add them to input queues
                    for (int j = 0; j < numPackets; ++j) {
                        Packet p;
                        see++;
                        p.id = packetCount++;
                        if(nonUniformTraffic&&i<4) p.priority=3;
                        else p.priority =  priority();// Higher priority for ports 0-3
                        p.arrivalTime = time;
                        p.inputPort = i;
                        p.outputPort = rand() % NUM_PORTS;
                        // debug(p.outputPort)

                        if (InputQueues[i][p.outputPort].size() < BUFFER_SIZE) {
                            InputQueues[i][p.outputPort].push(p);
                        } else {
                            debug("yes")
                            packetDropped++; // Drop packet if buffer is full
                        }
                    }
                }

            }

            this_thread::sleep_for(chrono::milliseconds(TRAFFIC_INTERVAL));
            time += TRAFFIC_INTERVAL;
        }
    }

    // Simulate packet processing for priority scheduling
    void processPackets(int time) {
        cout<<"low"<<endl;

        while (time < SIMULATION_TIME) {
            // set<int>s;
            vector<int>temp(NUM_PORTS,-1);
            unique_lock<mutex> lock(mtx); // Lock mutex for thread safety
            for(int i=0;i<NUM_PORTS;i++){// output
                set<int>output;
                for(int j=0;j<NUM_PORTS;j++){
                    if(!InputQueues[j][i].empty()){
                        output.insert(j);
                    }
                }
                if(!output.empty()){
                    //choose closest to Output_Match[i]
                    if(output.lower_bound(Output_Match[i])!=output.end()){
                        temp[i]=*output.lower_bound(Output_Match[i]);
                        Output_Match[i]=(temp[i]+1)%NUM_PORTS;  //for RR
                    }
                    else{
                        temp[i]=*output.begin();
                        Output_Match[i]=(temp[i]+1)%NUM_PORTS;  // for RR
                    }
                }
            }
            set<int>s[NUM_PORTS];
            for(int i=0;i<NUM_PORTS;i++){
                if(temp[i]!=-1){
                    s[temp[i]].insert(i);
                }
            }
            for(int i=0;i<NUM_PORTS;i++){
                if(!s[i].empty()){
                    Packet p;
                    int val;
                    if(s[i].lower_bound(Input_Match[i])!=s[i].end()){
                        val=*s[i].lower_bound(Input_Match[i]); 
                    }
                    else{
                        val=*s[i].begin();  
                    }
                    Input_Match[i]=(val+1)%NUM_PORTS;
                    // Output_Match[val]=(i+1)%NUM_PORTS;// for iSLIP 
                    p=InputQueues[i][val].front();
                    InputQueues[i][val].pop(); 
                    if (outputQueues[val].size() < BUFFER_SIZE) {
                            outputQueues[val].push(p);

                            // Update statistics
                            totalPacketsProcessed++;
                            int turnaroundTime = time - p.arrivalTime;
                            totalTurnaroundTime += turnaroundTime;
                            totalWaitingTime += turnaroundTime - 1; // Assume 1 unit time for processing
                            totalBufferOccupancy += outputQueues[val].size();
                    } else {
                        debug("no")
                        
                            packetDropped++; // Drop packet if output buffer is full
                    }



                }
                

            }
            if(time%5==1){
                for(int i=0;i<NUM_PORTS;i++){
                    if(!outputQueues[i].empty()){
                        cout<<i<<"---->"<<outputQueues[i].size()<<" ";
                        outputQueues[i].pop();

                    } 
                }
                cout<<endl;
            }
            this_thread::sleep_for(chrono::milliseconds(1));
            time += 1;
        }
    }

    // Print statistics for the simulation
    void printStatistics(int time) {
        cout << "Simulation Time: " << time << endl;
        cout << "Total Packets Processed: " << totalPacketsProcessed << endl;
        cout << "Total Packets Dropped: " << packetDropped << endl;
        cout << "Average Turnaround Time: " << (totalPacketsProcessed > 0 ? (totalTurnaroundTime / totalPacketsProcessed) : 0) << endl;
        cout << "Average Waiting Time: " << (totalPacketsProcessed > 0 ? (totalWaitingTime / totalPacketsProcessed) : 0) << endl;
        cout << "Average Buffer Occupancy: " << (totalPacketsProcessed > 0 ? (totalBufferOccupancy / totalPacketsProcessed) : 0) << endl;
        // cout << "Packet Drop Rate: " <<((packetDropped*100)/(packetDropped+totalPacketsProcessed));
    }
    void display(){
        for(int i=0;i<NUM_PORTS;i++){
            for(int j=0;j<NUM_PORTS;j++){
                cout<<InputQueues[i][j].size()<<" ";
            }
            cout<<endl;

        }
        for(int i=0;i<NUM_PORTS;i++){
            cout<<outputQueues[i].size()<<" ";
        }
    }
};

signed main() {
    freopen("output.txt","w",stdout);
    freopen("error.txt","w",stderr);
    srand(time(0));  // Seed random number generator

    // Simulation conditions: uniform, non-uniform, bursty traffic
    bool uniformTraffic = true;
    bool nonUniformTraffic =false;
    bool burstyTraffic = false;

    // Create router switch fabric
    RouterSwitchFabric router;

    // Threads for generating traffic and processing packets
    thread trafficThread([&router, &uniformTraffic, &nonUniformTraffic, &burstyTraffic]() {
        int time = 0;
        router.generateTraffic(time, uniformTraffic, nonUniformTraffic, burstyTraffic);
    });

    thread processingThread([&router]() {
        int time = 0;
        router.processPackets(time);
    });

    // Run both threads
    trafficThread.join();
    processingThread.join();

    // Print the final statistics after the simulation
    router.printStatistics(SIMULATION_TIME);
    debug(see)
    router.display();

    return 0;
}
