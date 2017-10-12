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


#define PORT "50000"
#define BACKLOG 10
#define LOGS_FILENAME "logs.txt"
#define MAXBUFSIZE 100
#define SESSION_TIME 60


enum commands {TIME, SESSION, END};


void sigchld_handler(int s);
void LogMessage(const char* ip, const char* command, FILE* logs);
void *get_in_addr(struct sockaddr *sa);
void getCurrentTime(char* message);
void startEchoServer();


void sigchld_handler(int s) {
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void LogMessage(const char* ip, const char* command, FILE* logs) {
    char timeStr[MAXBUFSIZE];
    struct tm* timeinfo;
    time_t rawtime;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeStr, MAXBUFSIZE, "%c", timeinfo);
    
    fprintf(logs, "(%s)  Client IP-%s  Client command-%s \n", timeStr, ip, command);
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getCurrentTime(char* message) {
    struct tm* timeinfo;
    time_t rawtime;
    
    time(&rawtime);

    timeinfo = localtime(&rawtime);

    strftime(message, MAXBUFSIZE, "%Y-%m-%d %X", timeinfo);
}

void startEchoServer() {
    enum commands comm;
    FILE* logs;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    struct timeval tv;
    struct sigaction sa;
    socklen_t sin_size;
    int yes = 1;
    int sockfd, new_fd;
    int rv, numbytes;
    char cli_ip[INET6_ADDRSTRLEN];

    logs = fopen(LOGS_FILENAME, "a");

    //Need for socket timeout opt
    tv.tv_sec = SESSION_TIME;
    tv.tv_usec = 0;

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

        //Handle "Address already in use" error message
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
            perror("server: setsockopt error");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
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

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("server: sigaction");
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
        //set socket timeout opt
        if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval)) < 0) {
            perror("server: setsockopt error");
            exit(1);
        }

        //Conver client IP to string
        inet_ntop(their_addr.ss_family, 
            get_in_addr((struct sockaddr *)&their_addr),
            cli_ip, sizeof cli_ip);

        if (!fork()) { // this is the child process
            char response[MAXBUFSIZE];

            bzero(response, MAXBUFSIZE);
            //Close main socket
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
                        LogMessage(cli_ip, "TIME", logs);

                        break;
                    case(SESSION):
                        sprintf(response, "%d", getpid());
                        LogMessage(cli_ip, "SESSION", logs);
                        
                        break;
                    case(END):
                        LogMessage(cli_ip, "END", logs);
                        close(new_fd);
                        fclose(logs);

                        exit(0);
                    default:
                        strcpy(response, "default");
                        break;
                }


                if (send(new_fd, response, MAXBUFSIZE-1, 0) < 0) {
                    perror("server: send error");
                    exit(1);
                }
            }


            fclose(logs);
            close(new_fd);
            exit(0);
        }
        fclose(logs);
        close(new_fd);  // parent doesn't need this
    }
}

int main(int argc, char *argv[]) {
    startEchoServer();

    return 0;
}