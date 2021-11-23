#include "Server.h"
#include "kingpin/core/EpollTPServer.h"

int main(int argc, char **argv) {
    ChessGameShareData data;
    EpollTPServer<ChessGameIOHandler, ChessGameShareData> server(8, 8889, &data);
    server.run();
    return 0;
}