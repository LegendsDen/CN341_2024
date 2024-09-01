#define _WIN32_WINNT 0x501

#include<unordered_map>
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <thread>
#include <mutex>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// Global map to store cookies for each client session
unordered_map<string, map<string, string>> session_cookies;
mutex session_mutex;
unordered_map<string, int> domain_access_count; // Global map to count access frequency

void log_request(const string& request, string &client_socket) {
        cout<<endl;

    cout << "Request from client [" << client_socket << "]:\n" << request << endl;
        cout<<endl;
          // Extract Host header
    size_t host_pos = request.find("Host:");
    string host = "default_host"; // Default value
    if (host_pos != string::npos) {
        size_t host_end_pos = request.find("\r\n", host_pos);
        if (host_end_pos == string::npos) {
            host_end_pos = request.length();
        }
        host = request.substr(host_pos + 5, host_end_pos - host_pos - 5);
    }

    lock_guard<mutex> lock(session_mutex);
    domain_access_count[host]++;
    cout << "Access frequency for [" << host << "]: " << domain_access_count[host] << endl;

}

void log_response(const string& response, string &client_socket) {
        cout<<endl;

    cout << "Response to client [" << client_socket << "]:\n" << response << endl;
        cout<<endl;

}



void parse_cookies(const string& response, string client_socket) {
    size_t pos = 0;
    while ((pos = response.find("Set-Cookie:", pos)) != string::npos) {
        size_t end_pos = response.find("\r\n", pos);
        if (end_pos == string::npos) {
            end_pos = response.length();
        }
        string cookie_line = response.substr(pos + 12, end_pos - pos - 12);  // 12 is the length of "Set-Cookie:"
        
        size_t delimiter = cookie_line.find(';');
        string cookie = cookie_line.substr(0, delimiter);
        
        size_t equals_pos = cookie.find('=');
        if (equals_pos != string::npos) {
            string cookie_name = cookie.substr(0, equals_pos);
            string cookie_value = cookie.substr(equals_pos + 1);
            
            {
                // Lock the mutex to ensure thread safety when modifying the session_cookies map
                lock_guard<mutex> lock(session_mutex);
                
                // Check if this is a new cookie or updating an existing one
                if (session_cookies[client_socket].find(cookie_name) == session_cookies[client_socket].end()) {
                    cout << "Storing the cookie for session [" << client_socket << "]: " << cookie << " cookie value: "<< cookie_value<< " cookie name: "<< cookie_name<< endl;
                } else {
                    cout << "Updating the cookie for session [" << client_socket << "]: " << cookie << " cookie value: "<< cookie_value<< " cookie name: "<< cookie_name<< endl;
                }
                
                // Store or update the cookie in the session-specific map
                session_cookies[client_socket][cookie_name] = cookie_value;
            }
        }

        pos = end_pos;
    }
}




string build_cookie_header(string client_socket) {
    lock_guard<mutex> lock(session_mutex);
    string cookie_header;
    
    if (session_cookies.find(client_socket) != session_cookies.end() && !session_cookies[client_socket].empty()) {
        cookie_header = "Cookie: ";
        
        for (const auto& pair : session_cookies[client_socket]) {
            cookie_header += pair.first + "=" + pair.second + "; ";
        }
        
        // Remove the trailing semicolon and space
        if (!cookie_header.empty()) {
            cookie_header.pop_back();
            cookie_header.pop_back();
        }
        
        cout << "Using the cookie stored on proxy server for session [" << client_socket << "]." << endl;
    }
    
    return cookie_header;
}


void handle_client(SOCKET client_socket)
{
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    cout << "Entered handle_client" << endl;

    // Read data from the client
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0)
    {
        cerr << "Failed to read data" << endl;
        closesocket(client_socket);
        return;
    }
    buffer[bytes_read] = '\0';  // Null-terminate the string
    string request(buffer);

    // Extract Client-ID header (custom header for identifying the client)
    size_t client_id_pos = request.find("Client-ID:");
    string client_id = "default_client";
    if (client_id_pos != string::npos) {
        size_t client_id_end_pos = request.find("\r\n", client_id_pos);
        if (client_id_end_pos == string::npos) {
            client_id_end_pos = request.length();
        }
        client_id = request.substr(client_id_pos + 10, client_id_end_pos - client_id_pos - 10);  // 10 is the length of "Client-ID:"
        // Trim any leading/trailing whitespace
        size_t start = client_id.find_first_not_of(" \t");
        size_t end = client_id.find_last_not_of(" \t");
        if (start != string::npos && end != string::npos) {
            client_id = client_id.substr(start, end - start + 1);
        }
    }
    log_request(request, client_id);

    // Extract the Host header
    size_t host_pos = request.find("Host:");
    string host = "www.google.com"; // Default to google.com
    if (host_pos != string::npos)
    {
        size_t host_end_pos = request.find("\r\n", host_pos);
        if (host_end_pos == string::npos) {
            host_end_pos = request.length();
        }
        string host_line = request.substr(host_pos + 5, host_end_pos - host_pos - 5);  // 5 is the length of "Host:"
        // Trim any leading/trailing whitespace
        size_t start = host_line.find_first_not_of(" \t");
        size_t end = host_line.find_last_not_of(" \t");
        if (start != string::npos && end != string::npos) {
            host = host_line.substr(start, end - start + 1);
        }
    }

    // Prepare to forward the request with cookies if available
    string cookie_header = build_cookie_header(client_id);
    if (!cookie_header.empty()) {
        size_t header_end_pos = request.find("\r\n\r\n");
        if (header_end_pos != string::npos) {
            request.insert(header_end_pos, "\r\n" + cookie_header);
            // cout<<"storing the cookie"<<"\n";
        }
        // else{
        //     cout<<"the cookie exists so we are using that"<<"\n";
        // }
    }

    // Resolve the extracted Host to its IP address
    addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *result = nullptr;

    int res = getaddrinfo(host.c_str(), "80", &hints, &result);
    if (res != 0)
    {
        cerr << "Failed to resolve hostname: " << host << ". Error: " << gai_strerror(res) << endl;
        closesocket(client_socket);
        return;
    }

    sockaddr_in *server_addr = reinterpret_cast<sockaddr_in *>(result->ai_addr);

    // Create socket to connect to the target server
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(client_socket);
        return;
    }

    // Connect to target server
    if (connect(server_socket, reinterpret_cast<sockaddr *>(server_addr), sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        cerr << "Connection to target server failed. Error Code: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        freeaddrinfo(result);
        closesocket(client_socket);
        return;
    }

    // Forward request to target server
    send(server_socket, request.c_str(), request.size(), 0);

    // Receive response from target server
    memset(buffer, 0, sizeof(buffer));
    int bytes_received;
    if ((bytes_received = recv(server_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        string response(buffer, bytes_received);
        parse_cookies(response, client_id);
        log_response(response, client_id);
        send(client_socket, buffer, bytes_received, 0);
    }

    // Clean up
    closesocket(server_socket);
    closesocket(client_socket);
    freeaddrinfo(result);
}

int main() {
    ios::sync_with_stdio(false);  // Disable output buffering

    WSADATA wsaData;
    SOCKET server_socket;
    struct sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock. Error Code : " << WSAGetLastError() << endl;
        return EXIT_FAILURE;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error Code : " << WSAGetLastError() << endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Listen for connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        cerr << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    cout << "Listening on port 8080..." << endl;

    while (true) {
        // Accept a connection
        cout << "Waiting for client connection..." << endl;
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Accept failed with error: " << WSAGetLastError() << endl;
            closesocket(server_socket);
            WSACleanup();
            return EXIT_FAILURE;
        }

        cout << "Connection accepted." << endl;

        // Handle the client request in a new thread
        thread client_thread(handle_client, client_socket);
        client_thread.detach();  // Detach the thread to handle the client independently
    }

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
