#include "tcpServer.h"

tcpServer::tcpServer() {
    this->sessionCounter = 1;
    this->prepareSocket();
}

tcpServer::~tcpServer() {
    close(this->socketFd);
}

const int tcpServer::getSessionCounter() const {
    return this->sessionCounter;
}

void tcpServer::startServer() {
    struct timeval tv;

    tv.tv_sec = SESSION_TIME;
    tv.tv_usec = 0;

    std::cout << "server: waiting for connections..." << std::endl;

    while (1) {
        char clientIp[INET6_ADDRSTRLEN];
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof(their_addr);
        int newSock = accept(this->socketFd, (struct sockaddr *)&their_addr, &sin_size);

        if (newSock < 0) {
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

        std::thread conn(&tcpServer::handleConnection, this, newSock, clientIp, this->getSessionCounter());
        conn.detach();

        this->sessionCounter += 1;
    }
}

void tcpServer::handleConnection(int newSock, const char* clientIp, int uniqCounter) {
    char response[MAXBUFSIZE];
    int numbytes;
    enum commands comm;
    FILE* logs;

    logs = fopen(LOGS_FILENAME, "a");

    while(1) {
        if ((numbytes = recv(newSock, &comm, sizeof(comm), 0)) < 0) {
            perror("server: recv error");
            break;
        } else if (numbytes == 0) {
            break;  
        }

        switch (comm) {
            case(TIME):
                this->getCurrentTime(response);
                this->logMessage(clientIp, "TIME", logs);

                break;
            case(SESSION):
                sprintf(response, "%d", uniqCounter);
                this->logMessage(clientIp, "SESSION", logs);
                
                break;
            case(END):
                this->logMessage(clientIp, "END", logs);
                close(newSock);
                fclose(logs);

                return;
            default:
                strcpy(response, "default");
                break;
        }

        if (send(newSock, response, MAXBUFSIZE-1, 0) < 0) {
            perror("server: send error");
            break;
        }
    }

        fclose(logs);
        close(newSock);
}

void* tcpServer::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void tcpServer::logMessage(const char* ip, const char* command, FILE* logs) {
    char timeStr[MAXBUFSIZE];
    struct tm* timeinfo;
    time_t rawtime;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, MAXBUFSIZE, "%c", timeinfo);
    
    fprintf(logs, "(%s)  Client IP-%s  Client command-%s \n", timeStr, ip, command);
}

void tcpServer::getCurrentTime(char* message) {
    struct tm* timeinfo;
    time_t rawtime;
    
    time(&rawtime);

    timeinfo = localtime(&rawtime);

    strftime(message, MAXBUFSIZE, "%Y-%m-%d %X", timeinfo);
}

void tcpServer::prepareSocket() {
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;

    std::memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        throw std::runtime_error("server: getaddrinfo");
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
            perror("server: setsockopt error");
            continue;
            // throw std::runtime_error("server: setsockopt error");
        }

        if (bind(this->socketFd, p->ai_addr, p->ai_addrlen) < 0) {
            close(this->socketFd);
            perror("server: bind");
            continue;
        }

        break;
    }


    if (p == NULL)  {
        throw std::runtime_error("server: failed to bind");
    }

    freeaddrinfo(servinfo); 
    

    if (listen(this->socketFd, MAXCLIENTS) < 0) {
        throw std::runtime_error("server: listen error");
    }
}
