
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

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <winsock2.h>

#include "TCP_Connection_Class.h"
#include "UDP_Connection_Class.h"

class NetworkConnection {
public:
    template <typename T>
    NetworkConnection(T conn) : self(std::make_shared<Model<T>>(std::move(conn))) {}

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
/*   int send_data(const char* data, int size,
        const std::unordered_map<std::string, std::string>& header) {
        return self->send_data(data, size, header);
    }*/ 
    int receive_data(char* buffer, int size) {
        return self->receive_data(buffer, size);
    }
   /* int receive_data(char* buffer, int size,
        std::unordered_map<std::string, std::string>& header) {
        return self->receive_data(buffer, size, header);
    }*/

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
   //     virtual int send_data(const char*, int, const std::unordered_map<std::string, std::string>&) = 0;
        virtual int receive_data(char*, int) = 0;
    //    virtual int receive_data(char*, int, std::unordered_map<std::string, std::string>&) = 0;

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
//        int send_data(const char* d, int s, const std::unordered_map<std::string, std::string>& h) override { return impl.send_data(d, s, h); }
        int receive_data(char* b, int s) override { return impl.receive_data(b, s); }
//        int receive_data(char* b, int s, std::unordered_map<std::string, std::string>& h) override { return impl.receive_data(b, s, h); }

        int get_available_data_size() override { return impl.get_available_data_size(); }
        int receive_null_data(int c) override { return impl.receive_null_data(c); }
        bool is_server() override { return impl.is_server(); }
    };

    std::shared_ptr<Concept> self;
};
