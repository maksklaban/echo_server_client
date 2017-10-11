#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT "50000"

#define BACKLOG 10
#define LOGS_FILENAME "logs.txt"
#define MAXBUFSIZE 100
#define TIME "t"
#define SESSION "s"
#define END "e"


void LogMessage(const char* message, const char* error);
void *get_in_addr(struct sockaddr *sa);


void LogMessage(const char* message, const char* error) {
    struct tm* timeinfo;
    FILE* logs;
    time_t rawtime;
    
    logs = fopen(LOGS_FILENAME, "a");

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    fprintf(logs, "%s %s !%s", message, error, asctime(timeinfo));

    fclose(logs);
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void) {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    struct timeval tv;
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv, numbytes;

    tv.tv_sec = 5;
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        LogMessage("getaddrinfo error", gai_strerror(rv));
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            LogMessage("socket create error ", strerror(errno));
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            LogMessage("bind error ", strerror(errno));
            continue;
        }

        break;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        LogMessage("setsockopt error ", strerror(errno));
        exit(1);
    }


    freeaddrinfo(servinfo); 

    if (p == NULL)  {
        LogMessage("server bind error ", "");
        exit(1);
    }


    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            LogMessage("accept error", "");
            continue;
        }

        if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval)) == -1) {
            LogMessage("setsockopt error ", strerror(errno));
            exit(1);
        }


        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        LogMessage("got connection from", s);

        if (!fork()) { // this is the child process
            char command;
            char response[MAXBUFSIZE];

            bzero()
            close(sockfd); // child doesn't need the listener

            while(1) {
                if ((numbytes = recv(new_fd, command, 1, 0)) < 0) {
                    LogMessage("recv error", strerror(errno));
                    break;
                } else if (numbytes == 0) {
                    LogMessage("Client close connection", "");
                    break;  
                }

                switch (command) {
                    case(TIME):
                        str
                        response = "time";

                }


                LogMessage(buff, "");

                if (send(new_fd, buff, MAXBUFSIZE-1, 0) < 0) {
                    LogMessage("send error", strerror(errno));
                    exit(1);
                }
            }

            // if (send(new_fd, "Hello, world!", 13, 0) == -1) {
            //     perror("send");
            // }

            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}