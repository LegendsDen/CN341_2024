#include <iostream>
#include <string>
#include <unordered_map>
#include <list>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

class LRUCache {
private:
    int capacity;
    list<string> order;
    unordered_map<string, pair<string, list<string>::iterator>> cache;

public:
    LRUCache(int cap) : capacity(cap) {}

    string get(const string& key) {
        if (cache.find(key) != cache.end()) {
            order.erase(cache[key].second);
            order.push_front(key);
            cache[key].second = order.begin();
            return cache[key].first;
        }
        return "";
    }

    void put(const string& key, const string& value) {
        if (cache.find(key) != cache.end()) {
            order.erase(cache[key].second);
        } else if (cache.size() >= capacity) {
            string old_key = order.back();
            order.pop_back();
            cache.erase(old_key);
        }
        order.push_front(key);
        cache[key] = {value, order.begin()};
    }

    void display_cache() {
        cout << "Current cache state (Most to Least recently used):" << endl;
        for (auto& k : order) {
            cout << k << ": " << cache[k].first.substr(0, 50) << "..." << endl;  // Show only the first 50 characters of content
        }
    }
};

// Dummy DNS lookup function
string dns_lookup(const string& domain) {
    // Simulate DNS resolution
    return "93.184.216.34";  // Example IP address for example.com
}

string http_get_request(const string& domain) {
    string ip_address = dns_lookup(domain);
    if (!ip_address.empty()) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            cerr << "Socket creation failed!" << endl;
            return "";
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(80);

        if (inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr) <= 0) {
            cerr << "Invalid address or address not supported!" << endl;
            return "";
        }

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "Connection failed!" << endl;
            return "";
        }

        string request = "GET / HTTP/1.1\r\nHost: " + domain + "\r\n\r\n";
        send(sockfd, request.c_str(), request.length(), 0);

        char buffer[4096];
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            cerr << "Failed to receive data!" << endl;
            return "";
        }

        buffer[bytes_received] = '\0';
        close(sockfd);
        return string(buffer);
    }
    return "";
}

string fetch_webpage(LRUCache& cache, const string& domain) {
    string cached_page = cache.get(domain);
    if (!cached_page.empty()) {
        cout << "Page for " << domain << " retrieved from cache." << endl;
        return cached_page;
    } else {
        cout << "Fetching page for " << domain << " from the web server..." << endl;
        string page_content = http_get_request(domain);
        if (!page_content.empty()) {
            cache.put(domain, page_content);
        }
        return page_content;
    }
}

int main() {
    LRUCache cache(5);  // Cache with capacity 5 pages

    string page = fetch_webpage(cache, "www.example.com");
    if (!page.empty()) {
        cout << page.substr(0, 100) << endl;  // Print first 100 characters of the page content
    }

    cache.display_cache();

return 0;
}