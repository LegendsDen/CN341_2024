#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <future>
#include <cstdlib>
#include <ctime>

using namespace std;

map<string, pair<string, string>> tldDNSCom = {
    {"example.com", {"Example.com", "BackupExample.com"}},
    {"google.com", {"Google.com", "BackupGoogle.com"}},
    {"amazon.com", {"Amazon.com", "BackupAmazon.com"}}
};

map<string, pair<string, string>> tldDNSOrg = {
    {"example.org", {"Example.org", "BackupExample.org"}},
    {"google.org", {"Google.org", "BackupGoogle.org"}},
    {"amazon.org", {"Amazon.org", "BackupAmazon.org"}}
};

map<string, pair<string, string>> tldDNSEdu = {
    {"example.edu", {"Example.edu", "BackupExample.edu"}},
    {"google.edu", {"Google.edu", "BackupGoogle.edu"}},
    {"amazon.edu", {"Amazon.edu", "BackupAmazon.edu"}}
};

map<string, map<string, pair<string, string>>> rootDNS = {
    {"com", tldDNSCom},
    {"org", tldDNSOrg},
    {"edu", tldDNSEdu}
};

map<string, string> authDNS = {
    {"Example.com", "102.0.2.1"},
    {"BackupExample.com", "102.0.2.11"},
    {"Example.org", "102.0.2.2"},
    {"BackupExample.org", "102.0.2.12"},
    {"Example.edu", "102.0.2.3"},
    {"BackupExample.edu", "102.0.2.13"},
    {"Google.com", "202.0.3.1"},
    {"BackupGoogle.com", "202.0.3.11"},
    {"Google.org", "202.0.3.2"},
    {"BackupGoogle.org", "202.0.3.12"},
    {"Google.edu", "202.0.3.3"},
    {"BackupGoogle.edu", "202.0.3.13"},
    {"Amazon.com", "302.0.4.1"},
    {"BackupAmazon.com", "302.0.4.11"},
    {"Amazon.org", "302.0.4.2"},
    {"BackupAmazon.org", "302.0.4.12"},
    {"Amazon.edu", "302.0.4.3"},
    {"BackupAmazon.edu", "302.0.4.13"}
};

string queryDNS(const string& serverType, const string& query) {
    this_thread::sleep_for(chrono::milliseconds(rand() % 400 + 100)); 
    if (rand() % 100 < 20) {
        cout << serverType << " failed!" << endl;
        return "";
    }
    return query;
}

string queryWithTimeout(const string& serverType, const string& query, int timeout) {
    auto futureQuery = async(launch::async, queryDNS, serverType, query);
    if (futureQuery.wait_for(chrono::milliseconds(timeout)) == future_status::timeout) {
        cout << serverType << " timed out!" << endl;
        return "";
    }
    return futureQuery.get();
}

string dnsLookup(const string& domain) {
    string ipAddress;
    
    // Extract TLD from domain
    string tld = domain.substr(domain.find_last_of('.') + 1);

    // Step 1: Query Root DNS Server for TLD DNS Server
    auto tldServerMapIt = rootDNS.find(tld);
    if (tldServerMapIt == rootDNS.end()) {
        return "Failed to find TLD in Root DNS.";
    }

    auto tldServerPair = tldServerMapIt->second;
    string tldServer = queryWithTimeout("Root DNS", tld, 100); 
    if (tldServer.empty()) {
        tldServer = queryWithTimeout("Root DNS", tld, 500); // Retry with timeout
        if (tldServer.empty()) {
            return "Failed to resolve domain at Root DNS level.";
        }
    }

    // Step 2: Query TLD DNS Server
    string domainWithoutTld = domain.substr(domain.find('.') + 1);
    auto authServerPairIt = tldServerPair.find(domainWithoutTld);
    if (authServerPairIt == tldServerPair.end()) {
        return "Failed to find domain in TLD DNS.";
    }

    string authServer = queryWithTimeout("TLD DNS", authServerPairIt->second.first, 100); 
    if (authServer.empty()) {
        cout << "Switching to backup TLD DNS Server..." << endl;
        authServer = queryWithTimeout("TLD DNS", authServerPairIt->second.second, 500); // Switch to backup server
        if (authServer.empty()) {
            return "Failed to resolve domain at TLD DNS level.";
        }
    }

    // Step 3: Query Authoritative DNS Server
    auto ipAddressIt = authDNS.find(authServer);
    if (ipAddressIt == authDNS.end()) {
        return "Failed to find IP address in Authoritative DNS.";
    }

    ipAddress = queryWithTimeout("Authoritative DNS", ipAddressIt->second, 100); 
    if (ipAddress.empty()) {
        cout << "Retrying Authoritative DNS Server..." << endl;
        ipAddress = queryWithTimeout("Authoritative DNS", ipAddressIt->second, 500); // Retry with timeout
        if (ipAddress.empty()) {
            return "Failed to resolve domain at Authoritative DNS level.";
        }
    }

    return ipAddress;
}

int main() {
    srand(time(0));  
    
    string domain;
    cin >> domain;
    cout << "Querying for domain: " << domain << endl;

    string ip = dnsLookup(domain);
    
    if (!ip.empty()) {
        cout << "IP Address for " << domain << ": " << ip << endl;
    } else {
        cout << "Failed to resolve " << domain << endl;
    }

    return 0;
}
