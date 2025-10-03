/*
Copyright (c) 2025 Dave Quinn <qcent@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#pragma once

#ifndef UDP_CONNECTION_H
#define UDP_CONNECTION_H

#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "identme.h" // query your IP address using http://ident.me

constexpr size_t DEFAULT_UDP_BUFFER_SIZE = 128;

class UDPConnection {
private:
    char hostAddress[16];
    char listenAddress[16];
    int port;
    int socklen;
    socklen_t addrlen;
    SOCKET udpSocket;
    struct sockaddr_in servaddr, cliaddr;
    struct sockaddr_in* other; // simplifies 1 server/1 client communication
    bool silent = false;
    bool server = false;
    char* defaultBuffer;
    size_t defaultBufferSize;
    bool alloc_buff = false;
    
    // Header packing and unpacking methods...
    std::vector<char> pack_header(const std::unordered_map<std::string, std::string>& header);
    std::unordered_map<std::string, std::string> unpack_header(const char* headerData, int dataSize);
    void setSocketTimeout(int socket, int timeoutMillisec);

    // Establishes a buffer for UDP communication
    void allocate_default_buffer();

public:
    UDPConnection();
    UDPConnection(const std::string& hostAddress = "127.0.0.1", int port = 5000, const std::string& listenAddress = "0.0.0.0");
    UDPConnection(int port = 5000, const std::string& listenAddress = "0.0.0.0");
    ~UDPConnection();

    // Handy Helpers
    std::string get_local_ip();
    std::string get_external_ip();
    void set_silence(bool setting);
    int set_server_blocking(bool block = true);
    int set_client_blocking(bool block = true);
    void set_server_timeout(int timeoutMillisec);
    void set_client_timeout(int timeoutMillisec);

    // Server-related methods...
    int start_as_server(int port);
    std::pair<SOCKET, sockaddr_in> await_connection();

    // Client-related methods...
    void start_as_client(const std::string& hostAddress, int port);
    int establish_connection(const std::string& servAddress, int PORT);   // need to provide host and port
    
    // Data communication methods..
    int send_data(const char* data, int size);
    int receive_data(char* buffer, int bufferSize);

    // new methods
    int get_available_data_size();  // dummy function, not relevant for UDP connections
    int receive_null_data(int count); // receives data over connection and does nothing with it, freeing up the socket buffer (TCP)
    bool is_server() { return server; }

};

UDPConnection::~UDPConnection() {
    if (udpSocket != INVALID_SOCKET) {
        closesocket(udpSocket);
    }
    WSACleanup();

    if (alloc_buff) {
        delete[] defaultBuffer;
    }
}

UDPConnection::UDPConnection(const std::string& hostAddress, int port, const std::string& listenAddress)
    : port(port), udpSocket(INVALID_SOCKET) {
    strcpy_s(this->hostAddress, sizeof(this->hostAddress), hostAddress.c_str());
    strcpy_s(this->listenAddress, sizeof(this->listenAddress), listenAddress.c_str());

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (!silent) std::cerr << "Failed to initialize Winsock" << std::endl;
    }

    addrlen = sizeof(servaddr);
    memset(&servaddr, 0, addrlen);
    memset(&cliaddr, 0, addrlen);

    allocate_default_buffer();
}

UDPConnection::UDPConnection(int port, const std::string& listenAddress)
    : port(port), udpSocket(INVALID_SOCKET), hostAddress("") {
    strcpy_s(this->listenAddress, sizeof(this->listenAddress), listenAddress.c_str());

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (!silent) std::cerr << "Failed to initialize Winsock" << std::endl;
    }

    addrlen = sizeof(servaddr);
    memset(&servaddr, 0, addrlen);
    memset(&cliaddr, 0, addrlen);

    allocate_default_buffer();
}

UDPConnection::UDPConnection() {

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
    }

    addrlen = sizeof(servaddr);
    memset(&servaddr, 0, addrlen);
    memset(&cliaddr, 0, addrlen);

    allocate_default_buffer();
}


int UDPConnection::start_as_server(int PORT) {
    port = PORT;
    
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return -1;
    }

    // Filling server information 
    servaddr.sin_family = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY;  // listen address
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address 
    if (bind(udpSocket, (sockaddr*)&servaddr, addrlen) == SOCKET_ERROR)
    {
        std::cerr << "Socket Bind failed" << std::endl;
        return -1;
    }

    other = &cliaddr;
    server = true;
    return 1;
}

void UDPConnection::start_as_client(const std::string& servAddress, int PORT) {
// Use establish_connection() for TCP cross compatibility
    strcpy_s(this->hostAddress, sizeof(this->hostAddress), servAddress.c_str());
    port = PORT;

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // <<< UDP socket
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Filling server information 
    servaddr.sin_family = AF_INET; // IPv4 
        // Get server ip as bit values
    inet_pton(AF_INET, hostAddress, &(servaddr.sin_addr)); // server ip
    servaddr.sin_port = htons(port);

    other = &servaddr;
}

void UDPConnection::set_silence(bool setting = true) {
    silent = setting;
}

int UDPConnection::set_server_blocking(bool block) {
    u_long mode = !block;
    return ioctlsocket(udpSocket, FIONBIO, &mode);
}
int UDPConnection::set_client_blocking(bool block) {
    u_long mode = !block;
    return ioctlsocket(udpSocket, FIONBIO, &mode);
}


int UDPConnection::send_data(const char* data, int size) {

    int bytesSent = sendto(udpSocket, data, size, 0, (sockaddr*)other, addrlen);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Failed to send data (SOCKET_ERROR)" << WSAGetLastError() << std::endl;
    }
    return bytesSent;
}

int UDPConnection::receive_data(char* buffer, int bufferSize) {
    int bytesReceived = recvfrom(udpSocket, buffer, bufferSize, 0, (sockaddr*)other, &addrlen);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "Failed to receive data (SOCKET_ERROR)" << WSAGetLastError() << std::endl;
    }
    return bytesReceived;
}


void UDPConnection::allocate_default_buffer() {
    alloc_buff = true;
    defaultBuffer = new char[DEFAULT_UDP_BUFFER_SIZE];
    defaultBufferSize = DEFAULT_UDP_BUFFER_SIZE;
}

/* TCP CROSS COMPATIBILITY METHODS */
std::pair<SOCKET, sockaddr_in> UDPConnection::await_connection() {
    sockaddr_in clientAddress{};

    int bytesReceived = recvfrom(udpSocket, defaultBuffer, defaultBufferSize, 0, (sockaddr*)other, &addrlen);
    if (bytesReceived == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)
            std::cerr << "Failed to await data (SOCKET_ERROR)" << err << std::endl;
        else
            return { INVALID_SOCKET, {} };
    }
    clientAddress = *other;
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

    if (udpSocket == INVALID_SOCKET) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)  if (!silent) std::cerr << "Failed to accept client connection : " << err << std::endl;
        return { INVALID_SOCKET, {} };
    }
    if (!silent)
        std::cout << "Connection from " << clientIP << ":" << ntohs(clientAddress.sin_port) << " established" << std::endl;

    return { udpSocket, clientAddress };
}

int UDPConnection::establish_connection(const std::string& servAddress, int PORT) {
    start_as_client(servAddress, PORT);
    send_data((const char*)defaultBuffer, 1);  // send 1 byte to initiate 'connection'

    return 1;
}

int UDPConnection::get_available_data_size() {
    return 0;
}

int UDPConnection::receive_null_data(int count) {
    char* tempBuffer = new char[count];
    int bytesReceived = recvfrom(udpSocket, tempBuffer, count, 0, (sockaddr*)other, &addrlen);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
        delete[] tempBuffer;
        return -1; // Error occurred
    }
    delete[] tempBuffer;
    return bytesReceived;
}


void UDPConnection::setSocketTimeout(int socket, int timeoutMillisec) {
    // Set receive timeout
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMillisec, sizeof(timeoutMillisec)) < 0) {
        if (!silent)
            std::cerr << "Error setting receive timeout" << std::endl;
    }

    // Set send timeout
    if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMillisec, sizeof(timeoutMillisec)) < 0) {
        if (!silent)
            std::cerr << "Error setting send timeout" << std::endl;
    }
}

void UDPConnection::set_server_timeout(int timeoutMillisec) {
    setSocketTimeout(udpSocket, timeoutMillisec);
}
void UDPConnection::set_client_timeout(int timeoutMillisec) {
    setSocketTimeout(udpSocket, timeoutMillisec);
}


std::string UDPConnection::get_local_ip() {
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
        if (!silent)
            std::cerr << "Failed to get local IP address" << std::endl;
        return "";
    }

    struct addrinfo hints {}, * result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostName, nullptr, &hints, &result) != 0) {
        if (!silent)
            std::cerr << "Failed to resolve local IP address" << std::endl;
        return "";
    }

    char ipAddress[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr, ipAddress, sizeof(ipAddress)) == nullptr) {
        if (!silent)
            std::cerr << "Failed to convert local IP address to string" << std::endl;
        return "";
    }

    freeaddrinfo(result);

    return ipAddress;
}

std::string UDPConnection::get_external_ip() {
    char addr[16];
    identme(addr, sizeof(addr));
    return addr;
}

#endif // UDP_CONNECTION_H
