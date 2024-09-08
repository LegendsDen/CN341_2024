#include <thread>
#include <vector>
#include <string>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <list>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <bits/stdc++.h>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;    
#define BUFFER_SIZE 4096
mutex client_mutex; 
struct Movement{
    int x;
    int y;
    int speed;
    bool On;
    string direction;
    Movement():x(0),y(0),speed(0),On(false),direction("None"){};
};

char xor_key = 'K'; // Example XOR key
map<string, struct sockaddr_in> mapping;

vector<char> xor_cipher(const vector<char>& data, char key) {
    vector<char> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key;
    }
    return result;
}
Movement deserialize_movement(const string& data) {
    Movement movement;
    stringstream ss(data);

    ss >> movement.x >> movement.y >> movement.speed >> movement.On >> movement.direction;

    return movement;
}
void handle_client_TCP(SOCKET newsockfd) {
    char buffer[BUFFER_SIZE]={0};
    // Receive the client identifier
    int id_bytes_read = recv(newsockfd, buffer, BUFFER_SIZE, 0);
    if (id_bytes_read < 0) {
        cerr << "Read failed with error: " << strerror(errno) << endl;
        return;
    }
    string client_id(buffer, buffer + id_bytes_read);
    cout << "Connected Client ID: " << client_id << endl;
    // Continue receiving telemetry data
   // In the while(true) loop on the server side
    while (true) {
        int bytes_read = recv(newsockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_read < 0) {
            cout << "Client " << client_id << " Disconnected" << endl;
            break;
        }

        vector<char> data(buffer, buffer + bytes_read);

        // Decrypt the received data
        auto decrypted_data = xor_cipher(data, xor_key);

        // Convert the decrypted data back to a string
        string decrypted_str(decrypted_data.begin(), decrypted_data.end());

        // Deserialize the string to a Movement object
        Movement received_movement = deserialize_movement(decrypted_str);

        // Print the received movement data
        cout << "Data from " << client_id << ": " << endl;
        cout << "Position: (" << received_movement.x << ", " << received_movement.y << ")" << endl;
        cout << "Speed: " << received_movement.speed <<" unit/sec" <<endl;
        cout << "On: " << (received_movement.On ? "True" : "False") << endl;
        cout << "Direction: " << received_movement.direction << endl;
    }
    closesocket(newsockfd);
}


void telemetry_data_handler(int port) {
    try {
        int sockfd;
        struct sockaddr_in server_addr, client_addr;
         

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed!" << endl;
            return;
        }

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == INVALID_SOCKET) {
            cerr << "Failed to create TCP socket for telemetry" << endl;
            return;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "Bind failed with error: " << strerror(errno) << endl;
            close(sockfd);
            return;
        }

        if (listen(sockfd, 5) < 0) {
            cerr << "Listen failed with error: " << strerror(errno) << endl;
            close(sockfd);
            return;
        }

        cout << "TCP Server listening on port " << port << endl;
        map<SOCKET, thread> client_threads_TCP;
        socklen_t len = sizeof(client_addr);
        while (true) {
            int newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &len);
            if (newsockfd == INVALID_SOCKET) {
                cerr << "Accept failed.\n";
                continue;
            }

            cout << "Client connected: socket fd = " << newsockfd << endl;

            // Handle client in a separate thread
            lock_guard<mutex> lock(client_mutex);
            client_threads_TCP[newsockfd] = thread(handle_client_TCP, newsockfd);
            client_threads_TCP[newsockfd].detach();  // Allow threads to run independently
        }
        close(sockfd);
    } catch (const exception& e) {
        cerr << "Telemetry Data Handler Error: " << e.what() << endl;
    }
}
void receive_thread(SOCKET sockfd) {
    char buffer[1024];
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    while (true) {
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            cerr << "Receive failed: " << WSAGetLastError() << endl;
        } else {
            buffer[recv_len] = '\0';  // Null-terminate the received data
            cout << "Received client ID: " << buffer << endl;
            lock_guard<mutex> lock(client_mutex);
            string str(buffer);
            mapping[str] = client_addr;
        }
    }
}


void send_thread(SOCKET sockfd) {
    string command;
    string to_whom;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    while (true) {
        cout << "Enter which client to send: ";
        getline(cin, to_whom);  // Get input from user

        lock_guard<mutex> lock(client_mutex);
        auto it = mapping.find(to_whom);
        if (it != mapping.end()) {
            client_addr = it->second;
            cout << "Enter command: ";
            getline(cin, command);

            // Send the user command to the client
            int send_result = sendto(sockfd, command.c_str(), command.size(), 0, (struct sockaddr*)&client_addr, client_addr_len);
            if (send_result == SOCKET_ERROR) {
                cerr << "Send failed: " << WSAGetLastError() << endl;
            } else {
                cout << "Sent command to client "<< to_whom<<" : " << command << endl;
            }
        } else {
            cerr << "Client not found." << endl;
        }
    }
}


void udp_data_handler(int port) {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr, client_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return;
    }

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    cout << "UDP Server listening on port " << port << endl;



    // Start receive and send threads
    thread recv_thread(receive_thread, sockfd);
    thread snd_thread(send_thread, sockfd);

    // Wait for threads to finish (they won't in this example as they run indefinitely)
    recv_thread.join();
    snd_thread.join();
    // Close the socket
    closesocket(sockfd);
    WSACleanup();
}
void handle_file_transfer_quic(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    
    ofstream output_file("3AsignCN", ios::binary);  // File to save the incoming data

    if (!output_file) {
        cerr << "Failed to open file for writing." << endl;
        return;
    }




    int bytes_received;
     bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        cerr << "Read failed with error: " << strerror(errno) << endl;
        return;
    }

    string client_id_quic(buffer, buffer + bytes_received);
    while(true){
        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            output_file.write(buffer, bytes_received);
            cout<<"Chunk recieved from client  "<<client_id_quic<<" : "<<bytes_received <<endl;;
        }
        if (bytes_received == SOCKET_ERROR) {
            cerr << "File receive failed: " << WSAGetLastError() << endl;
        } else {
            cout << "File received successfully from " << client_id_quic<< endl;
        }
        output_file.close();
    }
    closesocket(client_socket);
}
void quic_file_handler(int port){
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return;
    }

    // Create TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    cout << "File transfer server listening on port " << port << endl;

    // Accept incoming file transfer requests
    map<SOCKET,thread>quic_threads_TCP;
    socklen_t client_len = sizeof(client_addr);
    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Accept failed: " << WSAGetLastError() << endl;
            continue;
        }

        cout << "Client connected for file transfer." << endl;
        lock_guard<mutex>lock(client_mutex);
        quic_threads_TCP[client_socket]=thread(handle_file_transfer_quic, client_socket);
        quic_threads_TCP[client_socket].detach();

    }

    closesocket(server_socket);
    WSACleanup();


}


int main() {
    try {
        int control_port = 8080;
        int telemetry_port = 8081;
        int file_port = 8082;

        thread control_thread(udp_data_handler, control_port);
        thread telemetry_thread(telemetry_data_handler, telemetry_port);
        thread quic_thread(quic_file_handler,file_port);

        control_thread.join();
        telemetry_thread.join();
        quic_thread.join();
        
    } catch (const exception& e) {
        cerr << "Main Error: " << e.what() << endl;
    }

    return 0;
}