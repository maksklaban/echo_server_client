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

#include "../gen_settings.c"

#define SEQTIMEOUT 10

void print_help(const char* app_name);
void startClient(int argc, char *argv[]);
