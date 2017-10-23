#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../gen_settings.cpp"

class tcpClient {
    private:
        int socketFd;
        int comm;

        void prepareSocket();
    public:
        tcpClient();
        virtual ~tcpClient();
        
        void sequentialSession();
        void randomSession();
};

#endif // TCPCLIENT_H
