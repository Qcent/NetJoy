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

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstring>
#include <string>
#include <vector>       
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "identme.h" // query your IP address using http://ident.me

#pragma comment(lib, "ws2_32.lib")

#define NETWORK_TIMEOUT_MILLISECONDS 800

class TCPConnection {
private:
    char hostAddress[16];
    char listenAddress[16];
    int port;    
    SOCKET clientSocket;
    SOCKET serverSocket;
    bool silent = false;
    bool server = false;
    
    // Header packing and unpacking methods...
    std::vector<char> pack_header(const std::unordered_map<std::string, std::string>& header);
    std::unordered_map<std::string, std::string> unpack_header(const char* headerData, int dataSize);
    void setSocketTimeout(int socket, int timeoutMillisec);
public:
    TCPConnection(const std::string& hostAddress = "127.0.0.1", int port = 5000, const std::string& listenAddress = "0.0.0.0");
    TCPConnection(int port = 5000, const std::string& listenAddress = "0.0.0.0");
    ~TCPConnection();

    // Handy Helpers
    std::string get_local_ip();
    std::string get_external_ip();
    void set_silence(bool setting);
    int set_server_blocking(bool block = true);
    int set_client_blocking(bool block = true);
    void set_server_timeout(int timeoutMillisec);
    void set_client_timeout(int timeoutMillisec);

    // Server-related methods...
    int start_as_server(int dummyPORT = 0);
    std::pair<SOCKET, sockaddr_in> await_connection();

    // Client-related methods...
    int establish_connection(const std::string& dummyAddress="", int dummyPORT = 0); // dummy variables for UDP compatibility

    // Data communication methods...
    int send_data(const char* data, int size);
    int receive_data(char* buffer, int bufferSize);

    // Data communication methods with header...
    int send_data(const char* data, int size, const std::unordered_map<std::string, std::string>& header);
    int receive_data(char* buffer, int bufferSize, std::unordered_map<std::string, std::string>& header);

    // new methods...
    int get_available_data_size();  // returns the size of data available in the socket buffer
    int receive_null_data(int count);  // receives data over connection and does nothing with it, freeing up the socket buffer (TCP)
    bool is_server() { return server; }

    // Disables Nagle's algorithm in the TCP stack.
    // Nagle's algorithm increases network efficiency by combining would-be packets
    // to reach the MSS, delaying transmission up to 500ms
    void disable_nagle();

};

TCPConnection::~TCPConnection() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
    }
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }
    WSACleanup();
}

TCPConnection::TCPConnection(const std::string& hostAddress, int port, const std::string& listenAddress)
    : port(port), clientSocket(INVALID_SOCKET), serverSocket(INVALID_SOCKET) {
    strcpy_s(this->hostAddress, sizeof(this->hostAddress), hostAddress.c_str());
    strcpy_s(this->listenAddress, sizeof(this->listenAddress), listenAddress.c_str());

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (!silent) std::cerr << "Failed to initialize Winsock" << std::endl;
    }
}

TCPConnection::TCPConnection(int port, const std::string& listenAddress)
    : port(port), clientSocket(INVALID_SOCKET), serverSocket(INVALID_SOCKET), hostAddress("") {
    strcpy_s(this->listenAddress, sizeof(this->listenAddress), listenAddress.c_str());

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (!silent) std::cerr << "Failed to initialize Winsock" << std::endl;
    }
}

int TCPConnection::start_as_server(int dummyPORT) {
    // for compatibility with UPD class
    if (dummyPORT) {    // if dummy PORT is provided take as actual variable
        port = dummyPORT;        
    }

#if DEVTEST
    strcpy_s(this->listenAddress, sizeof(this->listenAddress), "127.0.0.1"); // local only
#endif

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        if (!silent) std::cerr << "Failed to create server socket" << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    //Convert listen address
    if (inet_pton(AF_INET, listenAddress, &(serverAddress.sin_addr)) <= 0) {
        if (!silent) std::cerr << "Failed to convert listen address" << std::endl;
        return -1;
    }

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        if (!silent) std::cerr << "Failed to bind server socket" << std::endl;
        return -1;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        if (!silent) std::cerr << "Failed to start listening on server socket" << std::endl;
        return -1;
    }
    if (!silent)
        std::cout << "Server listening on: " << listenAddress << ":" << port << " (TCP)" << std::endl;
    server = true;
    return 1;
}

std::pair<SOCKET, sockaddr_in> TCPConnection::await_connection() {
    sockaddr_in clientAddress{};
    int clientAddressSize = sizeof(clientAddress);
    clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

    if (clientSocket == INVALID_SOCKET) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK)  if (!silent) std::cerr << "Failed to accept client connection : " << err << std::endl;
        return { INVALID_SOCKET, {} };
    }
    if (!silent)
        std::cout << "Connection from " << clientIP << ":" << ntohs(clientAddress.sin_port) << " established" << std::endl;
    
    disable_nagle();

    return { clientSocket, clientAddress };
}

int TCPConnection::establish_connection(const std::string& dummyAddress, int dummyPORT) {

    // for compatibility with UPD class
    if (dummyAddress != "") {   // if dummy address is provided take as actual variables
        strcpy_s(this->hostAddress, sizeof(this->hostAddress), dummyAddress.c_str());
        port = dummyPORT;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        if (!silent) std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    addrinfo hints{}, * result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostAddress, std::to_string(port).c_str(), &hints, &result) != 0) {
        if (!silent) std::cerr << "Failed to resolve server address" << std::endl;
        return -1;
    }

    if (connect(clientSocket, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCKET_ERROR) {
        freeaddrinfo(result);
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            if (!silent) std::cerr << "Connection failed : " << err << std::endl;
            return -1;
        }
        return err;
    }

    freeaddrinfo(result);
    if (!silent) std::cout << "Connection established" << std::endl;

    disable_nagle();

    return 1;
}

int TCPConnection::send_data(const char* data, int size) {
    int bytesSent = send(clientSocket, data, size, 0);
    if (bytesSent == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            if (!silent) std::cerr << "Failed to send data : " << err << std::endl;
            return -1;
        }
        return -err;
    }
    return bytesSent;
}
int TCPConnection::receive_data(char* buffer, int bufferSize) {
    int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesReceived == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            if (!silent) std::cerr << "Failed to receive data : " << err << std::endl;
            return -1;
        }
        return -err; //lets keep errors negative
    }
    return bytesReceived;
}

std::vector<char> TCPConnection::pack_header(const std::unordered_map<std::string, std::string>& header) {
    std::vector<char> headerData;
    for (const auto& pair : header) {
        const std::string& name = pair.first;
        const std::string& value = pair.second;
        uint32_t nameLength = static_cast<uint32_t>(name.length());
        uint32_t valueLength = static_cast<uint32_t>(value.length());

        // Pack name length
        char nameLengthBuffer[sizeof(uint32_t)];
        memcpy(nameLengthBuffer, &nameLength, sizeof(uint32_t));
        headerData.insert(headerData.end(), nameLengthBuffer, nameLengthBuffer + sizeof(uint32_t));

        // Pack name
        headerData.insert(headerData.end(), name.begin(), name.end());

        // Pack value length
        char valueLengthBuffer[sizeof(uint32_t)];
        memcpy(valueLengthBuffer, &valueLength, sizeof(uint32_t));
        headerData.insert(headerData.end(), valueLengthBuffer, valueLengthBuffer + sizeof(uint32_t));

        // Pack value
        headerData.insert(headerData.end(), value.begin(), value.end());
    }
    return headerData;
}
std::unordered_map<std::string, std::string> TCPConnection::unpack_header(const char* headerData, int dataSize) {
    std::unordered_map<std::string, std::string> header;

    int offset = 0;
    while (offset < dataSize) {
        // Unpack name length
        uint32_t nameLength;
        memcpy(&nameLength, headerData + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Unpack name
        std::string name(headerData + offset, headerData + offset + nameLength);
        offset += nameLength;

        // Unpack value length
        uint32_t valueLength;
        memcpy(&valueLength, headerData + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Unpack value
        std::string value(headerData + offset, headerData + offset + valueLength);
        offset += valueLength;

        // Add to header
        header[name] = value;
    }
    return header;
}

int TCPConnection::send_data(const char* data, int size, const std::unordered_map<std::string, std::string>& header) {
    // Pack header
    std::vector<char> headerData = pack_header(header);

    // Prepare the combined data to be sent
    std::vector<char> combinedData;
    uint32_t headerSize = static_cast<uint32_t>(headerData.size());
    combinedData.insert(combinedData.end(), reinterpret_cast<const char*>(&headerSize), reinterpret_cast<const char*>(&headerSize) + sizeof(uint32_t));
    combinedData.insert(combinedData.end(), headerData.begin(), headerData.end());
    combinedData.insert(combinedData.end(), data, data + size);

    // Send the combined data
    int bytesSent = send(clientSocket, combinedData.data(), combinedData.size(), 0);
    if (bytesSent == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            if (!silent) std::cerr << "Failed to send data : " << err << std::endl;
            return -1;
        }
        return err;
    }
    return bytesSent;
}

int TCPConnection::receive_data(char* buffer, int bufferSize, std::unordered_map<std::string, std::string>& header) {
    // Receive the combined data
    int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesReceived == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            if (!silent) std::cerr << "Failed to receive data : " << err << std::endl;
            return -1;
        }
        return err;
    }

    // Extract header size
    if (bytesReceived < sizeof(uint32_t)) {
        if (!silent) std::cerr << "Received data is incomplete." << std::endl;
        return -1;
    }
    uint32_t headerSize;
    std::memcpy(&headerSize, buffer, sizeof(uint32_t));

    // Extract header data
    if (bytesReceived < sizeof(uint32_t) + headerSize) {
        if (!silent) std::cerr << "Received data is incomplete." << std::endl;
        return -1;
    }
    const char* headerData = buffer + sizeof(uint32_t);
    header = unpack_header(headerData, headerSize);

    // Extract payload data
    int payloadSize = bytesReceived - sizeof(uint32_t) - headerSize;
    if (payloadSize > 0) {
        std::memcpy(buffer, buffer + sizeof(uint32_t) + headerSize, payloadSize);
    }
    return payloadSize;
}

void TCPConnection::set_silence(bool setting) {
    silent = setting;
}

int TCPConnection::set_server_blocking(bool block) {
    u_long mode = !block;
    return ioctlsocket(serverSocket, FIONBIO, &mode);
}
int TCPConnection::set_client_blocking(bool block) {
    u_long mode = !block;
    return ioctlsocket(clientSocket, FIONBIO, &mode);
}

void TCPConnection::disable_nagle() {
    constexpr int opt = -1;
    SOCKET& sock_fd = is_server() ? serverSocket : clientSocket;
    if (setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt))) {
        if (!silent) 
            std::cerr << "Error disabling Nagle's algorithm" << std::endl;
    }
}

void TCPConnection::setSocketTimeout(int socket, int timeoutMillisec) {
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

void TCPConnection::set_server_timeout(int timeoutMillisec) {
    setSocketTimeout(serverSocket, timeoutMillisec);
}
void TCPConnection::set_client_timeout(int timeoutMillisec) {
    setSocketTimeout(clientSocket, timeoutMillisec);
}

std::string TCPConnection::get_local_ip() {
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

std::string TCPConnection::get_external_ip() {
    char addr[16];
    identme(addr, sizeof(addr));
    return addr;
}

int TCPConnection::get_available_data_size() {
    u_long availableBytes = 0;
    if (ioctlsocket(clientSocket, FIONREAD, &availableBytes) != 0) {
        std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
        return -1; // Error occurred
    }
    return static_cast<int>(availableBytes);
}
int TCPConnection::receive_null_data(int count) {
    char* tempBuffer = new char[count];
    int bytesReceived = recv(clientSocket, tempBuffer, count, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
        delete[] tempBuffer;
        return -1; // Error occurred
    }
    delete[] tempBuffer;
    return bytesReceived;
}

#endif // TCP_CONNECTION_H