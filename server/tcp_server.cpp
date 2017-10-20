#include "tcp_server.h"

tcpServer::tcpServer(int port, int maxClients, std::string node) {
    this->port = port;
    this->maxClients = maxClients;
    this->socketFd = this->prepareSocket();
}

tcpServer::~tcpServer() {
    close(this->socketFd);
}

const int tcpServer::getMaxClients() const {
    return this->maxClients;
}

const int tcpServer::getPort() const {
    return this->port;
}

const std::string tcpServer::getNode() const {
    return this->node;
}

void tcpServer::startServer(int timeout) {
    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    std::cout << "server: waiting for connections..." << std::endl;

    while (1) {
        char clientIp[INET6_ADDRSTRLEN];
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof(their_addr);
        int newSock = accept(this->socketFd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd < 0) {
            perror("server: accept error");
            continue;
        }

        if (setsockopt(newSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval)) < 0) {
            perror("server: setsockopt error");
            close(newSock);
            continue;
        }

        inet_ntop(their_addr.ss_family, 
            get_in_addr((struct sockaddr *)&their_addr),
            clientIp, sizeof clientIp);

        std::thread conn(&tcpServer::handleConnection, this, newSock, clientIp);
        conn.detach();
    }
}


int tcpServer::prepareSocket() {
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    struct sigaction sa;
    int yes = 1;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(this->node, this->port, &hints, &servinfo)) != 0) {
        throw SocketException("server: getaddrinfo");
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((this->socketFd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) < 0) {
            perror("server: socket init");
            continue;
        }

        //Handle "Address already in use" error message
        if (setsockopt(this->socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
            throw SocketException("server: setsockopt error");
        }

        if (bind(this->socketFd, p->ai_addr, p->ai_addrlen) < 0) {
            close(this->socketFd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); 

    if (p == NULL)  {
        throw SocketException("server: failed to bind");
    }


    if (listen(this->socketFd, this->maxClients) < 0) {
        throw SocketException("server: listen error");
    }

    sa.sa_handler = this->sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        throw SocketException("server: sigaction");
    }
}
