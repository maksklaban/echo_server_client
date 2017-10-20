#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <limits.h>
#include <signal.h>

#include "../gen_settings.c"

class tcpServer {
    private:
        int socketFd;
        int port;
        int maxClients;

        void sigchld_handler(int s);
        void *get_in_addr(struct sockaddr *sa);
        void logMessage(const char* ip, const char* message, std::ofstream file);
        void handleConnection(int newSocket, const char* ip);
        int prepareSocket();
    public:
        tcpServer(int port, int maxClients, std::string node = NULL);
        virtual ~tcpServer();

        const int getPort() const;
        const int getMaxClients() const;
        const std::string getNode() const;

        void startServer(int timeout);
};

#endif // TCPSERVER_H
