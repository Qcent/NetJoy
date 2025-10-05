
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

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <winsock2.h>

#include "TCP_Connection_Class.h"
#include "UDP_Connection_Class.h"

bool UDP_COMMUNICATION = false;
/*
#if UDP_COMMUNICATION
using NetworkConnection = UDPConnection;
#else
using NetworkConnection = TCPConnection;
#endif
*/


// failed attempt at duck typing below. UDP/TCP classes are destructed right after creation (don't understand move semantics enough? can't get working)
/*

class xXxNetworkConnection {

    struct _Uninitialized {

        std::string get_local_ip() { return "Uninitialized"; }
        std::string get_external_ip() { return "Uninitialized"; }
        void set_silence(bool) {}
        int set_server_blocking(bool) { return -1; }
        int set_client_blocking(bool) { return -1; }
        void set_server_timeout(int) {}
        void set_client_timeout(int) {}

        int start_as_server(int) { return -1; }
        std::pair<SOCKET, sockaddr_in> await_connection() {
            return { INVALID_SOCKET, sockaddr_in{} };
        }
        int establish_connection(const std::string&, int) { return -1; }

        int send_data(const char*, int) { return -1; }
        int receive_data(char*, int) { return -1; }

        int get_available_data_size() { return -1; }
        int receive_null_data(int) { return -1; }
        bool is_server() { return false; }
    };

public:
    xXxNetworkConnection()
        : self(std::make_unique<Model<_Uninitialized>>(_Uninitialized{})) {}

    template <typename T>
    xXxNetworkConnection(T&& conn)
        : self(std::make_unique<Model<std::decay_t<T>>>(std::forward<T>(conn))) {
    }

    // enable moves
    xXxNetworkConnection(xXxNetworkConnection&&) noexcept = default;
    xXxNetworkConnection& operator=(xXxNetworkConnection&&) noexcept = default;

    // disable copies
    xXxNetworkConnection(const xXxNetworkConnection&) = delete;
    xXxNetworkConnection& operator=(const xXxNetworkConnection&) = delete;


    // Handy Helpers
    std::string get_local_ip() { return self->get_local_ip(); }
    std::string get_external_ip() { return self->get_external_ip(); }
    void set_silence(bool setting) { self->set_silence(setting); }
    int set_server_blocking(bool block = true) { return self->set_server_blocking(block); }
    int set_client_blocking(bool block = true) { return self->set_client_blocking(block); }
    void set_server_timeout(int timeoutMs) { self->set_server_timeout(timeoutMs); }
    void set_client_timeout(int timeoutMs) { self->set_client_timeout(timeoutMs); }

    // Server-related
    int start_as_server(int port = 0) { return self->start_as_server(port); }
    std::pair<SOCKET, sockaddr_in> await_connection() { return self->await_connection(); }

    // Client-related
    int establish_connection(const std::string& addr = "", int port = 0) {
        return self->establish_connection(addr, port);
    }

    // Data communication
    int send_data(const char* data, int size) {
        return self->send_data(data, size);
    }
    int receive_data(char* buffer, int size) {
        return self->receive_data(buffer, size);
    }

    // Misc
    int get_available_data_size() { return self->get_available_data_size(); }
    int receive_null_data(int c) { return self->receive_null_data(c); }
    bool is_server() { return self->is_server(); }

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual std::string get_local_ip() = 0;
        virtual std::string get_external_ip() = 0;
        virtual void set_silence(bool) = 0;
        virtual int set_server_blocking(bool) = 0;
        virtual int set_client_blocking(bool) = 0;
        virtual void set_server_timeout(int) = 0;
        virtual void set_client_timeout(int) = 0;

        virtual int start_as_server(int) = 0;
        virtual std::pair<SOCKET, sockaddr_in> await_connection() = 0;
        virtual int establish_connection(const std::string&, int) = 0;

        virtual int send_data(const char*, int) = 0;
        virtual int receive_data(char*, int) = 0;
        //virtual int send_data(const char*, int, const std::unordered_map<std::string, std::string>&) = 0;
        //virtual int receive_data(char*, int, std::unordered_map<std::string, std::string>&) = 0;

        virtual int get_available_data_size() = 0;
        virtual int receive_null_data(int) = 0;
        virtual bool is_server() = 0;
    };

    template <typename T>
    struct Model : Concept {
        T impl;
        Model(T x) : impl(std::move(x)) {}

        std::string get_local_ip() override { return impl.get_local_ip(); }
        std::string get_external_ip() override { return impl.get_external_ip(); }
        void set_silence(bool s) override { impl.set_silence(s); }
        int set_server_blocking(bool b) override { return impl.set_server_blocking(b); }
        int set_client_blocking(bool b) override { return impl.set_client_blocking(b); }
        void set_server_timeout(int t) override { impl.set_server_timeout(t); }
        void set_client_timeout(int t) override { impl.set_client_timeout(t); }

        int start_as_server(int p) override { return impl.start_as_server(p); }
        std::pair<SOCKET, sockaddr_in> await_connection() override { return impl.await_connection(); }
        int establish_connection(const std::string& a, int p) override { return impl.establish_connection(a, p); }

        int send_data(const char* d, int s) override { return impl.send_data(d, s); }
        int receive_data(char* b, int s) override { return impl.receive_data(b, s); }
        //int send_data(const char* d, int s, const std::unordered_map<std::string, std::string>& h) override { return impl.send_data(d, s, h); }
        //int receive_data(char* b, int s, std::unordered_map<std::string, std::string>& h) override { return impl.receive_data(b, s, h); }

        int get_available_data_size() override { return impl.get_available_data_size(); }
        int receive_null_data(int c) override { return impl.receive_null_data(c); }
        bool is_server() override { return impl.is_server(); }
    };

    std::unique_ptr<Concept> self;
};

*/


// Fully Working Ducktyping below !!??? hopefully
class NetworkConnection {
    struct Concept {
        virtual ~Concept() = default;

        // Common network interface
        virtual std::string get_local_ip() = 0;
        virtual std::string get_external_ip() = 0;
        virtual void set_silence(bool) = 0;
        virtual int set_server_blocking(bool) = 0;
        virtual int set_client_blocking(bool) = 0;
        virtual void set_server_timeout(int) = 0;
        virtual void set_client_timeout(int) = 0;
        virtual int start_as_server(int) = 0;
        virtual std::pair<SOCKET, sockaddr_in> await_connection() = 0;
        virtual int establish_connection(const std::string&, int) = 0;
        virtual int send_data(const char*, int) = 0;
        virtual int receive_data(char*, int) = 0;
        virtual int get_available_data_size() = 0;
        virtual int receive_null_data(int) = 0;
        virtual bool is_server() = 0;
    };

    template <typename T>
    struct Model : Concept {
        T impl;
        template <typename... Args>
        explicit Model(Args&&... args) : impl(std::forward<Args>(args)...) {}

        std::string get_local_ip() override { return impl.get_local_ip(); }
        std::string get_external_ip() override { return impl.get_external_ip(); }
        void set_silence(bool s) override { impl.set_silence(s); }
        int set_server_blocking(bool b) override { return impl.set_server_blocking(b); }
        int set_client_blocking(bool b) override { return impl.set_client_blocking(b); }
        void set_server_timeout(int t) override { impl.set_server_timeout(t); }
        void set_client_timeout(int t) override { impl.set_client_timeout(t); }
        int start_as_server(int p) override { return impl.start_as_server(p); }
        std::pair<SOCKET, sockaddr_in> await_connection() override { return impl.await_connection(); }
        int establish_connection(const std::string& a, int p) override { return impl.establish_connection(a, p); }
        int send_data(const char* d, int s) override { return impl.send_data(d, s); }
        int receive_data(char* b, int s) override { return impl.receive_data(b, s); }
        int get_available_data_size() override { return impl.get_available_data_size(); }
        int receive_null_data(int c) override { return impl.receive_null_data(c); }
        bool is_server() override { return impl.is_server(); }
    };

    struct _Uninitialized : Concept {
        std::string get_local_ip() override { return "Uninitialized"; }
        std::string get_external_ip() override { return "Uninitialized"; }
        void set_silence(bool) override {}
        int set_server_blocking(bool) override { return -1; }
        int set_client_blocking(bool) override { return -1; }
        void set_server_timeout(int) override {}
        void set_client_timeout(int) override {}
        int start_as_server(int) override { return -1; }
        std::pair<SOCKET, sockaddr_in> await_connection() override { return { INVALID_SOCKET, sockaddr_in{} }; }
        int establish_connection(const std::string&, int) override { return -1; }
        int send_data(const char*, int) override { return -1; }
        int receive_data(char*, int) override { return -1; }
        int get_available_data_size() override { return -1; }
        int receive_null_data(int) override { return -1; }
        bool is_server() override { return false; }
    };

public:
    // Default uninitialized constructor
    NetworkConnection()
        : self(std::make_unique<Model<_Uninitialized>>()) {
    }

    // Main constructor: UDP or TCP based on bool
    template <typename... Args>
    explicit NetworkConnection(bool useUDP, Args&&... args) {
        if (useUDP) {
            self = std::make_unique<Model<UDPConnection>>(std::forward<Args>(args)...);
        }
        else {
            self = std::make_unique<Model<TCPConnection>>(std::forward<Args>(args)...);
        }
    }

    // Enable moves
    NetworkConnection(NetworkConnection&&) noexcept = default;
    NetworkConnection& operator=(NetworkConnection&&) noexcept = default;

    // Disable copies
    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection& operator=(const NetworkConnection&) = delete;

    // Forward all calls
    std::string get_local_ip() { return self->get_local_ip(); }
    std::string get_external_ip() { return self->get_external_ip(); }
    void set_silence(bool s) { self->set_silence(s); }
    int set_server_blocking(bool b = true) { return self->set_server_blocking(b); }
    int set_client_blocking(bool b = true) { return self->set_client_blocking(b); }
    void set_server_timeout(int t) { self->set_server_timeout(t); }
    void set_client_timeout(int t) { self->set_client_timeout(t); }
    int start_as_server(int port = 0) { return self->start_as_server(port); }
    std::pair<SOCKET, sockaddr_in> await_connection() { return self->await_connection(); }
    int establish_connection(const std::string& addr = "", int port = 0) {
        return self->establish_connection(addr, port);
    }
    int send_data(const char* data, int size) { return self->send_data(data, size); }
    int receive_data(char* buffer, int size) { return self->receive_data(buffer, size); }
    int get_available_data_size() { return self->get_available_data_size(); }
    int receive_null_data(int c) { return self->receive_null_data(c); }
    bool is_server() { return self->is_server(); }

private:
    std::unique_ptr<Concept> self;


    static std::vector<char> pack_header(const std::unordered_map<std::string, std::string>& header) {
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
    static std::unordered_map<std::string, std::string> unpack_header(const char* headerData, int dataSize) {
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
};