#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <zlib.h>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")  // Link with Ws2_32.lib
#define PORT 5000
#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10
using namespace std;

mutex client_mutex;  // Mutex to protect shared data



string decompress_data(const string& compressed_data) {
    string decompressed(BUFFER_SIZE, '\0');  // Initial buffer
    z_stream strm = {0};
    strm.total_in = strm.avail_in = compressed_data.size();
    strm.total_out = strm.avail_out = BUFFER_SIZE;
    strm.next_in = (Bytef *)compressed_data.data();
    strm.next_out = (Bytef *)decompressed.data();

    if (inflateInit(&strm) != Z_OK) {
        throw runtime_error("zlib: Failed to initialize decompression.");
    }

    int ret = inflate(&strm, Z_NO_FLUSH);
    if (ret != Z_STREAM_END && ret != Z_OK) {
        inflateEnd(&strm);
        throw runtime_error("zlib: Error during decompression.");
    }

    inflateEnd(&strm);
    decompressed.resize(strm.total_out);  // Resize to actual decompressed size
    return decompressed;
}

// Simulate varying network conditions by introducing delays and bandwidth limits
int get_network_delay(const string& condition) {
    if (condition == "poor") {
        return 2000; // 2 seconds delay for poor conditions
    } else if (condition == "normal") {
        return 500; // 100 milliseconds for normal conditions
    } else if (condition == "strong") {
        return 100; // 10 milliseconds for strong network
    }
    return 100; // Default to normal if no condition is provided
}

int get_bandwidth_limit(const string& condition) {
    if (condition == "poor") {
        return BUFFER_SIZE / 5; // Limit bandwidth in poor conditions (e.g., 1/10th of buffer)
    } else if (condition == "normal") {
        return BUFFER_SIZE / 2; // 50% of the buffer for normal conditions
    } else if (condition == "strong") {
        return BUFFER_SIZE; // Full buffer capacity for strong network
    }
    return BUFFER_SIZE / 2; // Default to normal bandwidth
}
unordered_map <SOCKET,int> last_packet_map;
//int last_packet_id = -1;  // Initialized to -1 to indicate no packet received yet

// Function to handle each client (weather station)
void handle_client(SOCKET client_socket, const string& network_condition) {
    char buffer[BUFFER_SIZE] = {0};
    int delay = get_network_delay(network_condition);
    int bandwidth_limit = get_bandwidth_limit(network_condition);
    int ssthresh = 64;  // Slow start threshold
last_packet_map[client_socket]=-1;
    srand(time(0) + client_socket);  // Use current time + client_socket as a unique seed

    while (true) {
        // Receive the data from the client
        int bytes_read = recv(client_socket, buffer, bandwidth_limit, 0);
        if (bytes_read <= 0) {
            cerr << "Client disconnected or error occurred.\n";
            break;
        }

      // Extract packet ID, cwnd, ssthresh, and compressed data
        string data(buffer, bytes_read);
        size_t first_delim = data.find("|");
        size_t second_delim = data.find("|", first_delim + 1);
        size_t third_delim = data.find("|", second_delim + 1);

        int packet_id = stoi(data.substr(0, first_delim));  // Extract packet ID
        int cwnd = stoi(data.substr(first_delim + 1, second_delim - first_delim - 1));  // Extract cwnd
        int ssthresh2 = stoi(data.substr(second_delim + 1, third_delim - second_delim - 1));  // Extract ssthresh
        ssthresh=ssthresh2;
        string compressed_data = data.substr(third_delim + 1);  // Extract the compressed data

        // Simulate network delay
        this_thread::sleep_for(chrono::milliseconds(delay));

        // Simulate packet loss based on the client's cwnd value
        if (cwnd > ssthresh) {
            int loss_probability = rand() % 100;  // Generate a random number between 0 and 99
            //cout<<loss_probability<<endl;
            if (loss_probability < 20) {  // 20% chance to simulate packet loss
                cerr << "Simulating packet loss for packet ID: " << packet_id << " with cwnd: " << cwnd << "\n";
                // Simulate packet loss: send the last successfully received packet ID
              
                continue;  // Skip sending the ACK for the current packet
            }
        }

        // Check if the received packet is the expected packet (i.e., last_packet_id + 1)
        if (packet_id == last_packet_map[client_socket] + 1) {
            // Process the packet (decompress data)
            try {
                string decompressed_data = decompress_data(compressed_data);
               //  cout << "Received weather data: " << decompressed_data << endl;
            } catch (const exception& e) {
                cerr << "Error decompressing data: " << e.what() << endl;
            }

            // Update last_packet_id to the current packet ID
            last_packet_map[client_socket] = packet_id;

            // Send ACK back to the client with the current packet ID
            string ack_message = to_string(packet_id);
            send(client_socket, ack_message.c_str(), ack_message.size(), 0);
        } else {
            // Packet received is not the expected one, send the ACK for the last successfully received packet
        //    cerr << "Received out-of-order packet. Expected: " << last_packet_id + 1 << ", but got: " << packet_id << ". Sending last ACK for packet " << last_packet_id << "\n";
            string ack_message = to_string(last_packet_map[client_socket]);
            send(client_socket, ack_message.c_str(), ack_message.size(), 0);
        }
    }
    closesocket(client_socket);
}

// Main server function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <network_condition: poor|normal|strong>\n";
        return 1;
    }

    string network_condition = argv[1];
    cout << "Simulating network condition: " << network_condition << "\n";

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed with error: " << iResult << endl;
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        cerr << "Bind failed.\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen for connections
    if (listen(server_socket, MAX_CONNECTIONS) == SOCKET_ERROR) {
        cerr << "Listen failed.\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port " << PORT << "...\n";

    map<SOCKET, thread> client_threads;

    // Accept incoming client connections
    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Accept failed.\n";
            continue;
        }

        cout << "Client connected.\n";

        // Handle client in a separate thread
        lock_guard<mutex> lock(client_mutex);
        client_threads[client_socket] = thread(handle_client, client_socket, network_condition);
        client_threads[client_socket].detach();  // Allow threads to run independently
    }

    // Cleanup
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
