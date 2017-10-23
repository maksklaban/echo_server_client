#include "tcpServer.h"

int main(int argc, char const *argv[]) {
    tcpServer* serv = new tcpServer();
    serv->startServer();

    delete serv;
    return 0;
}