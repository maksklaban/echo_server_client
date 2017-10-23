#include "tcpClient.h"

int main(int argc, char const *argv[]) {
    tcpClient* cli = new tcpClient();

    // cli->sequentialSession();
    cli->randomSession();
    return 0;
}