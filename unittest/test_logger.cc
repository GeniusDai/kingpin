#include <iostream>
#include <vector>
#include <memory>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <ctime>

#include "kingpin/AsyncLogger.h"

using namespace std;
using namespace kingpin;

void log() {
    int n = 10;
    while (n--) {
        ERROR << "TEST" << END << "NEW " << "ERROR" << END;
        ERROR << "ERROR LINE:\n";
        ERROR << "ERROR LINE " << 1 << "\n" << "ERROR LINE 2" << END;
    }
}

int main() {
    int n_thr = 5;
    vector<shared_ptr<thread> > tp;
    for (int i = 0; i < n_thr; ++i) {
        tp.push_back(shared_ptr<thread>(new thread(&log)));
    }
    for (int i = 0; i < n_thr; ++i) {
        tp[i]->join();
    }
    return 0;
}
