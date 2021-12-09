#include "kingpin/Utils.h"
#include "kingpin/Exception.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <thread>
#include <vector>

using namespace std;
using namespace kingpin;

void func() {
    int sock;
    try {
        sock = connectHost("localhost", 9000, 10);
    } catch (const TimeoutException &e) {
        cout << e.what() << endl;
        return;
    }
    cout << "connect complete" << endl;
    const char *str = "hello, kingpin";
    if (::write(sock, str, ::strlen(str)) < 0) {
        ::perror("syscall write error");
    }
}

int main() {
    vector<thread> vt;
    for (uint i = 0; i < 100; ++i) {
        cout << "round " << i << endl;
        vt.emplace_back(func);
    }
    for (uint i = 0; i < vt.size(); ++i) {
        vt[i].join();
    }
    return 0;
}