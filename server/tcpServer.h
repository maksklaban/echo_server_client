#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <cstdio>
#include <iostream>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <thread>
#include <exception>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../gen_settings.cpp"


class tcpServer {
    private:
        int socketFd;
        int sessionCounter;
        
        void prepareSocket();
        void handleConnection(int newSocket, const char* ip);
        void* get_in_addr(struct sockaddr *sa);
        void logMessage(const char* ip, const char* message, FILE* logs);
        void getCurrentTime(char* message);
    public:
        tcpServer();
        virtual ~tcpServer();

        const int getSessionCounter() const;

        void startServer();
};

#endif // TCPSERVER_H
