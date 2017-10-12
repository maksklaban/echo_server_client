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

void sigchld_handler(int s);
void LogMessage(const char* ip, const char* command, FILE* logs);
void *get_in_addr(struct sockaddr *sa);
void getCurrentTime(char* message);
void startEchoServer();
