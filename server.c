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
#define SESSION_TIME 60

enum commands {TIME, SESSION, END};


void LogMessage(const char* ip, const char* command);
void *get_in_addr(struct sockaddr *sa);
void getCurrentTime(char* message);


void getCurrentTime(char* message) {
    struct tm* timeinfo;
    time_t rawtime;
    
    time(&rawtime);

    timeinfo = localtime(&rawtime);

    strftime(message, MAXBUFSIZE, "%Y-%m-%d %X", timeinfo);
}

void LogMessage(const char* ip, const char* command) {
    char timeStr[MAXBUFSIZE];
    struct tm* timeinfo;
    FILE* logs;
    time_t rawtime;
    
    logs = fopen(LOGS_FILENAME, "a");

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, MAXBUFSIZE, "%c", timeinfo);
    
    fprintf(logs, "(%s)  Client IP-%s  Client command-%s \n", timeStr, ip, command);

    fclose(logs);
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void) {
    enum commands comm;
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    struct timeval tv;
    socklen_t sin_size;
    int yes = 1;
    char cli_ip[INET6_ADDRSTRLEN];
    int rv, numbytes;


    tv.tv_sec = SESSION_TIME;
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) < 0) {
            perror("server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("server: setsockopt error");
        exit(1);
    }


    freeaddrinfo(servinfo); 

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }


    if (listen(sockfd, BACKLOG) < 0) {
        perror("server: listen error");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd < 0) {
            perror("server: accept error");
            continue;
        }

        if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval)) < 0) {
            perror("server: setsockopt error");
            exit(1);
        }


        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            cli_ip, sizeof cli_ip);

        if (!fork()) { // this is the child process
            char response[MAXBUFSIZE];

            bzero(response, MAXBUFSIZE);
            close(sockfd);

            while(1) {
                if ((numbytes = recv(new_fd, &comm, sizeof(comm), 0)) < 0) {
                    perror("server: recv error");
                    break;
                } else if (numbytes == 0) {
                    break;  
                }

                switch (comm) {
                    case(TIME):
                        getCurrentTime(response);
                        LogMessage(cli_ip, "TIME");

                        break;
                    case(SESSION):
                        strcpy(response, "session");
                        break;
                    case(END):
                        strcpy(response, "end");
                        break;
                    default:
                        strcpy(response, "def");
                        break;
                }



                if (send(new_fd, response, MAXBUFSIZE-1, 0) < 0) {
                    perror("server: send error");
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