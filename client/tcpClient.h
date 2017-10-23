#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>

#include "../gen_settings.cpp"

class tcpClient {
    private:
        int socketFd;
        enum commands comm;

        void prepareSocket();
    public:
        tcpClient();
        virtual ~tcpClient();
        
        void sequentialSession();
        void randomSession();
};

#endif // TCPCLIENT_H
