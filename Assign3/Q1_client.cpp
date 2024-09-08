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
#include <chrono>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;
mutex movement_mutex;

#define BUFFER_SIZE 1024
struct Movement{
    int x;
    int y;
    int speed;
    bool On;
    string direction;
    Movement():x(0),y(0),speed(0),On(false),direction("None"){};
};

// Movement thread function
void movement_thread(Movement &movement) {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1000)); // Move every 1000 ms

        lock_guard<mutex> lock(movement_mutex);
        if (movement.On) {
            if (movement.direction == "right") {
                movement.x += movement.speed;
            } else if (movement.direction == "left") {
                movement.x -= movement.speed;
            } else if (movement.direction == "up") {
                movement.y += movement.speed;
            } else if (movement.direction == "down") {
                movement.y -= movement.speed;
            }
            cout << "Moving " << movement.direction << " to (" << movement.x << ", " << movement.y << ") at speed " << movement.speed << endl;
        }
    }
}

// Function to process received commands
void process_command(const string& command, Movement& movement) {
    lock_guard<mutex> lock(movement_mutex);
  
    if (command.rfind("Start", 0) == 0) { 
        // Start command with speed
        int speed = stoi(command.substr(6));  // Get speed from command
        movement.speed = speed;
        movement.On = true;
        movement.direction = "right";  // Default direction is right
        cout << "Started moving with speed " << movement.speed << endl;

    } else if (command == "move_left") {
        movement.direction = "left";
        cout << "Changed direction to left" << endl;

    } else if (command == "move_right") {
        movement.direction = "right";
        cout << "Changed direction to right" << endl;

    } else if (command == "move_up") {
        movement.direction = "up";
        cout << "Changed direction to up" << endl;

    } else if (command == "move_down") {
        movement.direction = "down";
        cout << "Changed direction to down" << endl;

    } else if (command.rfind("inc", 0) == 0) {  // Increment speed
        int increment = stoi(command.substr(4));  // Get increment value
        movement.speed += increment;
        cout << "Increased speed by " << increment << ", new speed: " << movement.speed << endl;

    } else if (command.rfind("dec", 0) == 0) {  // Decrease speed
        int decrement = stoi(command.substr(4));  // Get decrement value
        movement.speed = max(0, movement.speed - decrement);  // Ensure speed doesn't go below 0
        if(movement.speed==0){
            movement.On=false;
            cout<<"Stopped moving"<<endl;
        }
        else
        cout << "Decreased speed by " << decrement << ", new speed: " << movement.speed << endl;

    } else if (command == "Stop") {  // Stop the movement
        movement.speed = 0;
        movement.On = false;
        cout << "Movement stopped" << endl;
    }
}



char xor_key = 'K'; // Example XOR key
vector<char> xor_cipher(const vector<char>& data, char key) {
    vector<char> result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ key;
    }
    return result;
}


void send_udp_data(const string& server_ip, int port,const string& client_id, Movement &movement) {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);



    // Send the client ID  to the server
    int send_result = sendto(sockfd, client_id.c_str(), client_id.size(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (send_result == SOCKET_ERROR) {
        cerr << "Send failed: " << WSAGetLastError() << endl;
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    cout << "Sent client ID: " << client_id << endl;
    sleep(1);
    // Receive command from server
    char buffer[1024] = {0};
    int server_addr_len = sizeof(server_addr);
    while(true){
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &server_addr_len);
        if (recv_len == SOCKET_ERROR) {
            cerr << "Receive failed: " << WSAGetLastError() << endl;
        } else {
            buffer[recv_len] = '\0';  // Null-terminate the received data
            cout << "Received command: " << buffer << endl;
            string command(buffer);
            process_command(command, movement);
        }
    }

    // Close the socket
    closesocket(sockfd);
    WSACleanup();
}




string serialize_movement(const Movement& movement) {
    stringstream ss;
    ss << movement.x << " " << movement.y << " " << movement.speed << " " << movement.On << " " << movement.direction;
    return ss.str();
}
void send_telemetry_data(const string& server_ip, int port,const string& client_id,Movement &movement) {
    int sockfd;
    struct sockaddr_in server_addr;
    WSADATA wsaData;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << strerror(errno) << endl;
        WSACleanup();
        return;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    // Attempt to connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Connection failed: " << strerror(errno) << endl;
        close(sockfd);
        WSACleanup();
        return;
    }

    cout << "Connection made" << endl;
     // Send the client identifier
    ssize_t id_bytes_sent = send(sockfd, client_id.c_str(), client_id.size(), 0);
    if (id_bytes_sent < 0) {
        cerr << "Client ID send failed: " << strerror(errno) << endl;
        close(sockfd);
        WSACleanup();
        return;
    }
    sleep(1);
    
    while(true){
    // Encrypt the message
    string serialized_movement = serialize_movement(movement);
    auto encrypted_message = xor_cipher(vector<char>(serialized_movement.begin(), serialized_movement.end()), xor_key);
    // Send encrypted message
    ssize_t bytes_sent = send(sockfd, encrypted_message.data(), encrypted_message.size(),0);
    if (bytes_sent < 0) {
        cerr << "Write failed: " << strerror(errno) << endl;
    } else {
        cout << "Sent " << bytes_sent << " bytes." << endl;
        cout<<movement.direction<<endl;
    }

    // Optional sleep to ensure data is fully sent before closing
    sleep(100);
    }

    // Close the socket
    close(sockfd);

    // Clean up Winsock
    WSACleanup();
}
void send_quic_data(const string& server_ip, int port,const string& file_path,const string& client_id){
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Connection failed: " << WSAGetLastError() << endl;
        closesocket(sockfd);
        WSACleanup();
        return;
    }
    ssize_t id_bytes_sent = send(sockfd, client_id.c_str(), client_id.size(), 0);
    if (id_bytes_sent < 0) {
        cerr << "Client ID send failed: " << strerror(errno) << endl;
        close(sockfd);
        WSACleanup();
        return;
    }
    sleep(1);
    string check;
    while(true){
        // Open file
        cout << "Enter file_send  to send file via QUIC :";
        getline(cin,check);
        if(check=="file_send"){
            ifstream file(file_path, ios::binary);
            if (!file) {
                cerr << "Failed to open file: " << file_path << endl;
                closesocket(sockfd);
                WSACleanup();
                return;
            }
            // Send file in chunks
            char buffer[BUFFER_SIZE];
            while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
                int bytes_to_send = file.gcount();
                int bytes_sent = send(sockfd, buffer, bytes_to_send, 0);
                if (bytes_sent == SOCKET_ERROR) {
                    cerr << "File send failed: " << WSAGetLastError() << endl;
                    break;
                }
            }
            cout << "File sent successfully: " << file_path << endl;
            // Close file and socket
            file.close();
        }
        else{
            cout<<"Command not recoginzed"<<endl;
        }
    }
    closesocket(sockfd);
    WSACleanup();
}



int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <client_id>" << endl;
        return 1;
    }
    string server_ip = "192.168.0.101"; // Change to your server IP
    int control_port = 8080;
    int telemetry_port = 8081;
    int file_port = 8082;
    string client_id = argv[1];
    string file_path = "C:\\C++ course\\codingclub.cpp";  // Example file to send
    

    Movement movement;
    thread move_thread(movement_thread, ref(movement));
    thread udp (send_udp_data,server_ip,control_port,client_id, ref(movement));
    thread tele (send_telemetry_data,server_ip, telemetry_port, client_id, ref(movement));
    thread quic (send_quic_data,server_ip,file_port,file_path,client_id);
    udp.join();
    tele.join();
    quic.join();
    move_thread.join();
   
    
    
    return 0;
}