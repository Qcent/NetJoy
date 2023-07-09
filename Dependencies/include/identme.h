#pragma once
/* Simple C library for querying your IP address using http://ident.me
 *
 * Copyright (c) 2015-2022  Joachim Wiberg <troglobit@gmail.com> // ipify
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 // Modified for windows by Dave Quinn <qcent@yahoo.com> 15/06/2023

#ifndef IDENTME_H_
#define IDENTME_H_

#include <stddef.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define IDENTME_HOST   "ident.me"
#define HTTP_REQUEST "GET / HTTP/1.0\r\nHost: " IDENTME_HOST "\r\nUser-Agent: Qcent \r\n\r\n"

int identme_connect(void);
int identme_connect1(int family);
int identme_connect2(const char* host, int family);
int identme_query(int sd, char* addr, size_t len);
int identme_disconnect(int sd);
int identme(char* addr, size_t len);


int identme_connect2(const char* host, int family)
{
    struct addrinfo* info, * ai;
    struct addrinfo hints;
    int rc, sd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;

    rc = getaddrinfo(host, "http", &hints, &info);
    if (rc || !info)
        return -1;

    for (ai = info; ai; ai = ai->ai_next) {
        sd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sd == INVALID_SOCKET)
            continue;

        rc = connect(sd, ai->ai_addr, static_cast<int>(ai->ai_addrlen));
        if (rc == SOCKET_ERROR) {
            closesocket(sd);
            sd = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(info);

    return sd;
}

int identme_connect1(int family)
{
    return identme_connect2(IDENTME_HOST, family);
}

int identme_connect(void)
{
    return identme_connect1(AF_UNSPEC);
}

int identme_query(int sd, char* addr, size_t len)
{
    char tmp[sizeof(struct in6_addr)];
    const char* req = HTTP_REQUEST;
    char buf[512], * ptr;
    int rc;
    int family;

    rc = send(sd, req, static_cast<int>(strlen(req)), 0);
    if (rc == SOCKET_ERROR)
        return -1;

    rc = recv(sd, buf, sizeof(buf), 0);
    if (rc == SOCKET_ERROR)
        return -1;
#pragma warning(suppress: 6386)
    buf[rc] = 0;

    ptr = strstr(buf, "200 OK");
    if (!ptr)
        return 1;

    ptr = strstr(ptr, "\r\n\r\n");
    if (!ptr)
        return 1;
    ptr += 4;

    family = AF_INET;
    if (!inet_pton(family, ptr, tmp)) {
        family = AF_INET6;
        if (!inet_pton(family, ptr, tmp))
            return 1;
    }

    if (!inet_ntop(family, tmp, addr, len))
        return 1;

    return 0;
}

int identme_disconnect(int sd)
{
    shutdown(sd, SD_BOTH);
    return closesocket(sd);
}

int identme(char* addr, size_t len)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    int sd, ret;
    sd = identme_connect();
    if (sd == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    ret = identme_query(sd, addr, len);
    ret |= identme_disconnect(sd);

    WSACleanup();

    return ret;
}

#endif /* IDENTME_H_ */