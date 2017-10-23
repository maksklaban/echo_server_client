#include "tcpClient.h"

tcpClient::tcpClient() {
    this->prepareSocket();
}

tcpClient::~tcpClient() {
    close(this->socketFd);
}

void tcpClient::prepareSocket() {
    struct addrinfo hints, *servinfo, *p;

    std::memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((getaddrinfo(SERVER, PORT, &hints, &servinfo)) != 0) {
        throw std::runtime_error("client: getaddrinfo");
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((this->socketFd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) < 0) {
            perror("client: socket init");
            continue;
        }

        if (connect(this->socketFd, p->ai_addr, p->ai_addrlen) == -1) {
            close(this->socketFd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        throw std::runtime_error("client: failed to connect");
    }

    freeaddrinfo(servinfo); 
}

void tcpClient::sequentialSession() {
    char buf[MAXBUFSIZE];
    int numbytes;

    this->comm = 0;

    while (1) {
        if ((numbytes = send(this->socketFd, &comm , sizeof(comm), 0)) < 0) {
            throw std::runtime_error("client: send error");
        }

        if ((numbytes = recv(this->socketFd, &buf, MAXBUFSIZE, 0)) < 0) {
            throw std::runtime_error("client: recv error");
        } else if (numbytes == 0) {
            return;
        }

        buf[numbytes] = '\0';

        std::cout << "response - " << buf << std::endl;

        this->comm += 1;

        sleep(SEQTIMEOUT);
    }
}

void tcpClient::randomSession() {
    char buf[MAXBUFSIZE];
    int numbytes;

    srand(time(NULL));

    this->comm = rand() % 3;

    while (1) {
        if ((numbytes = send(this->socketFd, &comm , sizeof(comm), 0)) < 0) {
            throw std::runtime_error("client: send error");
        }

        if ((numbytes = recv(this->socketFd, &buf, MAXBUFSIZE, 0)) < 0) {
            throw std::runtime_error("client: recv error");
        } else if (numbytes == 0) {
            return;
        }

        buf[numbytes] = '\0';

        std::cout << "response - " << buf << std::endl;

        this->comm = (rand() % 3);

        sleep(SEQTIMEOUT);
    }
}