/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   listener layer management

*/


#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h> // for sockaddr_in
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils/log.hpp"
#include "utils/invalid_socket.hpp"
#include "core/server.hpp"

#if !defined(IP_TRANSPARENT)
#define IP_TRANSPARENT 19
#endif

struct Listen {
    Server & server;
    uint32_t s_addr;
    int port;
    int sck;
    bool exit_on_timeout;
    int timeout_sec;

    Listen(Server & server, uint32_t s_addr, int port, bool exit_on_timeout = false, int timeout_sec = 60, bool enable_ip_transparent = false)
        : server(server)
        , s_addr(s_addr)
        , port(port)
        , sck(0)
        , exit_on_timeout(exit_on_timeout)
        , timeout_sec(timeout_sec)
    {
        struct Socket
        {
            int sck;
            Socket()
             : sck(INVALID_SOCKET)
            {
                this->sck = socket(PF_INET, SOCK_STREAM, 0);
            }
            ~Socket(){
                if (this->sck != INVALID_SOCKET){
                    close(this->sck);
                }
            }
        } sck; 
    
        /* reuse same port if a previous daemon was stopped */
        int allow_reuse = 1;
        setsockopt(sck.sck, SOL_SOCKET, SO_REUSEADDR, &allow_reuse, sizeof(allow_reuse));

        /* set snd buffer to at least 32 Kbytes */
        int snd_buffer_size = 32768;
        unsigned int option_len = sizeof(snd_buffer_size);
        if (0 == getsockopt(this->sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
            if (snd_buffer_size < 32768) {
                snd_buffer_size = 32768;
                setsockopt(sck.sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, sizeof(snd_buffer_size));
            }
        }

        /* set non blocking */
        fcntl(sck.sck, F_SETFL, fcntl(sck.sck, F_GETFL) | O_NONBLOCK);

        union
        {
          struct sockaddr s;
          struct sockaddr_storage ss;
          struct sockaddr_in s4;
          struct sockaddr_in6 s6;
        } u;
        memset(&u, 0, sizeof(u));
        u.s4.sin_family = AF_INET;
        u.s4.sin_port = htons(this->port);
//        u.s4.sin_addr.s_addr = INADDR_ANY;
        u.s4.sin_addr.s_addr = this->s_addr;

        LOG(LOG_INFO, "Listen: binding socket %d on %s:%d", sck.sck, ::inet_ntoa(u.s4.sin_addr), this->port);
        if (0 != ::bind(sck.sck, &u.s, sizeof(u))) {
            LOG(LOG_ERR, "Listen: error binding socket [errno=%u] %s", errno, strerror(errno));
            return;
        }

        if (enable_ip_transparent) {
            LOG(LOG_INFO, "Enable transparent proxying on listened socket.\n");
            int optval = 1;

            if (setsockopt(sck.sck, SOL_IP, IP_TRANSPARENT, &optval, sizeof(optval))) {
                LOG(LOG_ERR, "Failed to enable transparent proxying on listened socket.\n");
                return;
            }
        }

        LOG(LOG_INFO, "Listen: listening on socket %d", sck.sck);
        if (0 != listen(sck.sck, 2)) {
            LOG(LOG_ERR, "Listen: error listening on socket\n");
        }

        // OK, keep the temporary socket everything was fine
        this->sck = sck.sck;
        // This to avoid closing old temporary socket through RAII
        sck.sck = INVALID_SOCKET;
        return;
    }

    ~Listen()
    {
        if (this->sck != INVALID_SOCKET) {
            shutdown(this->sck, 2);
            close(this->sck);
            this->sck = INVALID_SOCKET;
        }
    }

    // TODO Some values (server, timeout) become only necessary when calling check
    void run() {
        bool loop_listener = true;
        while (loop_listener) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(this->sck, &rfds);
            struct timeval timeout;
            timeout.tv_sec = this->timeout_sec;
            timeout.tv_usec = 0;

            switch (select(this->sck + 1, &rfds, nullptr, nullptr, &timeout)){
            default:
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINPROGRESS) || (errno == EINTR)) {
                    continue; /* these are not really errors */
                }
                LOG(LOG_WARNING, "socket error detected in listen (%s)\n", strerror(errno));
                loop_listener = false;
                break;
            case 0:
                if (this->exit_on_timeout){
                    LOG(LOG_INFO, "Listener exiting on timeout");
                    loop_listener = false;
                }
            break;
            case 1:
            {
                Server::Server_status res = this->server.start(this->sck);
                if (Server::START_WANT_STOP == res){
                    loop_listener = false;
                }
            }
            break;
            }
        }
    }
};

