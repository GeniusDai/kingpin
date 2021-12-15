#include "kingpin/Utils.h"

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

using namespace std;
using namespace kingpin;

int main() {
    int sock = initListen(9000, 1);
    for (int i = 0; i < 10; ++i) {
        sleep(10);
        int conn = ::accept(sock, NULL, NULL);
        cout << conn << " accepted" << endl;
    }
    return 0;
}