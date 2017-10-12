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

#define PORT "50000"
#define SERVER "localhost"

#define MAXDATASIZE 100 
#define SEQTIMEOUT 10

enum commands {TIME, SESSION, END};

void print_help(const char* app_name) {
    printf("Usage: %s [OPTION]\n", app_name);
    printf(" OPTIONS:\n");
    printf(" -h                'print this help'\n");
    printf(" -r                'for a chaotic sequence of queries'\n");
    printf(" -o                'for an ordered query sequence'\n");
}

int main(int argc, char *argv[]) {
    enum commands comm;
    struct addrinfo hints, *servinfo, *p;
    int sockfd, numbytes;  
    int rv;
    char buf[MAXDATASIZE];
    char s[INET6_ADDRSTRLEN];
    int flag;


    if (argc == 2 ) {
        if (strcmp(argv[1], "-r") == 0) {
            srand(time(NULL));
            flag = 1;
            comm = rand() % 3;
        } else if (strcmp(argv[1], "-o") == 0) {
            flag = 0;
            comm = 0;
        } else if (strcmp(argv[1], "-h") == 0) {
            print_help(argv[0]);
            exit(0);
        } else {
            printf("Wrong arg:\n");
            printf("    Type %s -h for help\n", argv[0]);
            exit(1);
        }
    } else {
        printf("Missing arg:\n");
        printf("    Type %s -h for help\n", argv[0]);
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(SERVER, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    while(1) {
        if ((numbytes = send(sockfd, &comm , sizeof(comm), 0)) < 0) {
            perror("recv");
            exit(1);
        }

        if ((numbytes = recv(sockfd, &buf, MAXDATASIZE, 0)) < 0) {
            perror("recv");
            exit(1);
        } else if (numbytes == 0) {
            exit(0);
        }

        buf[numbytes] = '\0';

        printf("response - %s\n", buf);

        if (flag) {
            comm = rand() % 3; 
        } else {
            comm += 1;
        }

        sleep(SEQTIMEOUT);
    }


    printf("client: received %s\n", buf);

    close(sockfd);

    return 0;
}