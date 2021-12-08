#include "kingpin/Utils.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdio>

using namespace std;
using namespace kingpin;

int main() {
    int sock;
    for (int i = 0; i < 100; ++i) {
        cout << "round " << i << endl;
        sock = connectHost("localhost", 9000, 30);
        const char *str = "hello, kingpin";
        char buf[1];
        if (::write(sock, str, ::strlen(str)) < 0) {
            perror("syscall write error");
        }
        cout << "write complete" << endl;
        if (::read(sock, buf, 1) < 0) {
            continue;
        }
    }
    return 0;
}